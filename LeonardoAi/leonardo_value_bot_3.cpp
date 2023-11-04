#include "leonardo_value_bot_3.hpp"
#include "leonardo_util.hpp"

int leonardo_value_bot_3::probe_tt(chess::U64 hash, int depth_from_root, int alpha, int beta, chess::Move& best_move)
{
	tt_item& item = tt[hash % tt.size()];

	if (item.key == hash) //entry is acutally the same
	{
		if (item.depth >= depth_from_root)
		{
			if (item.flags == TT_ITEM_TYPE::exact)
				return item.value;

			if ((item.flags == TT_ITEM_TYPE::alpha) && (item.value <= alpha))
				return alpha;

			if ((item.flags == TT_ITEM_TYPE::beta) && (item.value >= beta))
				return beta;
		}

		//best_move = item.best_move;
	}

	return tt_item::unknown_eval;
}

void leonardo_value_bot_3::record_tt(
	chess::U64 hash,
	int depth_from_root,
	int value, 
	TT_ITEM_TYPE flags,
	const chess::Move& best_move)
{
	tt_item& item = tt[hash % tt.size()];

	item.key = hash;
	item.best_move = best_move;
	item.value = value;
	item.flags = flags;
	item.depth = depth_from_root;
}

bool leonardo_value_bot_3::time_over()
{
	std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
	long long ms_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();

	return false; //TEMP
	return ms_taken > ms_per_move;
}

//PAWN, KNIGHT, BISHOP, ROOK, QUEEN
const int PIECE_EVAL[5] = { 100.0f, 320.0f, 330.0f, 500.0f, 900.0f };
//helps determin if a position is equal if you only count piece values and skip pawns
const int PIECE_VALUE_EQL_POSITIONS[5] = { 100, 300.0f, 300.0f, 500.0f, 900.0f };

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

static inline bool is_passed_pawn(
	int idx,
	chess::Bitboard enemy_pawns,
	bool we_are_white)
{
	int file = idx % 8;
	int rank = idx / 8;
	static const chess::Bitboard file_a = 0x101010101010101;
	static const chess::Bitboard file_h = 0x8080808080808080;
	chess::Bitboard file_bb = file_a << file;
	chess::Bitboard right = (file_bb << 1) & 0xfefefefefefefefe;
	chess::Bitboard left = (file_bb >> 1) & 0x7f7f7f7f7f7f7f7f;

	chess::Bitboard rank_restriction_bb = 0xffffffffffffffff;
	rank_restriction_bb =
		we_are_white ?
		rank_restriction_bb << ((rank + 1) * 8) :
		rank_restriction_bb >> (((7 - rank) + 1) * 8);

	chess::Bitboard mask = rank_restriction_bb & (left | file_bb | right);

	return (enemy_pawns & mask) == 0;
}

static inline chess::Bitboard get_pawn_attack_bb(chess::Bitboard pawn_board, bool is_white)
{
	return is_white ?
		((pawn_board << 7) & 0x7f7f7f7f7f7f7f7f) |
		((pawn_board << 9) & 0xfefefefefefefefe) :
		((pawn_board >> 7) & 0xfefefefefefefefe) |
		((pawn_board >> 9) & 0x7f7f7f7f7f7f7f7f);
}

static inline int manhatten_distance(int idx1, int idx2)
{
	int x1 = idx1 % 8;
	int y1 = idx1 / 8;
	int x2 = idx2 % 8;
	int y2 = idx2 / 8;
	return abs(x1 - x2) + abs(y1 - y2);
}

static inline int is_double_pawn(int sq, chess::Bitboard our_pawns)
{
	chess::Bitboard file_mask = 0x101010101010101 << (sq % 8);
	file_mask = ~(1ULL << sq) & file_mask;
	return (our_pawns & file_mask) != 0 ? 1 : 0;
}

int leonardo_value_bot_3::eval(chess::Board& board, chess::Movelist& moves, int depth)
{
	int score = 0.0f;

	std::pair<chess::GameResultReason, chess::GameResult> res = board.isGameOver(moves);
	if (res.first != chess::GameResultReason::NONE)
	{
		int val = 0.0f;
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

	const chess::Bitboard black_bb = board.us(chess::Color::BLACK);
	const chess::Bitboard white_bb = board.us(chess::Color::WHITE);
	const chess::Bitboard pawns_bb = board.pieces(chess::PieceType::PAWN);

	const chess::Bitboard white_pawn_attack_bb = get_pawn_attack_bb(pawns_bb & white_bb, true);
	const chess::Bitboard black_pawn_attack_bb = get_pawn_attack_bb(pawns_bb & black_bb, false);

	int double_pawn_count = 0;
	int non_pawn_count_white = 0;
	int non_pawn_count_black = 0;
	int passed_pawn_count = 0;
	int covered_pawn_count = 0;
	int board_material_equal_score = 0;
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
				board_material_equal_score -= PIECE_VALUE_EQL_POSITIONS[i];
				if (i == 0)
				{
					passed_pawn_count -= is_passed_pawn(sq, white_bb & pawns_bb, false);
					covered_pawn_count -= (black_pawn_attack_bb & curr_bb) != 0;
					double_pawn_count += is_double_pawn(sq, black_bb & pawns_bb); //double pawns are bad
				}
				else {
					non_pawn_count_black++;
				}
			}
			else //white
			{
				score += PIECE_EVAL[i];
				score += POSITION_VALUE[0][i][sq];
				board_material_equal_score += PIECE_VALUE_EQL_POSITIONS[i];
				if (i == 0)
				{
					passed_pawn_count += is_passed_pawn(sq, black_bb & pawns_bb, true);
					covered_pawn_count += (white_pawn_attack_bb & curr_bb) != 0;
					double_pawn_count -= is_double_pawn(sq, white_bb & pawns_bb);
				}
				else
				{
					non_pawn_count_white++;
				}
			}
		}
	}

	int game_duration_state_white = non_pawn_count_white < 3 ? 1 : 0;
	int game_duration_state_black = non_pawn_count_black < 3 ? 1 : 0;

	int king_score = POSITION_VALUE_KING[0][game_duration_state_white][board.kingSq(chess::Color::WHITE)];
	king_score -= POSITION_VALUE_KING[1][game_duration_state_black][board.kingSq(chess::Color::BLACK)];


	//if the other king has a significant piece and you dont
	//stay away from them
	if (non_pawn_count_black == 0 && non_pawn_count_white > 0)
	{
		int king_manhatten_distance = manhatten_distance(
			board.kingSq(chess::Color::WHITE),
			board.kingSq(chess::Color::BLACK));

		score += king_manhatten_distance / 2;
	}
	else if (non_pawn_count_white == 0 && non_pawn_count_black > 0)
	{
		int king_manhatten_distance = manhatten_distance(
			board.kingSq(chess::Color::WHITE),
			board.kingSq(chess::Color::BLACK));

		score -= king_manhatten_distance / 2;
	}

	score += covered_pawn_count * 5.0f;
	score += passed_pawn_count * 50.0f;
	//double pawns get counted twice. triple pawns are not that much more important
	score += ((float)double_pawn_count / 2) * 15.0f;
	score += king_score;

	//evaluate with nnet
	if (use_nnet)//if (board_material_equal_score == 0 && !board.inCheck())
	{
		int nnet_score = leonardo_util::get_board_val(
			pawns_bb & white_bb,
			pawns_bb & black_bb,
			pawn_w_bb,
			pawn_b_bb,
			value_nnet,
			input_matrix,
			pawn_nnet_table);

		score += nnet_score;

		if (print_count > 0)
		{

			std::cout << "----------\n";
			std::cout << "all: " << score << std::endl;
			std::cout << "nnet score: " << nnet_score << std::endl;

			std::cout << leonardo_util::get_pawn_structure_str(board);

			std::cout << "----------\n";
			print_count--;
		}


		leaf_nodes_evaluated_nnet++;
	}

	leaf_nodes++;

	return score / 100;
}

int leonardo_value_bot_3::recursive_eval(
	int ply_remaining,
	int ply_from_root,
	chess::Board& board,
	float alpha,
	float beta,
	chess::Move& best_move)
{
	if (time_over())
		return 0;

	nodes_visited++;

	TT_ITEM_TYPE tt_flag = TT_ITEM_TYPE::alpha;
	//int value = probe_tt(board.hash(), ply_from_root, alpha, beta, best_move);
	//if(value != tt_item::unknown_eval)
	//{
		transpositions_count++;
		//return value;
	//}

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	if (ply_remaining == 0)
	{
		int evaluation = eval(board, moves, ply_from_root);
		//record_tt(board.hash(), ply_from_root, evaluation, TT_ITEM_TYPE::exact, best_move);
		return evaluation;
	}

	bool maximizing = board.sideToMove() == chess::Color::WHITE;

	int best_score = maximizing ? INT_MIN : INT_MAX;

	if (moves.size() == 0 || board.isRepetition())
	{
		return eval(board, moves, ply_from_root);
	}

	//sort_move_list(moves, board);

	for (chess::Move move : moves)
	{
		//leonardo_util::make_move(board, input_matrix, move, value_nnet);
		board.makeMove(move);
		float score = recursive_eval(ply_remaining - 1, ply_from_root + 1, board, alpha, beta, best_move);
		board.unmakeMove(move);
		//leonardo_util::unmake_move(board, input_matrix, move, value_nnet);

		if (time_over())
			return best_score;

		if (maximizing)
		{
			if (score > best_score)
			{
				if (ply_from_root == 0)
					best_move = move;
				best_score = score;
			}
			
			if(alpha < score)
			{
				alpha = score;
				tt_flag = TT_ITEM_TYPE::alpha;
			}
			//alpha = std::max(alpha, score);
		}
		else
		{
			if (score < best_score)
			{
				if (ply_from_root == 0)
					best_move = move;
				best_score = score;
			}

			if(beta > score)
			{
				beta = score;	
				//record_tt(board.hash(), ply_from_root, score, TT_ITEM_TYPE::beta, best_move);
			}
			//beta = std::min(beta, score);
		}

		//tt_store(board, depth, score);

		if (beta <= alpha)
		{
			pruned++;
			break;
		}
	}

	//record_tt(board.hash(), ply_from_root, alpha, tt_flag, best_move);

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
	: leonardo_value_bot_3(5, false)
{}
leonardo_value_bot_3::leonardo_value_bot_3(int ms_per_move, bool given_use_nnet)
	: ms_per_move(ms_per_move), 
	use_nnet(given_use_nnet), 
	pawn_nnet_table(500) //200mb table
{
	load_openings();
	value_nnet = neural_network("nanopawn.parameters");
	input_matrix = matrix(leonardo_util::get_pawn_input_format());
}
void leonardo_value_bot_3::sort_move_list(chess::Movelist& moves, chess::Board& board)
{
	/*
	for (chess::Move& move : moves)
	{
		board.makeMove(move);
		size_t hash = board.hash();
		board.unmakeMove(move);
		tt_item* stored_tt_item = tt_get(board, INT_MAX);
		if (stored_tt_item != nullptr)
		{
			float val = stored_tt_item->value * 1000;
			val *= board.sideToMove() == chess::Color::WHITE ? 1 : -1;
			move.setScore(stored_tt_item->value * 1000); //always in front of the capture moves
		}
		else if (board.isCapture(move))
		{
			chess::PieceType from_type = board.at<chess::PieceType>(move.from());
			chess::PieceType to_type = board.at<chess::PieceType>(move.to());

			move.setScore((PIECE_EVAL[(int)to_type] - PIECE_EVAL[(int)from_type]) / 100);
		}
	}

	moves.sort();*/
}

void leonardo_value_bot_3::setup_nnet_for_move(const chess::Board& board)
{
	if(!use_nnet)
		return;

	const chess::Bitboard black_bb = board.us(chess::Color::BLACK);
	const chess::Bitboard white_bb = board.us(chess::Color::WHITE);
	const chess::Bitboard pawns_bb = board.pieces(chess::PieceType::PAWN);

	leonardo_util::encode_pawn_matrix(board, input_matrix);
	pawn_w_bb = white_bb & pawns_bb;
	pawn_b_bb = black_bb & pawns_bb;

	value_nnet.forward_propagation(input_matrix);
}

chess::Move leonardo_value_bot_3::get_move(chess::Board& board)
{
	use_nnet = true; //DEBUG
	int opening_move_idx = get_opening_move(board.hash());
	if (opening_move_idx != -1)
	{
#ifdef DEBUG_PRINT
		std::cout << "opening move found\n\n";
#endif
		return openings[opening_move_idx].second;
	}

	const size_t tt_item_size = sizeof(tt_item); // in byte
	const size_t tt_desired_size = 200; // in MB
	const size_t tt_size = tt_desired_size * 1024 * 1024 / tt_item_size;
	//tt.resize(tt_size);

	pruned = 0;
	nodes_visited = 0;

	leaf_nodes = 0;
	leaf_nodes_evaluated_nnet = 0;
	start_time = std::chrono::high_resolution_clock::now();

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	print_count = 0;
	transpositions_count = 0;

	bool maximizing = board.sideToMove() == chess::Color::WHITE;

	setup_nnet_for_move(board);

	int score = 0;
	auto start = std::chrono::high_resolution_clock::now();

	int reached_depth = 0;
	chess::Move best_move = chess::Move::NULL_MOVE;
	int transpositions_last = 0;
	for (int search_depth = 1; !time_over(); search_depth++)
	{
		//tt.clear();
		//tt.resize(tt_size); //inefficient
		transpositions_count = 0;
		chess::Move tmp = chess::Move::NULL_MOVE;

		score = recursive_eval(
			search_depth,
			0,
			board,
			INT_MIN,
			INT_MAX,
			tmp);

		if (!time_over())
		{
			reached_depth = search_depth;
			best_move = tmp;
			transpositions_last = transpositions_count;
		}
		if (search_depth == 5) break; //DEBUG
	}
	transpositions_count = transpositions_last;

	auto end = std::chrono::high_resolution_clock::now();
	long long ms_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	if (board.sideToMove() == chess::Color::BLACK)
		score *= -1;

	std::cout << "use nnet: " << use_nnet << "\n";
	std::cout << "transposition table size: " << tt.size()
		<< " | transpositions: " << transpositions_count
		<< "\n";
	std::cout << "ms: " << ms_taken << "\n";
	std::cout << "moves/ms " << tt.size() / (ms_taken + 1) << "\n";
	std::cout << "reached_depth: " << reached_depth << "\n";
	std::cout << "nodes: " << nodes_visited << "\n";
	std::cout << "----------------\n";
#ifdef DEBUG_PRINT
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
#endif
	//tt.clear();

	return best_move;
}
