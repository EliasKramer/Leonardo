#include "leonardo_value_bot_3.hpp"
#include "leonardo_util.hpp"

//PAWN, KNIGHT, BISHOP, ROOK, QUEEN
const float PIECE_EVAL[5] = { 100.0f, 320.0f, 330.0f, 500.0f, 900.0f };
//helps determin if a position is equal if you only count piece values and skip pawns
const float PIECE_VALUE_EQL_POSITIONS[5] = { 100, 300.0f, 300.0f, 500.0f, 900.0f };

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
float leonardo_value_bot_3::eval(chess::Board& board, int depth)
{
	float score = 0.0f;

	std::pair<chess::GameResultReason, chess::GameResult> res = board.isGameOver(moves);
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

	int non_pawn_count_white = 0;
	int non_pawn_count_black = 0;

	float board_material_equal_score = 0;
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
				board_material_equal_score -= PIECE_VALUE_EQL_POSITIONS[i];
				if(i != 0)
					non_pawn_count_black++;
			}
			else
			{
				score += PIECE_EVAL[i];
				score += POSITION_VALUE[0][i][sq];
				board_material_equal_score += PIECE_VALUE_EQL_POSITIONS[i];
				if(i != 0)
					non_pawn_count_white++;
			}
		}
	}

	int game_duration_state_white = non_pawn_count_white < 4 ? 1 : 0;
	int game_duration_state_black = non_pawn_count_black < 4 ? 1 : 0;

	float king_score = POSITION_VALUE_KING[0][game_duration_state_white][board.kingSq(chess::Color::WHITE)];
	king_score -= POSITION_VALUE_KING[1][game_duration_state_black][board.kingSq(chess::Color::BLACK)];

	score += king_score;

	//evaluate with nnet
	if (board_material_equal_score == 0 && !board.inCheck())
	{
		value_nnet.rest_partial_forward_prop();
		//this is the slow option to do this.
		//leonardo_util::encode_pawn_matrix(board, input_matrix);
		//value_nnet.partial_forward_prop(input_matrix, input_matrix, vector3(0, 0, 0));
		float nnet_score =
			leonardo_util::get_pawn_matrix_value(value_nnet.get_output()) * //is around -1 and 1 (not guaranteed)
			1000;

		score += nnet_score;

		if (print_count > 0)
		{

			std::cout << "----------\n";
			std::cout << "nnet score: " << nnet_score << std::endl;

			std::cout << leonardo_util::get_pawn_structure_str(board);

			std::cout << "----------\n";
			print_count--;
		}


		leaf_nodes_evaluated_nnet++;
	}

	leaf_nodes++;

	return score;
}

float leonardo_value_bot_3::recursive_eval(
	int depth,
	int depth_addition,
	chess::Board& board,
	float alpha,
	float beta,
	chess::Move& best_move)
{
	nodes_visited++;

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	if (depth >= (end_depth + depth_addition))
		return eval(board, moves, depth);

	if (depth >= end_depth)
		depth_addition -= 1;

	bool maximizing = board.sideToMove() == chess::Color::WHITE;

	float best_score = maximizing ? -FLT_MAX : FLT_MAX;

	//sort_move_list(moves, board);

	if (moves.size() == 0 || board.isRepetition())
	{
		return eval(board, moves, depth);
	}
	for (chess::Move move : moves)
	{
		leonardo_util::make_move(board, input_matrix, move, value_nnet);
		//leonardo_util::make_move(b)
		int depth_bonus = 0;

		float score = recursive_eval(depth + 1, depth_addition + depth_bonus, board, alpha, beta, best_move);

		leonardo_util::unmake_move(board, input_matrix, move, value_nnet);

		if (maximizing)
		{
			if (score > best_score)
			{
				if (depth == 0)
					best_move = move;
				best_score = score;
			}

			alpha = std::max(alpha, score);
		}
		else
		{
			if (score < best_score)
			{
				if (depth == 0)
					best_move = move;
				best_score = score;
			}
			beta = std::min(beta, score);
		}
		if (depth == 0)
			std::cout << "move: " << chess::uci::moveToUci(move) << " score: " << score << "\n";
		if (beta <= alpha)
		{
			pruned++;
			break;
		}
	}

	return best_score;
}
void leonardo_value_bot_3::load_openings()
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
int leonardo_value_bot_3::get_opening_move(size_t hash)
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
leonardo_value_bot_3::leonardo_value_bot_3()
	: leonardo_value_bot_3(5)
{}
leonardo_value_bot_3::leonardo_value_bot_3(int end_depth)
	: end_depth(end_depth)
{
	load_openings();
	value_nnet = neural_network("nanopawn.parameters");
	//value_nnet.set_input_format(leonardo_util::get_pawn_input_format());
	//value_nnet.add_fully_connected_layer(32, leaky_relu_fn);
	//value_nnet.add_fully_connected_layer(16, leaky_relu_fn);
	//value_nnet.add_fully_connected_layer(8, leaky_relu_fn);
	//value_nnet.add_fully_connected_layer(leonardo_util::get_value_nnet_output_format(), identity_fn);

	input_matrix = matrix(leonardo_util::get_pawn_input_format());
}
void leonardo_value_bot_3::sort_move_list(chess::Movelist& moves, chess::Board& board)
{
	//throw std::logic_error("The method or operation is not implemented.");
	for (chess::Move& move : moves)
	{
		if (board.isCapture(move))
		{
			chess::PieceType from_type = board.at<chess::PieceType>(move.from());
			chess::PieceType to_type = board.at<chess::PieceType>(move.to());

			move.setScore(PIECE_EVAL[(int)to_type] - PIECE_EVAL[(int)from_type]);
		}
	}

	moves.sort();
}

chess::Move leonardo_value_bot_3::get_move(chess::Board& board)
{
	int opening_move_idx = get_opening_move(board.hash());
	if (opening_move_idx != -1)
	{
		std::cout << "opening move found\n\n";
		return openings[opening_move_idx].second;
	}

	pruned = 0;
	nodes_visited = 0;

	leaf_nodes = 0;
	leaf_nodes_evaluated_nnet = 0;

	print_count = 0;

	bool maximizing = board.sideToMove() == chess::Color::WHITE;
	chess::Move best_move = chess::Move::NULL_MOVE;

	leonardo_util::encode_pawn_matrix(board, input_matrix);

	auto start = std::chrono::high_resolution_clock::now();
	float score = recursive_eval(
		0,
		0,
		board,
		-FLT_MAX,
		FLT_MAX,
		best_move);
	auto end = std::chrono::high_resolution_clock::now();
	long long ms_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	if (board.sideToMove() == chess::Color::BLACK)
		score *= -1;


	std::cout
		<< "pruned: " << pruned << "\n"
		<< "leaf nodes: " << leaf_nodes << "\n"
		<< "leaf nodes evaluated by nnet: " << leaf_nodes_evaluated_nnet << "\n"
		<< "nnet eval leaf nodes: " << (leaf_nodes_evaluated_nnet / (float)std::max(1, leaf_nodes)) * 100.0f << "%\n"
		<< "nodes visited: " << nodes_visited << "\n"
		<< "time taken: " << ms_taken << "ms\n"
		<< "nodes/second: " << (nodes_visited / (ms_taken / 1000.0)) << "\n"
		<< "chosen move: " << chess::uci::moveToUci(best_move) << " "
		<< score << "\n";

	return best_move;
}
