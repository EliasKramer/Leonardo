#include "leonardo_value_bot_1.hpp"
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

float leonardo_value_bot_1::eval(chess::Board& board, int depth)
{
	return eval(board, depth, nullptr);
}

float leonardo_value_bot_1::eval(chess::Board& board, int depth, chess::Movelist* move_list)
{
	float score = 0.0f;

	std::pair<chess::GameResultReason, chess::GameResult> res =
		move_list == nullptr ?
		board.isGameOver() :
		board.isGameOver(*move_list);
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

	return score;
}

float leonardo_value_bot_1::recursive_eval(
	int depth,
	int depth_addition,
	int max_depth_addition,
	int end_depth,
	chess::Board& board,
	float alpha,
	float beta,
	int& depth_reached,
	bool should_sort,
	neural_network& value_nnet,
	matrix& input_matrix
)
{
	depth_reached = std::max(depth_reached, depth + depth_addition);

	if ((depth + depth_addition) >= (end_depth + max_depth_addition))
	{
		return eval(board, depth + depth_addition);
	}

	bool maximizing = board.sideToMove() == chess::Color::WHITE;
	float best_score = maximizing ? -FLT_MAX : FLT_MAX;
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	if (moves.size() == 0 || board.isRepetition())
	{
		return eval(board, depth + depth_addition, &moves);
	}

	bool moves_sorted =
		should_sort &&
		max_depth_addition != 0 &&
		depth + depth_addition < end_depth;
	if (moves_sorted)
	{
		sort_move_list(moves, board, false, value_nnet, input_matrix);
	}

	int i = 0;
	for (chess::Move move : moves)
	{
		board.makeMove(move);
		int depth_bonus = 0;

		int new_depth = 0;
		int new_depth_addition = 0;

		if (depth >= end_depth)
		{
			new_depth = depth + 1;
			new_depth_addition = 0;
		}
		else
		{
			new_depth = 0;
			new_depth_addition = depth_addition + 1;
		}

		int new_max_depth_addition = max_depth_addition;

		if (moves_sorted)
		{
			if (i < 3)
				depth_bonus = 0;
			else if (i < 7)
				depth_bonus = -1;
			else if (i < 8)
				depth_bonus = -2;
			else
				depth_bonus = -max_depth_addition;

			new_max_depth_addition += depth_bonus;
			if (new_max_depth_addition < 0)
				new_max_depth_addition = 0;
		}

		float score = recursive_eval(
			new_depth,
			new_depth_addition,
			new_max_depth_addition,
			end_depth,
			board,
			alpha,
			beta,
			depth_reached,
			should_sort,
			value_nnet,
			input_matrix);
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
void leonardo_value_bot_1::load_openings()
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
int leonardo_value_bot_1::get_opening_move(size_t hash)
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
void leonardo_value_bot_1::pre_sort_move_list(
	chess::Movelist& moves,
	chess::Board& board)
{
	float alpha = -FLT_MAX;
	float beta = FLT_MAX;

	for (chess::Move& move : moves)
	{
		int depth_reached = 0;
		bool maximizing = board.sideToMove() == chess::Color::WHITE;
		board.makeMove(move);
		float score = recursive_eval(
			1,
			0,
			0,
			3, //d
			board,
			alpha,
			beta,
			depth_reached,
			true,
			base_value_nnet,
			base_input_matrix
		);
		board.unmakeMove(move);

		if (board.sideToMove() == chess::Color::BLACK)
			score *= -1;

		move.setScore(score);
	}

	sort_move_list(moves, board, true, base_value_nnet, base_input_matrix);
}
leonardo_value_bot_1::leonardo_value_bot_1()
	: leonardo_value_bot_1(5)
{}

leonardo_value_bot_1::leonardo_value_bot_1(int end_depth)
	: end_depth(end_depth)
{
	load_openings();
	base_value_nnet = neural_network("value.parameters");
	base_input_matrix = matrix(leonardo_util::get_input_format());
}

void leonardo_value_bot_1::sort_move_list(
	chess::Movelist& moves,
	chess::Board& board,
	bool precise,
	neural_network& value_nnet,
	matrix& input_matrix
)
{
	for (chess::Move& move : moves)
	{
		board.makeMove(move);

		//white to move - black is good here so white is bad - high negative score
		//black to move - white is good here so white is good - high positive score
		float eval_score = eval(board, 0, &moves);
		if (board.sideToMove() == chess::Color::BLACK)
			eval_score = -eval_score;

		float score = -leonardo_util::get_value_nnet_eval(value_nnet, input_matrix, board, precise);
		//score += eval_score / 100 ;
		move.setScore((move.score() / 100) + (eval_score / 100) + score);
		board.unmakeMove(move);
	}

	moves.sort();
}

void leonardo_value_bot_1::thread_task(
	int t_idx,
	chess::Movelist& moves,
	std::mutex& print_mutex,
	int idx_start_incl,
	int idx_end_excl,
	int depth_addition,
	std::vector<float>& results,
	chess::Board board,
	std::vector<stockfish_interface::sf_move>& sf_moves)
{
	neural_network value_nnet = base_value_nnet;
	matrix input_matrix = base_input_matrix;

	for (int idx = idx_start_incl; idx < idx_end_excl; idx++)
	{
		board.makeMove(moves[idx]);

		int depth_reached = 0;
		float alpha = -FLT_MAX;
		float beta = FLT_MAX;
		float score = recursive_eval(
			1,
			0,
			depth_addition,
			end_depth,
			board,
			alpha,
			beta,
			depth_reached,
			true,
			value_nnet,
			input_matrix
		);
		if (board.sideToMove() == chess::Color::WHITE)
			score *= -1;
		results[idx] = score;

		int sf_idx = 0;
		stockfish_interface::sf_move sf_move = sf_moves[0];
		for (int j = 0; j < sf_moves.size(); j++)
		{
			if (chess::uci::moveToUci(moves[idx]) == sf_moves[j].move_str_uci)
			{
				sf_idx = j;
				sf_move = sf_moves[j];
				break;
			}
		}

		std::lock_guard<std::mutex> lock(print_mutex);
		std::cout
			<< chess::uci::moveToUci(moves[idx]) << " "
			<< std::setfill(' ') << std::setw(6) << score
			<< "\t t_idx: " << t_idx
			<< "\t preeval:" << std::setprecision(2) << moves[idx].score()
			<< "\t depth bonus: " << depth_addition
			<< " depth reached: " << depth_reached
			<< " sf eval: " << std::setprecision(2) << sf_move.value
			<< "\t sf idx: " << sf_idx
			<< "\n";
		board.unmakeMove(moves[idx]);
	}
}

static int get_depth_bonus(int move_idx, int move_count)
{
	static const std::vector<int> bonus = {
		3,2,2,2,1,1,1,1,0,0,0,0
	};

	if (move_idx < 0 || move_idx >= move_count)
		return 0;

	//move count is 30
	//move_idx is 0-29
	//move_idx / move_count is 0-1
	//move_idx / move_count * 3 is 0-3
	int chosen_idx = ((float)(move_idx) / (float)move_count) * (float)bonus.size();
	chosen_idx = std::clamp(chosen_idx, 0, (int)bonus.size() - 1);

	return bonus[chosen_idx];
}

chess::Move leonardo_value_bot_1::get_move(chess::Board& board)
{
	std::vector<stockfish_interface::sf_move> sf_moves = stockfish_interface::get_best_moves(board.getFen(), 4);

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
	int best_move_sf_rank = -1;
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	pre_sort_move_list(moves, board);

	float alpha = -FLT_MAX;
	float beta = FLT_MAX;

	std::vector<float> scores;
	std::vector<std::thread> threads;
	std::mutex print_mutex;

	int x = 0;
	int last_idx_in_thread = 0;
	int t_capacity = 2;
	for (chess::Move& move : moves)
	{
		int depth_bonus = get_depth_bonus(x, moves.size());
		scores.push_back(0);
		if ((x - last_idx_in_thread + 1) % t_capacity == 0)
		{
			threads.push_back(
				std::thread(
					&leonardo_value_bot_1::thread_task,
					this,
					threads.size(),
					std::ref(moves),
					std::ref(print_mutex),
					last_idx_in_thread,
					x,
					depth_bonus,
					std::ref(scores),
					board,
					std::ref(sf_moves)
				)
			);
			std::lock_guard<std::mutex> lock(print_mutex);
			std::cout << "starting thread " << threads.size() << "\n";
			std::cout << "from " << last_idx_in_thread << " to " << x << "\n";
			t_capacity = t_capacity + 2;
			last_idx_in_thread = x;
		}
		x++;
	}

	if (last_idx_in_thread != moves.size() - 1)
	{
		int depth_bonus = get_depth_bonus(x, moves.size());

		threads.push_back(
			std::thread(
				&leonardo_value_bot_1::thread_task,
				this,
				threads.size(),
				std::ref(moves),
				std::ref(print_mutex),
				last_idx_in_thread,
				moves.size() - 1,
				depth_bonus,
				std::ref(scores),
				board,
				std::ref(sf_moves)
			)
		);
	}

	for (auto& thread : threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}

	for (int i = 0; i < moves.size(); i++)
	{
		int sf_idx = 0;
		stockfish_interface::sf_move sf_move = sf_moves[0];
		for (int j = 0; j < sf_moves.size(); j++)
		{
			if (chess::uci::moveToUci(moves[i]) == sf_moves[j].move_str_uci)
			{
				sf_idx = j;
				sf_move = sf_moves[j];
				break;
			}
		}
		if (scores[i] > best_score)
		{
			best_score = scores[i];
			best_move = moves[i];
			best_move_sf_rank = sf_idx;
		}
		i++;
	}

	float sf_score_for_move = sf_moves[best_move_sf_rank].value;
	float best_possible_sf_move_score = sf_moves[0].value;
	std::cout << "leonardos choice: " << chess::uci::moveToUci(best_move) << "\n";
	std::cout << "leonardos choice in sf's rank: " << best_move_sf_rank << "\n";
	std::cout << "best possible move score: " << best_possible_sf_move_score << "\n";
	std::cout << "sf score for move: " << sf_score_for_move << "\n";
	std::cout << "best move score (according to leonardo): " << best_score << "\n";
	return best_move;
}
