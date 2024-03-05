#include "leonardo.hpp"

bool leonardo::search_cancelled()
{
	std::chrono::steady_clock::time_point now = std::chrono::high_resolution_clock::now();

	return
		now > deadline &&						//search time over
		iterative_deepening_depth >= chess_constants::MIN_DEPTH; //we have searched at least a bit
}

int32_t leonardo::corrected_result_score(chess::GameResult result, int32_t ply_from_root)
{
	switch (result)
	{
	case chess::GameResult::WIN:
		throw std::runtime_error("a win cannot occur. only the loss of the opponent is a win");
	case chess::GameResult::LOSE:
		return -chess_constants::MATE_SCORE + ply_from_root; //its better to have a lot of ply between us and mate
	case chess::GameResult::DRAW:
		return -chess_constants::DRAW_SCORE + ply_from_root; //a draw is better if it is further in the future
	default:
		throw std::runtime_error("cannot occur. game result not recognized.");
	}
}

bool leonardo::move_causes_draw(chess::Board& board, chess::Move& move)
{
	board.makeMove(move);
	bool ret_val = board.isHalfMoveDraw() || board.isRepetition() || board.isInsufficientMaterial();
	board.unmakeMove(move);

	return ret_val;
}

int32_t leonardo::eval(chess::Board& board)
{
	int32_t score = 0.0f;
	int32_t side_mult = board.sideToMove() == chess::Color::WHITE ? 1 : -1;

	const chess::Bitboard black_bb = board.us(chess::Color::BLACK);
	const chess::Bitboard white_bb = board.us(chess::Color::WHITE);
	const chess::Bitboard pawns_bb = board.pieces(chess::PieceType::PAWN);

	for (int i = 0; i < 5; i++)
	{
		chess::Bitboard curr_bb = board.pieces(chess::PieceType(i));

		while (curr_bb)
		{
			unsigned int sq = chess::builtin::poplsb(curr_bb);
			chess::Bitboard curr_bb = chess::Bitboard(1) << sq;
			if ((curr_bb & black_bb) != 0) //black
			{
				score -= static_eval::PIECE_EVAL[i];
				score -= static_eval::POSITION_VALUE[1][i][sq];
			}
			else
			{
				score += static_eval::PIECE_EVAL[i];
				score += static_eval::POSITION_VALUE[0][i][sq];
			}
		}
	}

	return score * side_mult;
}

int32_t leonardo::quiescene_search(chess::Board& board, int alpha, int beta)
{
	chess::Movelist moves;
	chess::movegen::legalmoves<chess::MoveGenType::CAPTURE>(moves, board);

	int32_t score = eval(board);

	if (board.isInsufficientMaterial() || board.isRepetition() || board.isHalfMoveDraw())
	{
		return corrected_result_score(chess::GameResult::DRAW, chess_constants::MAX_DEPTH);
	}

	if (score >= beta)
	{
		return beta;
	}
	if (score > alpha)
	{
		alpha = score;
	}

	if (search_cancelled())
	{
		return 0;
	}

	//order_moves_quiescene(moves, board);

	for (chess::Move& move : moves)
	{
		board.makeMove(move);
		score = -quiescene_search(board, -beta, -alpha);
		board.unmakeMove(move);

		if (score >= beta)
		{
			return beta;
		}
		if (score > alpha)
		{
			alpha = score;
		}
	}

	return alpha;
}

int32_t leonardo::search(
	int32_t depth,
	int32_t ply_from_root,
	chess::Board& board,
	int32_t alpha, // alpha is the best value that the maximizing player can guarantee at that level or above.
	int32_t beta, // beta is the best value that the minimizing player can guarantee at that level or above.
	chess::Move& chosen_move_to_play)
{
	if (search_cancelled())
		return 0;

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	if (moves.size() == 0)
	{
		return board.inCheck() ?
			corrected_result_score(chess::GameResult::LOSE, ply_from_root) :
			corrected_result_score(chess::GameResult::DRAW, ply_from_root);
	}

	if (board.isRepetition() ||
		board.isHalfMoveDraw() ||
		board.isInsufficientMaterial())
	{
		return corrected_result_score(chess::GameResult::DRAW, ply_from_root);
	}

	if (depth <= 0)
	{
		return quiescene_search(board, alpha, beta);
	}

	tt::entry_type tt_flag = tt::entry_type::upper_bound;
	int32_t tt_value = tt.get_stored_score(board.hash(), ply_from_root, depth, alpha, beta);
	if (tt_value != tt::unknown_eval)
	{
		chess::Move stored_move = tt.get_move(board.hash());

		if (!move_causes_draw(board, stored_move))
		{
			if (ply_from_root == 0)
			{
				chosen_move_to_play = tt.get_move(board.hash());
			}

			return tt_value;
		}
	}

	sort_move_list(moves, board, ply_from_root, depth);

	chess::Move& best_current_move = moves[0];
	for (chess::Move move : moves)
	{
		if (search_cancelled())
			return 0;

		board.makeMove(move);
		int score = -search(depth - 1, ply_from_root + 1, board, -beta, -alpha, chosen_move_to_play);
		board.unmakeMove(move);

		//current score is better than our opponent can achieve. our opponent will not pick this path
		if (score >= beta)
		{
			tt.store(board.hash(), ply_from_root, depth, beta, tt::entry_type::lower_bound, move);

			return beta;
		}
		if (score > alpha)
		{
			alpha = score;
			tt_flag = tt::entry_type::exact;
			if (ply_from_root == 0)
			{
				chosen_move_to_play = move;
			}
			best_current_move = move;
		}
	}
	
	tt.store(board.hash(), ply_from_root, depth, alpha, tt_flag, best_current_move);
	
	return alpha;
}

void leonardo::sort_move_list(chess::Movelist& moves, chess::Board& board, int32_t ply_from_root, int32_t depth)
{
	chess::Move tt_move = tt.get_move(board.hash());

	for (chess::Move& move : moves)
	{
		if (move == tt_move)
		{
			move.setScore(30000);
		}
		else {
			move.setScore(0);
		}
	}

	moves.sort();
}

void leonardo::setup_members(int32_t ms_left)
{
	deadline = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(ms_left);
	iterative_deepening_depth = 1;
	reached_depth = 0;
}

leonardo::leonardo(uint32_t hash_table_size_mb)
	:tt(hash_table_size_mb)
{}

chess::Move leonardo::get_move(chess::Board& board, int32_t ms_left, std::string& info)
{
	setup_members(ms_left);

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	chess::Move best_move = chess::Move::NULL_MOVE;
	for (iterative_deepening_depth = 1; !search_cancelled() && iterative_deepening_depth < chess_constants::MAX_DEPTH; iterative_deepening_depth++)
	{
		chess::Move current_best_move = chess::Move::NULL_MOVE;

		int score = search(
			iterative_deepening_depth,
			0,
			board,
			-1000000000,
			1000000000,
			current_best_move);

		if (!search_cancelled())
		{
			reached_depth = iterative_deepening_depth;
			best_move = current_best_move;
		}
	}

	if (best_move == chess::Move::NULL_MOVE)
	{
		best_move = moves[0];
	}

	return best_move;
}

void leonardo::resize_tt(uint32_t hash_table_size_mb)
{
	tt = transposition_table(hash_table_size_mb);
}
