#include "minimax_player.hpp"

const float PIECE_EVAL[13] = {
	1,//WHITEPAWN,
	3.2,//WHITEKNIGHT,
	3.3,//WHITEBISHOP,
	5,//WHITEROOK,
	9,//WHITEQUEEN,
	1000,//WHITEKING,
	-1,//BLACKPAWN,
	-3.2,//BLACKKNIGHT,
	-3.3,//BLACKBISHOP,
	-5,//BLACKROOK,
	-9,//BLACKQUEEN,
	-1000,//BLACKKING,
	0,//NONE
};

static float eval(chess::Board& board)
{
	float score = 0.0f;

	std::pair<chess::GameResultReason, chess::GameResult> res = board.isGameOver();
	if (res.first != chess::GameResultReason::NONE)
	{
		float val = 0.0f;
		switch (res.second)
		{
		case chess::GameResult::WIN:
			val = 1000000;
			break;
		case chess::GameResult::LOSE:
			val = -1000000;
			break;
		case chess::GameResult::DRAW:
			val = -1000.0f;
			break;
		}
		if (board.sideToMove() == chess::Color::BLACK)
			return -val;
		else
			return val;
	}

	for (int i = 0; i < 64; i++)
	{
		chess::Piece curr_piece = board.at<chess::Piece>(chess::Square(i));

		if (curr_piece != chess::Piece::NONE)
		{
			score += PIECE_EVAL[(int)curr_piece];
		}
	}

	return score;
}

static float rec_eval(chess::Board& board, int depth)
{
	if (depth == 0)
		return eval(board);

	float best_score = -1000000.0f;

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);


	for (chess::Move move : moves)
	{
		board.makeMove(move);
		float score = -rec_eval(board, depth - 1);
		board.unmakeMove(move);
		if (score > best_score)
			best_score = score;
	}
	return best_score;
}

minimax_player::minimax_player()
	:minimax_player(3)
{
}

minimax_player::minimax_player(int start_depth)
	:start_depth(start_depth)
{
}

chess::Move minimax_player::get_move(chess::Board& board)
{
	bool white_to_move = board.sideToMove() == chess::Color::WHITE;
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);
	float best_score = -1000000.0f;
	chess::Move best_move = moves[0];
	for (chess::Move move : moves)
	{
		board.makeMove(move);
		float score = -rec_eval(board, start_depth);

		board.unmakeMove(move);
		if (score > best_score)
		{
			best_score = score;
			best_move = move;
		}
	}
	return best_move;
}
