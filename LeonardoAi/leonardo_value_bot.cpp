#include "leonardo_value_bot.hpp"
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

float leonardo_value_bot::eval(chess::Board& board, int depth)
{
	float score = 0.0f;

	std::pair<chess::GameResultReason, chess::GameResult> res = board.isGameOver();
	if (res.first != chess::GameResultReason::NONE)
	{
		float val = 0.0f;
		switch (res.second)
		{
		case chess::GameResult::WIN:
			val = 100000 - depth;
			break;
		case chess::GameResult::LOSE:
			val = -100000 + depth;
			break;
		case chess::GameResult::DRAW:
			val = -1000.0f + depth;
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

	score /= 100;

	float nnet_eval = 0;

	if (std::abs(score) < 1.0f)
	{
		nnet_eval = leonardo_util::get_value_nnet_eval(value_nnet, input_matrix, board, false);
		if (board.sideToMove() == chess::Color::BLACK)
			nnet_eval = -nnet_eval;
	}

	return score + (nnet_eval * .05f);
}

static bool should_sort(int curr_depth)
{
	return curr_depth == 1;
}

float leonardo_value_bot::search(
	int depth,
	int depth_addition,
	chess::Board& board,
	float alpha,
	float beta)
{
	if (depth >= (end_depth + depth_addition))
		return eval(board, depth);

	if (depth >= end_depth)
		depth_addition -= 1;

	bool maximizing = board.sideToMove() == chess::Color::WHITE;
	float best_score = maximizing ? -FLT_MAX : FLT_MAX;
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	if (moves.size() == 0 || board.isRepetition())
	{
		return eval(board, depth);
	}

	int i = 0;
	for (chess::Move move : moves)
	{
		board.makeMove(move);
		int depth_bonus = 0;

		float score = search(depth + 1, depth_addition + depth_bonus, board, alpha, beta);
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
		i++;
	}

	return best_score;
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
		chess::Board board = chess::Board(DEFAULT_FEN);
		std::vector<std::string> book_moves = leonardo_util::split_string(str, ' ');

		for (int i = 0; i < book_moves.size(); i++)
		{
			std::string move_str = book_moves[i];

			chess::Movelist moves;
			chess::movegen::legalmoves(moves, board);

			bool move_found = false;
			for (chess::Move& curr : moves)
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
int leonardo_value_bot::get_opening_move(size_t hash)
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

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, indices.size() - 1);

	return indices[dis(gen)];
}
leonardo_value_bot::leonardo_value_bot()
	: leonardo_value_bot(5)
{}

leonardo_value_bot::leonardo_value_bot(int end_depth)
	: end_depth(end_depth)
{
	load_openings();
	value_nnet = neural_network("value.parameters");
	input_matrix = matrix(leonardo_util::get_input_format());
}

void leonardo_value_bot::sort_move_list(chess::Movelist& moves, chess::Board& board)
{
	for (chess::Move& move : moves)
	{
		board.makeMove(move);
		float score = leonardo_util::get_value_nnet_eval(value_nnet, input_matrix, board, true);
		//score *= -1; // because the value nnet is seeing it from the other side's perspective
		move.setScore(score);
		board.unmakeMove(move);
	}

	moves.sort();
}


chess::Move leonardo_value_bot::get_move(chess::Board& board)
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
	float best_score = -FLT_MAX;
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	auto sf_moves = stockfish_interface::get_best_moves(board.getFen(), 3);

	sort_move_list(moves, board);

	float alpha = -FLT_MAX;
	float beta = FLT_MAX;
	int i = 0;
	for (chess::Move& move : moves)
	{

		bool maximizing = board.sideToMove() == chess::Color::WHITE;
		board.makeMove(move);
		float score = search(
			1,
			0,
			board,
			alpha,
			beta);
		board.unmakeMove(move);
		if (board.sideToMove() == chess::Color::BLACK)
			score *= -1;

		int sf_idx = 0;
		stockfish_interface::sf_move sf_move = sf_moves[0];
		for (int j = 0; j < sf_moves.size(); j++)
		{
			if (chess::uci::moveToUci(move) == sf_moves[j].move_str_uci)
			{
				sf_idx = j;
				sf_move = sf_moves[j];
				break;
			}
		}

		//move.setScore(score);

		std::cout
			<< chess::uci::moveToUci(move) << " "
			<< score * (maximizing ? 1 : -1) 
			<< " preeval: " << move.score()
			<< " sf: " << sf_move.value
			<< " sf idx: " << sf_idx
			<< "\n";
		// this displays the relative eval

		if (score > best_score)
		{
			best_score = score;
			best_move = move;
		}
		i++;
	}

	return best_move;
}
