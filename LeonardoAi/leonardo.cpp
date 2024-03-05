#include "leonardo.hpp"
#include "leonardo_util.hpp"
#include <filesystem>

#define MATE_SCORE 10000000
#define DRAW_SCORE		  0
#define MAX_DEPTH		256
#define MIN_DEPTH		  5

bool leonardo::search_cancelled()
{
	std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
	long long ms_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();

	return
		ms_taken > ms_per_move &&				//search time over
		iterative_deepening_depth >= MIN_DEPTH; //we have searched at least a bit
}

//PAWN, KNIGHT, BISHOP, ROOK, QUEEN
const int PIECE_EVAL[5] = { 100, 320, 330, 500, 900 };
//accounts for kings aswell
const int SEE_PIECE_EVAL[6] = { 100, 330, 330, 500, 900, 2000 };
//helps determin if a position is equal if you only count piece values and skip pawns
const int PIECE_VALUE_EQL_POSITIONS[5] = { 100, 300, 300, 500, 900 };

const int POSITION_VALUE[2][5][64]
{
	//it seems to be flipped, but look at with indexes
	//index 0 is a1 - index 63 is h8
	//so the first row represents actually the last board

	//white
	{
		//Pawn
		{0, 0, 0, 0, 0, 0, 0, 0,
		5, 10, 10,-20,-20, 10, 10, 5,
		5, -5,-10, 0 , 0 ,-10, -5, 5,
		0,  0,  0, 20, 20,  0,  0, 0,
		5,  5, 10, 25, 25, 10,  5, 5,
		10, 10, 20, 30, 30, 20, 10, 10,
		50, 50, 50, 50, 50, 50, 50, 50,
		0, 0, 0, 0, 0, 0, 0, 0},

		//Knight
		{-50,-40,-30,-30,-30,-30,-40,-50,
		-40,-20, 0, 5, 5, 0,-20,-40,
		-30, 5, 10, 15, 15, 10, 5,-30,
		-30, 0, 15, 20, 20, 15, 0,-30,
		-30, 5, 15, 20, 20, 15, 5,-30,
		-30, 0, 10, 15, 15, 10, 0,-30,
		-40,-20, 0, 0, 0, 0,-20,-40,
		-50,-40,-30,-30,-30,-30,-40,-50},

		//Bishop
		{-20,-10,-10,-10,-10,-10,-10,-20,
		-10, 5, 0, 0, 0, 0, 5,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10, 0, 10, 10, 10, 10, 0,-10,
		-10, 5, 10, 10, 10, 10, 5,-10,
		-10, 0, 5, 10, 10, 5, 0,-10,
		-10, 0, 0, 0, 0, 0, 0,-10,
		-20,-10,-10,-10,-10,-10,-10,-20},

		//Rook
		{0,0, 0, 5, 5, 0, 0, 0,
		-5, 0, 0, 0, 0, 0, 0, -5,
		-5, 0, 0, 0, 0, 0, 0, -5,
		-5, 0, 0, 0, 0, 0, 0, -5,
		-5, 0, 0, 0, 0, 0, 0, -5,
		-5, 0, 0, 0, 0, 0, 0, -5,
		5, 10, 10, 10, 10, 10, 10, 5,
		0, 0, 0, 0, 0, 0, 0, 0},

		//Queen
		{-20,-10,-10, -5, -5,-10,-10,-20,
		-10, 0, 5, 0, 0, 0, 0,-10,
		-10, 5, 5, 5, 5, 5, 0,-10,
		0, 0, 5, 5, 5, 5, 0, -5,
		-5, 0, 5, 5, 5, 5, 0, -5,
		-10, 0, 5, 5, 5, 5, 0,-10,
		-10, 0, 0, 0, 0, 0, 0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20}
	 },
	//black
	{
		//Pawn
		{0, 0, 0, 0, 0, 0, 0, 0,
		50, 50, 50, 50, 50, 50, 50, 50,
		10, 10, 20, 30, 30, 20, 10, 10,
		 5, 5, 10, 25, 25, 10, 5, 5,
		 0, 0, 0, 20, 20, 0, 0, 0,
		 5, -5,-10, 0, 0,-10, -5, 5,
		 5, 10, 10,-20,-20, 10, 10, 5,
		 0, 0, 0, 0, 0, 0, 0, 0},

		 //Knight
		 {-50,-40,-30,-30,-30,-30,-40,-50,
		 -40,-20, 0, 0, 0, 0,-20,-40,
		 -30, 0, 10, 15, 15, 10, 0,-30,
		 -30, 5, 15, 20, 20, 15, 5,-30,
		 -30, 0, 15, 20, 20, 15, 0,-30,
		 -30, 5, 10, 15, 15, 10, 5,-30,
		 -40,-20, 0, 5, 5, 0,-20,-40,
		 -50,-40,-30,-30,-30,-30,-40,-50},

		 //Bishop
		 {-20,-10,-10,-10,-10,-10,-10,-20,
		 -10, 0, 0, 0, 0, 0, 0,-10,
		 -10, 0, 5, 10, 10, 5, 0,-10,
		 -10, 5, 5, 10, 10, 5, 5,-10,
		 -10, 0, 10, 10, 10, 10, 0,-10,
		 -10, 10, 10, 10, 10, 10, 10,-10,
		 -10, 5, 0, 0, 0, 0, 5,-10,
		 -20,-10,-10,-10,-10,-10,-10,-20},

		 //Rook
		 {0, 0, 0, 0, 0, 0, 0, 0,
		 5, 10, 10, 10, 10, 10, 10, 5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 0, 0, 0, 5, 5, 0, 0, 0},

		 //Queen
		 {-20,-10,-10, -5, -5,-10,-10,-20,
		 -10, 0, 0, 0, 0, 0, 0,-10,
		 -10, 0, 5, 5, 5, 5, 0,-10,
		  -5, 0, 5, 5, 5, 5, 0, -5,
		   0, 0, 5, 5, 5, 5, 0, -5,
		 -10, 5, 5, 5, 5, 5, 0,-10,
		 -10, 0, 5, 0, 0, 0, 0,-10,
		 -20,-10,-10, -5, -5,-10,-10,-20}
	}
};
const int POSITION_VALUE_KING[2][2][64] =
{
	//white	
	{
		//midgame
		{20, 30, 10, 0, 0, 10, 30, 20,
		20, 20, 0, 0, 0, 0, 20, 20,
		-10, -20, -20, -20, -20, -20, -20, -10,
		-20, -30, -30, -40, -40, -30, -30, -20,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30},

		//endgame
		{-50, -30, -30, -30, -30, -30, -30, -50,
		-30, -30, 0, 0, 0, 0, -30, -30,
		-30, -10, 20, 30, 30, 20, -10, -30,
		-30, -10, 30, 40, 40, 30, -10, -30,
		-30, -10, 30, 40, 40, 30, -10, -30,
		-30, -10, 20, 30, 30, 20, -10, -30,
		-30, -20, -10, 0, 0, -10, -20, -30,
		-50, -40, -30, -20, -20, -30, -40, -50}
	},
	//black
	{
		//midgame
		{-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-20,-30,-30,-40,-40,-30,-30,-20,
		-10,-20,-20,-20,-20,-20,-20,-10,
		20, 20, 0, 0, 0, 0, 20, 20,
		20, 30, 10, 0, 0, 10, 30, 20},
		//endgame
		{-50,-40,-30,-20,-20,-30,-40,-50,
		-30,-20,-10, 0, 0,-10,-20,-30,
		-30,-10, 20, 30, 30, 20,-10,-30,
		-30,-10, 30, 40, 40, 30,-10,-30,
		-30,-10, 30, 40, 40, 30,-10,-30,
		-30,-10, 20, 30, 30, 20,-10,-30,
		-30,-30, 0, 0, 0, 0,-30,-30,
		-50,-30,-30,-30,-30,-30,-30,-50}
	}
};

int leonardo::eval(chess::Board& board, chess::Movelist& moves, int ply, bool only_caputes_in_moves)
{
	int score = 0.0f;
	int side_mult = board.sideToMove() == chess::Color::WHITE ? 1 : -1;

	std::pair<chess::GameResultReason, chess::GameResult> res = only_caputes_in_moves ? board.isGameOver() : board.isGameOver(moves);
	if (res.first != chess::GameResultReason::NONE)
	{
		int val = 0.0f;
		switch (res.second)
		{
		case chess::GameResult::WIN:
			val = MATE_SCORE - ply;
			break;
		case chess::GameResult::LOSE:
			val = -MATE_SCORE + ply;
			break;
		case chess::GameResult::DRAW:
			val = DRAW_SCORE + ply;
			break;
		}

		return val;
	}

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
				score -= PIECE_EVAL[i];
				score -= POSITION_VALUE[1][i][sq];
			}
			else
			{
				score += PIECE_EVAL[i];
				score += POSITION_VALUE[0][i][sq];
			}
		}
	}

	return score * side_mult;
}

int leonardo::search(
	int depth,
	int ply_from_root,
	chess::Board& board,
	int alpha, // alpha is the best value that the maximizing player can guarantee at that level or above.
	int beta, // beta is the best value that the minimizing player can guarantee at that level or above.
	chess::Move& best_move)
{
	if (search_cancelled())
		return 0;

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	if (moves.size() == 0 ||
		board.isRepetition() ||
		board.isHalfMoveDraw() ||
		board.isInsufficientMaterial())
	{
		return eval(board, moves, ply_from_root, false);
	}

	if (depth <= 0)
	{
		return eval(board, moves, ply_from_root, false);
	}

	chess::Move& best_current_move = moves[0];
	for (chess::Move move : moves)
	{
		board.makeMove(move);
		int score = -search(depth - 1, ply_from_root + 1, board, -beta, -alpha, best_move);
		board.unmakeMove(move);

		if (search_cancelled())
			return 0;

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
				best_move = move;
				searched_at_least_one_move = true;
			}
			best_current_move = move;
		}
	}

	return alpha;
}

leonardo::leonardo(uint32_t hash_table_size_mb)
{}

chess::Move leonardo::get_move(chess::Board& board, int ms_left, std::string& info)
{
	ms_per_move = ms_left;

	start_time = std::chrono::high_resolution_clock::now();
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	bool maximizing = board.sideToMove() == chess::Color::WHITE;

	int score = 0;

	int reached_depth = 0;
	chess::Move best_move = chess::Move::NULL_MOVE;
	for (iterative_deepening_depth = 1; !search_cancelled() && iterative_deepening_depth < MAX_DEPTH; iterative_deepening_depth++)
	{
		searched_at_least_one_move = false;

		chess::Move tmp = chess::Move::NULL_MOVE;

		score = search(
			iterative_deepening_depth,
			0,
			board,
			-1000000000,
			1000000000,
			tmp);

		if (!search_cancelled())
		{
			reached_depth = iterative_deepening_depth;
			best_move = tmp;
		}
	}

	if (best_move == chess::Move::NULL_MOVE)
	{
		best_move = moves[0];
	}

	return best_move;
}