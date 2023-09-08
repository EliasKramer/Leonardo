#include "leonardo_value_bot.hpp"

leonardo_value_bot::leonardo_value_bot(neural_network given_value_nnet)
	: leonardo_value_bot(given_value_nnet, 4, 0)
{}

leonardo_value_bot::leonardo_value_bot(
	neural_network given_value_nnet,
	int max_capture_depth,
	float dropout
)
	: Player("leonardo value bot "),
	value_net(given_value_nnet),
	max_capture_depth(max_capture_depth),
	dropout(dropout)
{
	if (given_value_nnet.is_in_gpu_mode())
	{
		std::cout << "not supported gpu mode\n";
		return;
	}
}

void leonardo_value_bot::mutate()
{
	std::cout << "Mutate\n";
	//random number between 0 and 6
	//number gen
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 6);

	int rand_num = dis(gen);

	//mutation amount
	std::uniform_real_distribution<> dis2(0, 1);
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
			if (board.getCurrentTurnColor() == White)
			{
				return -1000000; //disencourages stalemates
			}
			else
			{
				return 1000000;
			}
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

	//worst case for color: 4 pawns on same color
	//best case for color: 8 pawns on same color
	float pawn_same_color_bonus = std::abs(white_pawns_on_same_color) - std::abs(black_pawns_on_same_color);
	score += pawn_same_color_bonus * pawn_same_color_bonus_mult;

	//worst case for color: 0 pawns protected
	//best case for color: 7 pawns protected
	float pawn_self_protection = white_pawn_self_protection - black_pawn_self_protection;
	score += pawn_self_protection * pawn_self_protection_mult;

	//worst case 0
	//best case 8
	float passed_pawn_bonus = white_passed_pawn_bonus - black_passed_pawn_bonus;
	score += passed_pawn_bonus * passed_pawn_mult;

	float king_safety = white_king_safety - black_king_safety;
	score += king_safety * king_safety_mult;

	//Evaluating the king position
	//usually the king is worth 20k and the position value is added to that.
	//this is not done here, since these values cancel out. - could be done for clarity
	float king_pos_val =
		(POSITION_VALUE_KING[White][board.getGameDurationState()][boardRep.KingPos[White]] -
			POSITION_VALUE_KING[Black][board.getGameDurationState()][boardRep.KingPos[Black]]);

	score += (king_pos_val / 100) * king_pos_mult;

	//TODO
	//to encourage pawns for more structure, it would be useful to look if they get protected
	//by another pawn. and increase the value if it is that way

	//print fen, score and pawn protection

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
	bool& search_finished)
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

	for (std::unique_ptr<Move>& curr : possible_capture_moves)
	{
		ChessBoard copyBoard = board.getCopyByValue();
		copyBoard.makeMove(*curr);

		nodes_searched++;
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
			start_time, search_finished);

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
	}

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
	bool& search_finished)
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
			search_finished);
	}
	else
	{
		std::vector<std::unique_ptr<Move>> moves = board.getAllLegalMoves();

		//no more moves
		if (moves.size() == 0)
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
				board.getCurrentTurnColor() == White ?
				-2000000.0f :
				2000000.0f;
		}

		bool nnet_pruning = false;
		//curr_depth == depth - 1 || //depth 2
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
		for (int i = 0; i < branch_count; i++)
		{
			ChessBoard copyBoard = board.getCopyByValue();
			copyBoard.makeMove(*moves[move_indices[i]]);

			nodes_searched++;
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
				search_finished);

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
		}
		return bestEval;
	}
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
	while (std::chrono::high_resolution_clock::now() < start + std::chrono::milliseconds(ms_per_move))
	{
		bool search_finished = true;
		float tmp_move_score =
			get_move_score_recursively(
				board,
				i_depth - 1,
				is_white_to_move,
				BLACK_WIN_EVAL_VALUE,
				WHITE_WIN_EVAL_VALUE,
				nodesSearched,
				endPointsEvaluated,
				max_capture_depth_reached,
				input_board,
				ms_per_move,
				start,
				search_finished);

		if (search_finished)
		{
			move_score = tmp_move_score;
		}
		i_depth++;
	}

	scores[thread_id] = move_score * colorMult;

#ifdef PRINT_SEARCH_INFO
	std::cout
		<< "Move: " + move_str
		+ ", Depth reached: " + std::to_string(i_depth)
		+ ", Thread: " + std::to_string(thread_id)
		+ ", Nodes Searched: " + std::to_string(nodesSearched)
		+ ", Score: " + std::to_string(move_score * colorMult)
		+ ", Max Capture Depth reached: " + std::to_string(max_capture_depth_reached)
		+ ", Endstates Evaluated: " + std::to_string(endPointsEvaluated)
		+ "\n";
#endif // PRINT_SEARCH_INFO
}

int leonardo_value_bot::getMove(const ChessBoard& board, const UniqueMoveList& legal_moves)
{
	auto begin = std::chrono::high_resolution_clock::now();

	int moveIdx = 0;
	std::vector<float> move_scores(legal_moves.size());
	std::vector<std::thread> threads;

	for (const std::unique_ptr<Move>& curr : legal_moves)
	{
		ChessBoard boardCopy = board;
		boardCopy.makeMove(*curr);

		threads.push_back(std::thread(
			&leonardo_value_bot::thread_task,
			this,
			moveIdx,
			curr.get()->getString(),
			std::ref(move_scores),
			boardCopy
		));
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

	matrix input_board(leonardo_util::get_input_format());

#ifdef PRINT_SEARCH_INFO
	std::cout << "done. took " << ms_to_str(duration) << "\n";
	std::cout << "current position according to stockfish: " << stockfish_interface::eval(board.getFen(), 8) << "\n";
	std::cout << "current position according to leonardo after searching: " << bestMoveScore << "\n";
	std::cout << "leonardo static eval: " << get_eval(board, input_board) * (board.getCurrentTurnColor() == White ? 1 : -1) << "\n";
#endif // DEBUG
	return bestMoveIdx;
}