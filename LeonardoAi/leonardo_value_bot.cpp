#include "leonardo_value_bot.hpp"

leonardo_value_bot::leonardo_value_bot(neural_network given_value_nnet)
	: leonardo_value_bot(given_value_nnet, 4, 0, 0, 0, 0, 0, 0, 0, 0)
{
	throw std::runtime_error("not supported");
}

leonardo_value_bot::leonardo_value_bot(
	neural_network given_value_nnet,
	int max_capture_depth,
	float dropout,
	float piece_value_mult,
	float piece_pos_value_mult,
	float pawn_same_color_bonus_mult,
	float pawn_self_protection_mult,
	float passed_pawn_mult,
	float king_pos_mult,
	float king_safety_mult
)
	: Player("leonardo value bot"),
	value_net(given_value_nnet),
	max_capture_depth(max_capture_depth),
	dropout(dropout),
	piece_value_mult(piece_value_mult),
	piece_pos_value_mult(piece_pos_value_mult),
	pawn_same_color_bonus_mult(pawn_same_color_bonus_mult),
	pawn_self_protection_mult(pawn_self_protection_mult),
	passed_pawn_mult(passed_pawn_mult),
	king_pos_mult(king_pos_mult),
	king_safety_mult(king_safety_mult)
{
	if (given_value_nnet.is_in_gpu_mode())
	{
		std::cout << "not supported gpu mode\n";
		return;
	}
	load_openings();
}

void leonardo_value_bot::reroll_params()
{
	//random number between 0 and 6
	//number gen
	static std::random_device rd;
	static std::mt19937 gen(rd());
	//random float

	std::uniform_real_distribution<> dis(0.001, 5);

	piece_value_mult = dis(gen);
	piece_pos_value_mult = dis(gen);
	pawn_same_color_bonus_mult = dis(gen);
	pawn_self_protection_mult = dis(gen);
	passed_pawn_mult = dis(gen);
	king_pos_mult = dis(gen);
	king_safety_mult = dis(gen);
}

void leonardo_value_bot::mutate(float min, float max)
{
	//random number between 0 and 6
	//number gen
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 6);

	int rand_num = dis(gen);

	//mutation amount
	std::uniform_real_distribution<> dis2(min, max);
	float mutation_amount = dis2(gen);

	float min_val = 0.001f;

	switch (rand_num)
	{
	case 0:
		piece_value_mult += mutation_amount;
		break;
	case 1:
		piece_pos_value_mult += mutation_amount;
		break;
	case 2:
		pawn_same_color_bonus_mult += mutation_amount;
		break;
	case 3:
		pawn_self_protection_mult += mutation_amount;
		break;
	case 4:
		passed_pawn_mult += mutation_amount;
		break;
	case 5:
		king_pos_mult += mutation_amount;
		break;
	case 6:
		king_safety_mult += mutation_amount;
		break;
	}
}

void leonardo_value_bot::get_params_from_other(const leonardo_value_bot& other)
{
	piece_value_mult = other.piece_value_mult;
	piece_pos_value_mult = other.piece_pos_value_mult;
	pawn_same_color_bonus_mult = other.pawn_same_color_bonus_mult;
	pawn_self_protection_mult = other.pawn_self_protection_mult;
	passed_pawn_mult = other.passed_pawn_mult;
	king_pos_mult = other.king_pos_mult;
	king_safety_mult = other.king_safety_mult;
}

std::string leonardo_value_bot::param_string()
{
	return
		"piece_value_mult: " + std::to_string(piece_value_mult) + "\n" +
		"piece_pos_value_mult: " + std::to_string(piece_pos_value_mult) + "\n" +
		"pawn_same_color_bonus_mult: " + std::to_string(pawn_same_color_bonus_mult) + "\n" +
		"pawn_self_protection_mult: " + std::to_string(pawn_self_protection_mult) + "\n" +
		"passed_pawn_mult: " + std::to_string(passed_pawn_mult) + "\n" +
		"king_pos_mult: " + std::to_string(king_pos_mult) + "\n" +
		"king_safety_mult: " + std::to_string(king_safety_mult) + "\n";
}

float leonardo_value_bot::get_nnet_eval(const ChessBoard& board, matrix& input_board)
{
	// - if white wins return highest possible number
	// - if black wins return lowest possible number
	leonardo_util::set_matrix_from_chessboard(board, input_board);
	value_net.forward_propagation(input_board);
	float nnet_eval = leonardo_util::get_value_nnet_output(value_net.get_output());

	float color_mult = board.getCurrentTurnColor() == White ? 1.0f : -1.0f;

	return nnet_eval * color_mult;
}

static BitBoard get_pawn_attack_bb(ChessColor color, BitBoard pawns)
{
	if (color == White)
	{
		return ((pawns << NORTH_WEST) & ~FILE_A) | ((pawns << NORTH_EAST) & ~FILE_H);
	}
	else
	{
		return ((pawns >> 7) & ~FILE_H) | ((pawns >> 9) & ~FILE_A);
	}
}
static BitBoard get_file(Square s)
{
	return FILE_A << (s % 8);
}
static BitBoard get_file_neighbours(Square s)
{
	return get_file(s) | ((get_file(s) >> 1) & ~FILE_H) | ((get_file(s) << 1) & ~FILE_A);
}
static BitBoard remove_rank(Square s, ChessColor c)
{
	int rank = s / 8;
	if (c == White)
	{
		return (BITBOARD_ALL << ((rank + 1) * 8));
	}
	else
	{
		return (BITBOARD_ALL >> ((8 - rank) * 8));
	}
}
static bool passed_pawn_bb(Square s, ChessColor c)
{
	return get_file_neighbours(s) & remove_rank(s, c);
}
static bool half_passed_pawn_bb(Square s, ChessColor c)
{
	return get_file(s) & remove_rank(s, c);
}

static int manhatten_distance(Square s1, Square s2)
{
	int s1_rank = s1 / 8;
	int s1_file = s1 % 8;
	int s2_rank = s2 / 8;
	int s2_file = s2 % 8;

	return abs(s1_rank - s2_rank) + abs(s1_file - s2_file);
}
static int x_distance(Square s1, Square s2)
{
	int s1_file = s1 % 8;
	int s2_file = s2 % 8;

	return abs(s1_file - s2_file);
}

float leonardo_value_bot::get_simpel_eval(const ChessBoard& board)
{
	return get_simpel_eval(board, false);
}
float leonardo_value_bot::get_simpel_eval(const ChessBoard& board, bool print)
{
	BoardRepresentation boardRep = board.getBoardRepresentation();

	//get the obvious out the way 
	// - if white wins return highest possible number
	// - if black wins return lowest possible number
	// - stalemate and draw is 0
	GameState state = board.getGameState();
	float gameStatePoints = GAME_STATE_EVALUATION[state];
	//if the game is still ongoing the value will be -1 and thus should be continued evaluating
	if (gameStatePoints != -1)
	{
		if (state == Draw || state == Stalemate)
		{
			return draw_score[board.getCurrentTurnColor()];
		}
		else
		{
			return gameStatePoints;
		}
	}

	BitBoard white_pawn_attack_bb = get_pawn_attack_bb(White, boardRep.PiecesOfColor[White] & boardRep.PiecesOfType[Pawn]);
	BitBoard black_pawn_attack_bb = get_pawn_attack_bb(Black, boardRep.PiecesOfColor[Black] & boardRep.PiecesOfType[Pawn]);

	//now calculate the score
	float score = 0;

	float black_pawns_on_same_color = 0;
	float white_pawns_on_same_color = 0;

	float white_pawn_self_protection = 0;
	float black_pawn_self_protection = 0;

	float white_passed_pawn_bonus = 0;
	float black_passed_pawn_bonus = 0;

	float white_king_safety = 0;
	float black_king_safety = 0;

	for (int i = A1; i <= H8; i++)
	{
		BitBoard idxBB = BB_SQUARE[i];

		//iterate over every type except king
		for (int typeIdx = 0; typeIdx <= Queen; typeIdx++)
		{
			//if current square overlaps with the current piece type
			if (bitboardsOverlap(idxBB, boardRep.PiecesOfType[typeIdx]))
			{
				//get the material value of the piece type
				float materialValue = (float)PIECETYPE_VALUE[typeIdx] / 100.0f * piece_value_mult;

				bool currPieceIsBlack = bitboardsOverlap(idxBB, boardRep.PiecesOfColor[Black]);
				ChessColor currPieceColor = currPieceIsBlack ? Black : White;

				materialValue += ((float)POSITION_VALUE[currPieceColor][typeIdx][i] / 100.0f) * piece_pos_value_mult;

				//if pieces are black, negate the value
				if (currPieceIsBlack)
				{
					materialValue *= -1;
				}

				if (typeIdx == Pawn)
				{
					//pawn self protection
					if (currPieceIsBlack)
					{
						black_pawn_self_protection += bitboardsOverlap(idxBB, black_pawn_attack_bb) ? 1 : 0;
					}
					else
					{
						white_pawn_self_protection += bitboardsOverlap(idxBB, white_pawn_attack_bb) ? 1 : 0;
					}

					//pawn on same color
					float score_for_same_pawn_bb = bitboardsOverlap(WHITE_SQUARES, idxBB) ? 1 : -1;
					if (currPieceIsBlack)
					{
						black_pawns_on_same_color += score_for_same_pawn_bb;
					}
					else
					{
						white_pawns_on_same_color += score_for_same_pawn_bb;
					}

					//passed pawn
					if (passed_pawn_bb((Square)i, currPieceColor))
					{
						if (currPieceIsBlack)
						{
							black_passed_pawn_bonus += 1;
						}
						else
						{
							white_passed_pawn_bonus += 1;
						}
					}
					else if (half_passed_pawn_bb((Square)i, currPieceColor))
					{
						if (currPieceIsBlack)
						{
							black_passed_pawn_bonus += 0.5;
						}
						else
						{
							white_passed_pawn_bonus += 0.5;
						}
					}

					//king safety
					if (x_distance(boardRep.KingPos[currPieceColor], (Square)i) <= 1 &&
						manhatten_distance(boardRep.KingPos[currPieceColor], (Square)i) <= 1)
					{
						if (currPieceIsBlack)
						{
							black_king_safety += 1;
						}
						else
						{
							white_king_safety += 1;
						}
					}
				}

				//add the value to the total score multiplied by material weight
				score += materialValue;
			}
		}
	}
	float material_value = score;

	//worst case for color: 4 pawns on same color
	//best case for color: 8 pawns on same color
	float pawn_same_color_bonus = std::abs(white_pawns_on_same_color) - std::abs(black_pawns_on_same_color);
	pawn_same_color_bonus *= pawn_same_color_bonus_mult;
	score += pawn_same_color_bonus;

	//worst case for color: 0 pawns protected
	//best case for color: 7 pawns protected
	float pawn_self_protection = white_pawn_self_protection - black_pawn_self_protection;
	pawn_self_protection *= pawn_self_protection_mult;
	score += pawn_self_protection;

	float passed_pawn_bonus = white_passed_pawn_bonus - black_passed_pawn_bonus;
	passed_pawn_bonus *= passed_pawn_mult;
	score += passed_pawn_bonus;

	float king_safety = white_king_safety - black_king_safety;
	king_safety *= king_safety_mult;
	score += king_safety;

	//Evaluating the king position
	//usually the king is worth 20k and the position value is added to that.
	//this is not done here, since these values cancel out. - could be done for clarity
	float king_white_pos_val = POSITION_VALUE_KING[White][board.getGameDurationState()][boardRep.KingPos[White]] / 100;
	float king_black_pos_val = POSITION_VALUE_KING[Black][board.getGameDurationState()][boardRep.KingPos[Black]] / 100;



	float king_pos_val = king_white_pos_val - king_black_pos_val;
	king_pos_val *= king_pos_mult;
	score += king_pos_val;

	if (print)
	{
		//print summary of how the evaluation was formed
		std::cout << "evaulation summary: \n"
			<< score << " SCORE\n"
			<< material_value << " material value\n"

			<< pawn_same_color_bonus << " pawn same color bonus "
			<< std::abs(white_pawns_on_same_color) << " - " << std::abs(black_pawns_on_same_color) << " = "
			<< pawn_same_color_bonus / pawn_same_color_bonus_mult << " * "
			<< pawn_same_color_bonus_mult << "\n"

			<< pawn_self_protection << " pawn self protection "
			<< white_pawn_self_protection << " - " << black_pawn_self_protection << " = "
			<< pawn_self_protection / pawn_self_protection_mult << " * "
			<< pawn_self_protection_mult << "\n"

			<< passed_pawn_bonus << " passed pawn bonus "
			<< white_passed_pawn_bonus << " - " << black_passed_pawn_bonus << " = "
			<< passed_pawn_bonus / passed_pawn_mult << " * "
			<< passed_pawn_mult << "\n"

			<< king_safety << " king safety "
			<< white_king_safety << " - " << black_king_safety << " = "
			<< king_safety / king_safety_mult << " * "
			<< king_safety_mult << "\n"

			<< king_pos_val << " king pos val "
			<< king_white_pos_val << " - " << king_black_pos_val << " = "
			<< king_pos_val / king_pos_mult << " * "
			<< king_pos_mult << "\n"

			<< std::endl;
	}

	return score;
}

float leonardo_value_bot::get_eval(const ChessBoard& board, matrix& input_board)
{
	//float hard_coded_eval = hard_coded_influence != 0 ? get_simpel_eval(board) : 0;
	//float nnet_eval = nnet_influence != 0 ? get_nnet_eval(board, input_board) : 0;

	//std::cout << "Hard coded eval: " << hard_coded_eval << " Nnet eval: " << nnet_eval << std::endl;


	//float eval =
		//hard_coded_eval * hard_coded_influence +
		//nnet_eval * nnet_influence;

	return get_simpel_eval(board);
}

float leonardo_value_bot::get_capture_move_score_recursively(
	const ChessBoard& board,
	int curr_depth,
	bool is_maximizing_player,
	float alpha,
	float beta,
	int& nodes_searched,
	int& end_states_searched,
	int& max_depth_reached,
	matrix& input_board,
	long long allowed_time_ms,
	std::chrono::steady_clock::time_point& start_time,
	bool& search_finished,
	std::string& best_moves_str)
{
	//if time is up, return
	if (std::chrono::steady_clock::now() > start_time + std::chrono::milliseconds(allowed_time_ms))
	{
		search_finished = false;
		return 0;
	}

	UniqueMoveList possible_capture_moves = board.getAllLegalCaptureMoves();

	if (possible_capture_moves.size() == 0 || curr_depth >= max_capture_depth)
	{
		nodes_searched++;
		end_states_searched++;
		max_depth_reached = std::max(max_depth_reached, curr_depth);
		float eval = get_eval(board, input_board);
		return eval;
	}

	float best_eval = is_maximizing_player ? -FLT_MAX : FLT_MAX;

	int best_idx = 0;
	std::string best_move_str = "";
	int i = 0;
	for (std::unique_ptr<Move>& curr : possible_capture_moves)
	{
		ChessBoard copyBoard = board.getCopyByValue();
		copyBoard.makeMove(*curr);

		nodes_searched++;
		std::string best_move = "";
		float evaluation = get_capture_move_score_recursively(
			copyBoard,
			curr_depth + 1,
			!is_maximizing_player,
			alpha,
			beta,
			nodes_searched,
			end_states_searched,
			max_depth_reached,
			input_board,
			allowed_time_ms,
			start_time, search_finished,
			best_move);

		if (is_maximizing_player)
		{
			best_eval = std::max(best_eval, evaluation);
			alpha = std::max(alpha, evaluation);
			if (beta <= alpha)
			{
				break;
			}
		}
		else
		{
			best_eval = std::min(best_eval, evaluation);
			beta = std::min(beta, evaluation);
			if (beta <= alpha)
			{
				break;
			}
		}

		if (best_eval == evaluation)
		{
			best_idx = i;
			best_move_str = best_move;
		}
		i++;
	}

	best_moves_str = possible_capture_moves[best_idx].get()->getString() + " " + best_move_str;// "(d" + std::to_string(curr_depth) + ", val:" + std::to_string(bestEval) + ") " + best_move_str;

	return best_eval;
}

float leonardo_value_bot::get_move_score_recursively(
	const ChessBoard& board,
	int curr_depth,
	bool is_maximizing_player,
	float alpha,
	float beta,
	int& nodes_searched,
	int& end_states_searched,
	int& max_capture_depth_reached,
	matrix& input_board,
	long long allowed_time_ms,
	std::chrono::steady_clock::time_point& start_time,
	bool& search_finished,
	std::string& best_moves_str,
	std::string prefix)
{
	//if time is up, return
	if (std::chrono::steady_clock::now() > start_time + std::chrono::milliseconds(allowed_time_ms))
	{
		search_finished = false;
		return 0;
	}

	if (curr_depth <= 0)
	{
		end_states_searched++;
		nodes_searched++;

		return get_capture_move_score_recursively(
			board,
			0,
			is_maximizing_player,
			alpha,
			beta,
			nodes_searched,
			end_states_searched,
			max_capture_depth_reached,
			input_board,
			allowed_time_ms,
			start_time,
			search_finished,
			best_moves_str);
	}
	else
	{
		std::vector<std::unique_ptr<Move>> moves = board.getAllLegalMoves();

		//CAN BE IMPROVED
		if (board.getGameState() != Ongoing)
		{
			nodes_searched++;
			end_states_searched++;
			//dont know if this works
			return board.isKingInCheck() ?
				(board.getCurrentTurnColor() == White ?
					//when the depth is very high, the checkmate can be done earlier
					//(when you search with depth 4, the function gets called with 3, 2, 1, 0 recursively)
					//therefore a higher depth in the argument means actually low depth.
					////finding a checkmate at a low depth is better, because it can be delivered earlier
					GAME_STATE_EVALUATION[BlackWon] - curr_depth :
					GAME_STATE_EVALUATION[WhiteWon] + curr_depth) :
				//draw
				draw_score[board.getCurrentTurnColor()];
		}

		bool nnet_pruning =
			false;//	curr_depth == 3; //|| //depth 2
		//curr_depth == depth - 2;//depth 3

		float keep_branch_count;
		if (nnet_pruning)
		{
			keep_branch_count = (float)moves.size() - ((float)moves.size() * dropout);
			if (keep_branch_count < 1)
			{
				keep_branch_count = 1;
			}
		}
		else
		{
			keep_branch_count = moves.size();
		}
		int branch_count = (int)keep_branch_count;

		std::vector<int> move_indices;

		if (nnet_pruning)
		{
			std::vector<float> move_scores;
			int idx = 0;
			for (std::unique_ptr<Move>& curr : moves)
			{
				ChessBoard copyBoard = board.getCopyByValue();
				copyBoard.makeMove(*curr);
				float val = get_eval(copyBoard, input_board);
				move_scores.push_back(val);
				move_indices.push_back(idx);
				idx++;
			}
			//highest score is always first. no matter the color
			std::sort(move_indices.begin(), move_indices.end(), [&move_scores](int i1, int i2) {return move_scores[i1] > move_scores[i2]; });
		}
		else
		{
			for (int i = 0; i < moves.size(); i++)
			{
				move_indices.push_back(i);
			}
		}

		float bestEval = is_maximizing_player ? -FLT_MAX : FLT_MAX;
		int best_idx = 0;
		std::string best_move_str = "";
		for (int i = 0; i < branch_count; i++)
		{
			if (moves[move_indices[i]].get()->getString() == "d4f6")
			{
				int a = 0; //DEBUG
			}
			ChessBoard copyBoard = board.getCopyByValue();
			copyBoard.makeMove(*moves[move_indices[i]]);

			nodes_searched++;

			std::string best_move = "";
			float evaluation = get_move_score_recursively(
				copyBoard,
				curr_depth - 1,
				!is_maximizing_player,
				alpha,
				beta,
				nodes_searched,
				end_states_searched,
				max_capture_depth_reached,
				input_board,
				allowed_time_ms,
				start_time,
				search_finished,
				best_move,
				prefix + "|");
			if (some_print)
			{
				std::cout << prefix << " " << (moves[move_indices[i]].get()->getString()) << "      " << evaluation << "\n";
			}
			if (is_maximizing_player)
			{
				bestEval = std::max(bestEval, evaluation);
				alpha = std::max(alpha, evaluation);
				if (beta <= alpha)
				{
					break;
				}
			}
			else
			{
				bestEval = std::min(bestEval, evaluation);
				beta = std::min(beta, evaluation);
				if (beta <= alpha)
				{
					break;
				}
			}

			if (bestEval == evaluation)
			{
				best_idx = i;
				best_move_str = best_move;
			}
		}

		best_moves_str = moves[best_idx].get()->getString() + " " + best_move_str;// "(d" + std::to_string(curr_depth) + ", val:" + std::to_string(bestEval) + ") " + best_move_str;

		return bestEval;
	}
}

static std::vector<std::string> split_string(const std::string& input, char separator) {
	std::vector<std::string> result;
	std::string current;

	for (char c : input) {
		if (c == separator) {
			if (!current.empty()) {
				result.push_back(current);
				current.clear();
			}
		}
		else {
			current += c;
		}
	}

	if (!current.empty()) {
		result.push_back(current);
	}

	return result;
}
void leonardo_value_bot::add_opening_position(const ChessBoard& board, const std::string& move_str)
{
	if (position_is_known_opening(board))
	{
		//get the vector and add the move
		opening_positions[board].push_back(move_str);
	}
	else
	{
		//create a new vector and add the move
		std::vector<std::string> moves;
		moves.push_back(move_str);
		opening_positions[board] = moves;
	}
}

void leonardo_value_bot::load_openings()
{
	std::ifstream file("openings.txt");

	if (!file)
	{
		std::cout << "opening file not found\n";
		return;
	}

	std::string str;
	while (std::getline(file, str))
	{
		ChessBoard board(STARTING_FEN);
		std::vector<std::string> book_moves = split_string(str, ' ');

		for (int i = 0; i < book_moves.size(); i++)
		{
			std::string move_str = book_moves[i];
			add_opening_position(board, move_str);

			UniqueMoveList moves = board.getAllLegalMoves();
			bool move_found = false;
			for (std::unique_ptr<Move>& curr : moves)
			{
				if (curr->getString() == move_str)
				{
					move_found = true;
					board.makeMove(*curr);
					break;
				}
			}
			if (!move_found)
			{
				std::cout << "opening move not found\n";
				break;
			}
		}
	}
	file.close();
}

int leonardo_value_bot::get_random_opening_move(const ChessBoard& board)
{
	UniqueMoveList legal_moves = board.getAllLegalMoves();

	//random 
	std::random_device rd;
	std::mt19937 g(rd());

	std::vector<std::string> opening_moves = opening_positions[board];
	//random idx
	std::uniform_int_distribution<> dis(0, opening_moves.size() - 1);
	int idx = dis(g);

	std::string book_move = opening_moves[idx];

#ifdef PRINT_SEARCH_INFO
	std::cout << "Opening position found!\n";
#endif // PRINT_SEARCH_INFO

	for (int i = 0; i < legal_moves.size(); i++)
	{
		if (legal_moves[i]->getString() == book_move)
		{
			return i;
		}
	}

#ifdef PRINT_SEARCH_INFO
	std::cout << "+-+-+-+-+-+-+-+-+\n";
	std::cout << "Opening Position was found, but had no valid move!\n";
	std::cout << "+-+-+-+-+-+-+-+-+\n";
#endif // PRINT_SEARCH_INFO

	return 0;
}

void leonardo_value_bot::thread_task(
	int thread_id,
	const std::string& move_str,
	std::vector<float>& scores,
	ChessBoard board)
{

	bool is_white_to_move = board.getCurrentTurnColor() == White;
	int colorMult = is_white_to_move ? -1 : 1;


	int endPointsEvaluated = 0;
	int nodesSearched = 0;
	int max_capture_depth_reached = 0;

	matrix input_board(leonardo_util::get_input_format());

	auto start = std::chrono::high_resolution_clock::now();
	//iterative deepening.
	//save best moves
	//sort them										!!!!!!!!!!!!!!important!!!!!!!!!!!

	//search them first
	//if the best move is found, stop searching
	float move_score = 0;
	int i_depth = 1;
	std::string best_moves_str = "";
	while (std::chrono::high_resolution_clock::now() < start + std::chrono::milliseconds(ms_per_move))
	{
		bool search_finished = true;
		std::string best_move_str_tmp = "";
		float tmp_move_score =
			get_move_score_recursively(
				board,
				i_depth - 1,
				is_white_to_move,
				-FLT_MAX,
				FLT_MAX,
				nodesSearched,
				endPointsEvaluated,
				max_capture_depth_reached,
				input_board,
				ms_per_move,
				start,
				search_finished,
				best_move_str_tmp,
				"");
		if (some_print)
		{
			std::cout << "\n----------------------------- " << std::to_string(i_depth) + "\n";
		}
		if (search_finished)
		{
			move_score = tmp_move_score;
			best_moves_str = best_move_str_tmp;
			i_depth++;
		}
	}

	scores[thread_id] = move_score * colorMult;

#ifdef PRINT_SEARCH_INFO
	std::cout
		<< "Move: " + move_str
		+ ", Depth finished: " + std::to_string(i_depth)
		+ ", Thread: " + std::to_string(thread_id)
		+ ", Nodes Searched: " + std::to_string(nodesSearched)
		+ ", Score: " + std::to_string(move_score * colorMult)
		+ ", Max Capture Depth reached: " + std::to_string(max_capture_depth_reached)
		+ ", Endstates Evaluated: " + std::to_string(endPointsEvaluated)
		+ " evaluated combination: " + best_moves_str
		+ "\n";
#endif // PRINT_SEARCH_INFO
}

bool leonardo_value_bot::position_is_known_opening(const ChessBoard& board)
{
	return opening_positions.find(board) != opening_positions.end();
}

int leonardo_value_bot::getMove(const ChessBoard& board, const UniqueMoveList& legal_moves)
{
	if (position_is_known_opening(board))
	{
		return get_random_opening_move(board);
	}

	auto begin = std::chrono::high_resolution_clock::now();

#ifdef PRINT_SEARCH_INFO
	ChessBoard cpy = board.getCopyByValue();

	float eval_before = get_simpel_eval(board) * (board.getCurrentTurnColor() == White ? 1 : -1);
	std::cout << "eval before: " << eval_before << "\n";
#endif // PRINT_SEARCH_INFO


	int moveIdx = 0;
	std::vector<float> move_scores(legal_moves.size());
	std::vector<std::thread> threads;

	for (const std::unique_ptr<Move>& curr : legal_moves)
	{
		ChessBoard boardCopy = board.getCopyByValue();
		boardCopy.makeMove(*curr);

		if (curr->getString() == "b5e2" || true) //DEBUG
		{
			threads.push_back(std::thread(
				&leonardo_value_bot::thread_task,
				this,
				moveIdx,
				curr.get()->getString(),
				std::ref(move_scores),
				boardCopy
			));
		}
		moveIdx++;
	}

	for (std::thread& curr : threads)
	{
		if (curr.joinable())
		{
			curr.join();
		}
	}

	float bestMoveScore = -FLT_MAX;
	int bestMoveIdx = 0;
	for (int i = 0; i < move_scores.size(); i++)
	{
		std::cout << legal_moves[i]->getString() << " " << move_scores[i] << "\n";
		if (move_scores[i] > bestMoveScore)
		{
			std::cout << "new best move\n";
			bestMoveScore = move_scores[i];
			bestMoveIdx = i;
		}
	}
	static bool first = true;
	if (legal_moves[bestMoveIdx]->getString() == "b5e2")
	{
		if (first)
		{
			first = false;
		}
		else
		{
			//ms_per_move = 9000000;
			ChessBoard boardCopy = board.getCopyByValue();
			Move m(B5, E2);
			boardCopy.makeMove(m);
			std::cout << GAME_STATE_STRING[boardCopy.getGameState()] << "\n";
			//some_print = true;
			thread_task(
				0,
				"b5e2",
				std::ref(move_scores),
				boardCopy
			);
		}
	}

	auto end = std::chrono::high_resolution_clock::now();

	long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();


#ifdef PRINT_SEARCH_INFO
	cpy.makeMove(*legal_moves[bestMoveIdx]);
	float eval_after = get_simpel_eval(cpy) * (board.getCurrentTurnColor() == White ? 1 : -1);

	float eval_diff = eval_after - eval_before;
	std::cout << "eval after: " << eval_after << "\n";

	std::cout << "done. took " << ms_to_str(duration) << "\n";
	std::cout << "current position according to stockfish: " << stockfish_interface::eval(board.getFen(), 8) << "\n";
	std::cout << "current position according to leonardo after searching: " << bestMoveScore << "\n";
	std::cout << "leonardo static eval: " << eval_after << "\n";
	std::cout << "leardo immediate move eval: " << eval_diff << "\n";

	//get_simpel_eval(board, true);
#endif // DEBUG
	return bestMoveIdx;
}

void leonardo_value_bot::print_eval(std::string fen)
{
	ChessBoard board(fen);
	get_simpel_eval(board, true);
}