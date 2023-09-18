#include "leonardo_value_bot.hpp"

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


leonardo_value_bot::leonardo_value_bot(int ms_per_move, std::string name)
	: Player(name),
	ms_per_move(ms_per_move)
{
	value_net = neural_network("value.parameters");
	if (value_net.is_in_gpu_mode())
	{
		std::cout << "not supported gpu mode\n";
		return;
	}
	load_openings();
}

leonardo_value_bot::leonardo_value_bot(int ms_per_move)
	: leonardo_value_bot(ms_per_move, "leonardo_value_bot")
{}

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
			<< board.getFen() << "\n"
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
	float hard_coded_eval = get_simpel_eval(board);
	float nnet_eval = nnet_influence != 0 ? get_nnet_eval(board, input_board) : 0;

	//std::cout << "Hard coded eval: " << hard_coded_eval << " Nnet eval: " << nnet_eval << std::endl;

	float eval =
		hard_coded_eval +
		nnet_eval * nnet_influence;

	return eval;
}

float leonardo_value_bot::get_move_score_recursively(
	const ChessBoard& board, //1
	int curr_depth, //2
	int max_depth, //2
	bool is_maximizing_player, //3
	float alpha, //4
	float beta, //5
	int& nodes_searched, //6
	int& end_states_searched, //7
	matrix& input_board, //8
	long long allowed_time_ms, //9
	std::chrono::steady_clock::time_point& start_time, //10
	bool& search_cancelled, //11
	std::string& best_moves_str, //12
	std::string prefix) //13
{
	//if time is up, return
	if (std::chrono::steady_clock::now() > start_time + std::chrono::milliseconds(allowed_time_ms))
	{
		search_cancelled = true;
		return 0;
	}

	if (curr_depth == max_depth)
	{
		end_states_searched++;
		nodes_searched++;
		return get_eval(board, input_board);
	}

	std::vector<std::unique_ptr<Move>> moves = board.getAllLegalMoves();
	GameState gamestate = board.getGameState(moves);
	if (gamestate != Ongoing)
	{
		nodes_searched++;
		end_states_searched++;

		int depth_left = max_depth - curr_depth;

		float score =
			gamestate == WhiteWon ? GAME_STATE_EVALUATION[WhiteWon] + depth_left :
			gamestate == BlackWon ? GAME_STATE_EVALUATION[BlackWon] - depth_left :
			draw_score[board.getCurrentTurnColor()];

		return score;
	}

	float bestEval = is_maximizing_player ? -FLT_MAX : FLT_MAX;
	int best_idx = 0;
	std::string best_move_str = "";
	int i = 0;
	for (const std::unique_ptr<Move>& m : moves)
	{
		ChessBoard copyBoard = board;
		copyBoard.makeMove(*m);

		nodes_searched++;

		std::string best_move = "";
		float evaluation = get_move_score_recursively(
			copyBoard,
			curr_depth + 1,
			max_depth,
			!is_maximizing_player,
			alpha,
			beta,
			nodes_searched,
			end_states_searched,
			input_board,
			allowed_time_ms,
			start_time,
			search_cancelled,
			best_move,
			prefix + "|");
		if (print_tree)
		{
			std::cout << prefix << " " << (m->getString()) << "      " << evaluation << "\n";
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
		i++;
	}

	best_moves_str = moves[best_idx].get()->getString() + " " + best_move_str;// "(d" + std::to_string(curr_depth) + ", val:" + std::to_string(bestEval) + ") " + best_move_str;

	return bestEval;

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

int leonardo_value_bot::get_random_opening_move(const ChessBoard& board, const UniqueMoveList& legal_moves)
{
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
		bool search_canceled = false;
		std::string best_move_str_tmp = "";
		float tmp_move_score =
			get_move_score_recursively(
				board, //1
				1,
				i_depth, //2
				is_white_to_move, //3
				-FLT_MAX, //4
				FLT_MAX, //5
				nodesSearched, //6
				endPointsEvaluated, //7
				input_board, //8
				ms_per_move, //9
				start, //10
				search_canceled, //11
				best_move_str_tmp, //12
				"" //13
			);
		if (print_tree)
		{
			std::cout << "\n----------------------------- " << std::to_string(i_depth) + "\n";
			std::cout << "eval: " << std::to_string(tmp_move_score) << "\n";
			std::cout << "\n----------------------------- " << std::to_string(i_depth) + "\n";
		}
		if (!search_canceled)
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
		+ ", Depth finished: " + std::to_string(i_depth - 1)
		+ ", Thread: " + std::to_string(thread_id)
		+ ", Nodes Searched: " + std::to_string(nodesSearched)
		+ ", Score: " + std::to_string(move_score * colorMult)
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
		return get_random_opening_move(board, legal_moves);
	}

	auto begin = std::chrono::high_resolution_clock::now();

#ifdef PRINT_SEARCH_INFO
	ChessBoard cpy = board;

	float eval_before = get_simpel_eval(board) * (board.getCurrentTurnColor() == White ? 1 : -1);
	std::cout << "eval before: " << eval_before << "\n";
#endif // PRINT_SEARCH_INFO


	int moveIdx = 0;
	std::vector<float> move_scores(legal_moves.size());
	std::vector<std::thread> threads;

	for (const std::unique_ptr<Move>& curr : legal_moves)
	{
		ChessBoard boardCopy = board;
		boardCopy.makeMove(*curr);

		if (curr->getString() == "b5c6" || true) //DEBUG
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
		if (move_scores[i] > bestMoveScore)
		{
			bestMoveScore = move_scores[i];
			bestMoveIdx = i;
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