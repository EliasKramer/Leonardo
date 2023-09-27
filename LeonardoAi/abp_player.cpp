#include "abp_player.hpp"
#include "leonardo_util.hpp"

//PAWN, KNIGHT, BISHOP, ROOK, QUEEN
const float PIECE_EVAL[5] = { 100.0f, 320.0f, 330.0f, 500.0f, 900.0f };

const float POSITION_VALUE[2][5][64]
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

static float eval(chess::Board& board, int depth)
{
	float score = 0.0f;

	std::pair<chess::GameResultReason, chess::GameResult> res = board.isGameOver();
	if (res.first != chess::GameResultReason::NONE)
	{
		float val = 0.0f;
		switch (res.second)
		{
		case chess::GameResult::WIN:
			val = 100000 + depth;
			break;
		case chess::GameResult::LOSE:
			val = -100000 - depth;
			break;
		case chess::GameResult::DRAW:
			val = -1000.0f - depth;
			break;
		}
		if (board.sideToMove() == chess::Color::BLACK)
			return -val;
		else
			return val;
	}

	chess::Bitboard black_bb = board.us(chess::Color::BLACK);
	chess::Bitboard white_bb = board.us(chess::Color::WHITE);

	for (int i = 0; i < 5; i++)
	{
		chess::Bitboard curr_bb = board.pieces(chess::PieceType(i));

		while (curr_bb)
		{
			unsigned int sq = chess::builtin::poplsb(curr_bb);
			chess::Bitboard curr_bb = chess::Bitboard(1) << sq;
			if ((curr_bb & black_bb) != 0)
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

	return score / 100;
}

static float eval(chess::Board& board)
{
	return eval(board, 0);
}

static float recursive_eval(
	int depth,
	chess::Board& board,
	bool maximizing,
	float alpha,
	float beta)
{
	if (depth == 0)
		return eval(board);
	float best_score = maximizing ? -FLT_MAX : FLT_MAX;
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	if (moves.size() == 0 || board.isRepetition())
	{
		return eval(board, depth);
	}

	for (chess::Move move : moves)
	{
		board.makeMove(move);
		float score = recursive_eval(depth - 1, board, !maximizing, alpha, beta);
		board.unmakeMove(move);
		if (maximizing)
		{
			best_score = std::max(best_score, score);
			alpha = std::max(alpha, score);
		}
		else
		{
			best_score = std::min(best_score, score);
			beta = std::min(beta, score);
		}
		if (beta <= alpha)
			break;
	}

	return best_score;
}
void abp_player::load_openings()
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
		chess::Board board = chess::Board(DEFAULT_FEN);
		std::vector<std::string> book_moves = leonardo_util::split_string(str, ' ');

		for (int i = 0; i < book_moves.size(); i++)
		{
			std::string move_str = book_moves[i];

			chess::Movelist moves;
			chess::movegen::legalmoves(moves, board);

			bool move_found = false;
			for (chess::Move curr : moves)
			{
				if (chess::uci::moveToUci(curr) == move_str)
				{
					move_found = true;
					openings.emplace_back(std::make_pair(board.hash(), curr));
					board.makeMove(curr);
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
int abp_player::get_opening_move(size_t hash)
{
	std::vector<int> indices;
	for (int i = 0; i < openings.size(); i++)
	{
		if (openings[i].first == hash)
		{
			indices.push_back(i);
		}
	}
	if (indices.empty())
	{
		return -1;
	}

	//random generator
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, indices.size() - 1);

	return indices[dis(gen)];
}
abp_player::abp_player()
	: abp_player(5)
{}

abp_player::abp_player(int start_depth)
	: start_depth(start_depth)
{
	load_openings();
}

chess::Move abp_player::get_move(chess::Board& board)
{
	int opening_move_idx = get_opening_move(board.hash());
	if (opening_move_idx != -1)
	{
		std::cout << "opening move found\n\n";
		return openings[opening_move_idx].second;
	}

	chess::Move best_move = chess::Move::NULL_MOVE;
	bool black = board.sideToMove() == chess::Color::BLACK;
	bool white = board.sideToMove() == chess::Color::WHITE;
	float best_score = -1000000.0f;
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);
	for (chess::Move move : moves)
	{
		board.makeMove(move);
		float score = recursive_eval(start_depth - 1, board, board.sideToMove() == chess::Color::WHITE, -FLT_MAX, FLT_MAX);
		if (board.sideToMove() == chess::Color::WHITE)
			score *= -1;
		board.unmakeMove(move);
		std::cout << chess::uci::moveToUci(move) << " " << score << "\n";
		if (score > best_score)
		{
			best_score = score;
			best_move = move;
		}
	}

	return best_move;
}
