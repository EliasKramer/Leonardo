#include "leonardo.hpp"

constexpr int32_t MATE_SCORE = 10000000;
constexpr int32_t DRAW_SCORE = 0;
constexpr int32_t MAX_DEPTH = 256;
constexpr int32_t MIN_DEPTH = 2;

bool leonardo::search_cancelled()
{
	std::chrono::steady_clock::time_point now = std::chrono::high_resolution_clock::now();

	return
		now > deadline &&						//search time over
		iterative_deepening_depth >= MIN_DEPTH; //we have searched at least a bit
}

int32_t leonardo::corrected_result_score(chess::GameResult result, int32_t ply_from_root)
{
	switch (result)
	{
	case chess::GameResult::WIN:
		throw std::runtime_error("a win cannot occur. only the loss of the opponent is a win");
	case chess::GameResult::LOSE:
		return -MATE_SCORE + ply_from_root; //its better to have a lot of ply between us and mate
	case chess::GameResult::DRAW:
		return -DRAW_SCORE + ply_from_root; //a draw is better if it is further in the future
	default:
		throw std::runtime_error("cannot occur. game result not recognized.");
	}
}

int32_t leonardo::eval(chess::Board& board, chess::Movelist& moves, int ply, bool only_caputes_in_moves)
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
		return eval(board, moves, ply_from_root, false);
	}

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
			return beta;
		}
		if (score > alpha)
		{
			alpha = score;
			if (ply_from_root == 0)
			{
				chosen_move_to_play = move;
			}
			best_current_move = move;
		}
	}

	return alpha;
}

void leonardo::setup_members(int32_t ms_left)
{
	deadline = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(ms_left);
	iterative_deepening_depth = 1;
	reached_depth = 0;
}

leonardo::leonardo(uint32_t hash_table_size_mb)
{}

chess::Move leonardo::get_move(chess::Board& board, int32_t ms_left, std::string& info)
{
	setup_members(ms_left);

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	chess::Move best_move = chess::Move::NULL_MOVE;
	for (iterative_deepening_depth = 1; !search_cancelled() && iterative_deepening_depth < MAX_DEPTH; iterative_deepening_depth++)
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