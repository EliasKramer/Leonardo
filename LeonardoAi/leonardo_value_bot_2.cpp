#include "leonardo_value_bot_2.hpp"
#include "leonardo_util.hpp"
#include <filesystem>

#define MATE_SCORE 10000000
#define DRAW_SCORE		  0
#define MAX_DEPTH		256
#define MIN_DEPTH		  5

static bool is_mate_score(int score)
{
	return std::abs(score) >= MATE_SCORE - MAX_DEPTH;
}

static int store_corrected_score(int score, int ply)
{
	if (is_mate_score(score)) // is mate 
	{
		//ply from root to checkmate node
		int all_ply = std::abs(MATE_SCORE - std::abs(score));

		//ply from current node to checkmate node
		int corrected_ply = all_ply - ply;

		//we need to save the difference
		int sign = score > 0 ? 1 : -1;
		score = sign * (MATE_SCORE - corrected_ply);
	}

	return score;
}

static int retrieve_corrected_score(int score, int ply)
{
	if (is_mate_score(score)) // is mate 
	{
		//ply difference from curr node to checkmate node
		int ply_before_mate_found = std::abs(MATE_SCORE - std::abs(score));

		//we saved the difference. now we need to add it to the current ply
		int actual_ply = ply_before_mate_found + ply;

		//we add the difference to the score
		int sign = score > 0 ? 1 : -1;
		score = sign * (MATE_SCORE - actual_ply);
	}

	return score;
}

int leonardo_value_bot_2::probe_tt(chess::U64 hash, int ply, int depth, int alpha, int beta)
{
	//mate ply 4 depth 6 current ply 2
	//mate ply 3 depth 6 current ply 2
	tt_item::unknown_eval;

	tt_item& item = tt[hash % tt.size()];

	if (item.key == hash) //entry is acutally the same
	{
		if (item.depth >= depth)
		{
			int corrected_value = retrieve_corrected_score(item.value, ply);

			if (item.flags == TT_ITEM_TYPE::exact)
			{
				return corrected_value;
			}

			// We have stored the upper bound of the eval for this position. If it's less than alpha then we don't need to
			// search the moves in this position as they won't interest us; otherwise we will have to search to find the exact value
			// the current position is less worth, than the best available move - skip
			if ((item.flags == TT_ITEM_TYPE::upper_bound) && (corrected_value <= alpha))
			{
				return alpha;
			}

			// We have stored the lower bound of the eval for this position. Only return if it causes a beta cut-off.
			// beta cut-off means, that the opponent will not allow us to reach this position, because they have a better move
			if ((item.flags == TT_ITEM_TYPE::lower_bound) && (corrected_value >= beta))
			{
				return beta;
			}
		}
	}

	return tt_item::unknown_eval;
}

const chess::Move& leonardo_value_bot_2::tt_get_move(chess::U64 hash)
{
	tt_item& item = tt[hash % tt.size()];

	if (item.key == hash)
	{
		return item.best_move;
	}

	return chess::Move::NULL_MOVE;
}

void leonardo_value_bot_2::record_tt(
	chess::U64 hash,
	int ply,
	int depth,
	int value,
	TT_ITEM_TYPE flags,
	const chess::Move& best_move)
{
	tt_item& item = tt[hash % tt.size()];

	item.key = hash;
	item.best_move = best_move;
	item.value = store_corrected_score(value, ply);
	item.flags = flags;
	item.depth = depth;
	tt_inserts++;
}

bool leonardo_value_bot_2::search_cancelled()
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

static inline bool is_passed_pawn(
	int idx,
	chess::Bitboard enemy_pawns,
	bool we_are_white)
{
	int file = idx % 8;
	int rank = idx / 8;
	constexpr chess::Bitboard file_a = 0x101010101010101;
	constexpr chess::Bitboard file_h = 0x8080808080808080;
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

int leonardo_value_bot_2::eval(chess::Board& board, chess::Movelist& moves, int ply, bool only_caputes_in_moves)
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


	//return score * side_mult;

	int game_duration_state_white = non_pawn_count_white < 3 ? 1 : 0;
	int game_duration_state_black = non_pawn_count_black < 3 ? 1 : 0;

	int king_score = POSITION_VALUE_KING[0][game_duration_state_white][board.kingSq(chess::Color::WHITE)];
	king_score -= POSITION_VALUE_KING[1][game_duration_state_black][board.kingSq(chess::Color::BLACK)];

	/*
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
	*/
	score += covered_pawn_count * 5.0f;
	score += passed_pawn_count * 50.0f;
	//double pawns get counted twice. triple pawns are not that much more important
	score += ((float)double_pawn_count / 2) * 15.0f;
	score += king_score;

	//evaluate with nnet
	if (nnet_mult > 0)//if (board_material_equal_score == 0 && !board.inCheck())
	{
		int nnet_score = leonardo_util::get_board_val(
			pawns_bb & white_bb,
			pawns_bb & black_bb,
			pawn_w_bb,
			pawn_b_bb,
			value_nnet,
			input_matrix,
			pawn_nnet_table,
			board.sideToMove() == chess::Color::WHITE);

		score += ((float)nnet_score * nnet_mult);

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

	//leaf_nodes++;

	return score * side_mult;
}

bool leonardo_value_bot_2::stored_move_is_draw(chess::Board& board, int ply_from_root)
{
	if (ply_from_root != 0)
	{
		return false;
	}

	const chess::Move& cached_move = tt_get_move(board.hash());

	if (cached_move == chess::Move::NULL_MOVE)
	{
		return false;
	}

	board.makeMove(cached_move);
	//std::cout << "is draw: " << board.isHalfMoveDraw() << " is rep: " << board.isRepetition() << "\n";
	bool ret_val = board.isHalfMoveDraw() || board.isRepetition() || board.isInsufficientMaterial();
	board.unmakeMove(cached_move);

	return ret_val;
}

void leonardo_value_bot_2::order_moves_quiescene(chess::Movelist& moves, chess::Board& board)
{
	for (chess::Move& m : moves)
	{
		//mvv/lva
		https://github.com/kobolabs/stockfish/blob/be470063cea1283ed53c4b74a03b96a7cc7acf6c/movepick.cpp#L147C10-L147C10
		if (static_exchange_evaluation_better_than_threshold(board, m, 0))
		{
			//if to() is a queen and from() is a pawn then +800
			//if to() is a pawn and from() is a queen then -800
			m.setScore(
				PIECE_EVAL[(int)board.at(m.to())] -
				PIECE_EVAL[(int)board.at(m.from())]
			);
		}
		else
		{
			m.setScore(-1000000);
		}
	}

	moves.sort();
}

int leonardo_value_bot_2::quiescene(chess::Board& board, int alpha, int beta)
{
	nodes_visited++;
	chess::Movelist moves;
	chess::movegen::legalmoves<chess::MoveGenType::CAPTURE>(moves, board);

	int score = eval(board, moves, MAX_DEPTH, true);
	if (board.isInsufficientMaterial() || board.isRepetition() || board.isHalfMoveDraw())
	{
		return score;
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

	order_moves_quiescene(moves, board);

	for (chess::Move& move : moves)
	{
		if (!board.inCheck() &&
			move.score() == -1000000) // SEE in move ordering to avoid bad captures
		{
			continue;
		}

		board.makeMove(move);
		score = -quiescene(board, -beta, -alpha);
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

int leonardo_value_bot_2::search(
	bool is_pv,
	int ply_remaining,
	int ply_from_root,
	chess::Board& board,
	int alpha, // alpha is the best value that the maximizing player can guarantee at that level or above.
	int beta, // beta is the best value that the minimizing player can guarantee at that level or above.
	chess::Move& best_move,
	bool allow_nmp
)
{
	if (search_cancelled())
		return 0;

	nodes_visited++;

	TT_ITEM_TYPE tt_flag = TT_ITEM_TYPE::upper_bound;
	int value = probe_tt(board.hash(), ply_from_root, ply_remaining, alpha, beta);
	if (value != tt_item::unknown_eval
		&& !board.isRepetition()
		&& !board.isHalfMoveDraw()
		&& !board.isInsufficientMaterial()
		&& !stored_move_is_draw(board, ply_from_root))
	{
		if (ply_from_root == 0)
		{
			//bool stuff = stored_move_is_draw(board, ply_from_root);
			//std::cout << "stored move is rep: " << stuff << "\n";
			best_move = tt_get_move(board.hash());
		}

		transpositions_count++;
		return value;
	}

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	if (moves.size() == 0 ||
		board.isRepetition() ||
		board.isHalfMoveDraw() ||
		board.isInsufficientMaterial())
	{
		leaf_nodes++;
		return eval(board, moves, ply_from_root, false);
	}

	if (ply_remaining <= 0)
	{
		leaf_nodes++;
		return quiescene(board, alpha, beta);
	}

	if (allow_nmp &&
		!is_pv &&
		ply_from_root > 1 &&
		ply_remaining >= 3 &&
		!board.inCheck()) // nmp - null move pruning
	{
		board.makeNullMove();

		//the score represents the value of the position after the null move
		int score = -search(false, ply_remaining - 3 - (ply_remaining / 3), ply_from_root + 1, board, -beta, -alpha, best_move, false);

		board.unmakeNullMove();

		//if we get a score that causes a cutoff even after skipping a move
		//we can assume that our advantage is so high, that we can just return that score
		if (score >= beta)
		{
			nmp_pruned++;
			return beta;
		}
	}

	sort_move_list(moves, board, ply_from_root, ply_remaining);
	chess::Move& best_current_move = moves[0];
	chess::Move pv = tt_get_move(board.hash());
	for (chess::Move move : moves)
	{
		if (pv != chess::Move::NULL_MOVE &&
			move != pv &&
			!board.inCheck() &&
			board.isCapture(move) &&
			ply_from_root > 1 && // maybe >= 1 ?
			!static_exchange_evaluation_better_than_threshold(board, move, -20 * ply_from_root))
		{
			/*
			std::cout << "SEE PRUNING\n";
			std::cout << board << "\n";
			std::cout << move << "\n";
			std::cout << "val: " << move.score() << "\n";
			std::cout << "threshold: " << -20 * ply_remaining << "\n";
			std::cout << "------------\n";
			*/
			continue;
		}

		board.makeMove(move);
		int score = -search(move == pv, ply_remaining - 1, ply_from_root + 1, board, -beta, -alpha, best_move, true);
		board.unmakeMove(move);

		if (search_cancelled())
			return 0;

		//current score is better than our opponent can achieve. our opponent will not pick this path
		if (score >= beta)
		{
			//store lower bound
			record_tt(board.hash(), ply_from_root, ply_remaining, beta, TT_ITEM_TYPE::lower_bound, move); // BEST LOCAL MOVE
			pruned++;

			if (!board.isCapture(move) && move.typeOf() != chess::Move::PROMOTION)
			{
				if (ply_from_root < max_killer_ply)
				{
					killer_moves[ply_from_root].add(move);
				}
				history[(int)board.sideToMove()][move.from()][move.to()] += ply_remaining * 2;
			}
			//maye killer move

			return beta;
		}
		if (score > alpha)
		{
			alpha = score;
			tt_flag = TT_ITEM_TYPE::exact;
			if (ply_from_root == 0)
			{
				best_move = move;
				searched_at_least_one_move = true;
			}
			best_current_move = move;
		}
	}

	record_tt(board.hash(), ply_from_root, ply_remaining, alpha, tt_flag, best_current_move);

	return alpha;
}

void leonardo_value_bot_2::load_openings()
{
	std::ifstream file("openings.txt");

	if (!file)
	{
		std::cout << "opening file not found\n";
		std::cout << "current dir: " << std::filesystem::current_path() << "\n";

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
int leonardo_value_bot_2::get_opening_move(size_t hash)
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
leonardo_value_bot_2::leonardo_value_bot_2()
	: leonardo_value_bot_2(5, false)
{}
leonardo_value_bot_2::leonardo_value_bot_2(int ms_per_move, float nnet_mult)
	: ms_per_move(ms_per_move),
	nnet_mult(nnet_mult),
	pawn_nnet_table(500) //in megabyte
{
	load_openings();

	value_nnet = neural_network("128_64_32_32_three_layers.parameters");
	/*
	value_nnet.set_input_format(leonardo_util::get_pawn_input_format());
	value_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	value_nnet.add_fully_connected_layer(32, leaky_relu_fn);
	value_nnet.add_fully_connected_layer(16, leaky_relu_fn);
	value_nnet.add_fully_connected_layer(leonardo_util::get_value_nnet_output_format(), identity_fn);
	value_nnet.xavier_initialization();
	*/
	//std::cout << value_nnet.parameter_analysis();
	input_matrix = matrix(leonardo_util::get_pawn_input_format());

	const size_t tt_item_size = sizeof(tt_item); // in byte
	const size_t tt_desired_size = 200; // in MB
	const size_t tt_size = tt_desired_size * 1024 * 1024 / tt_item_size;
	tt.resize(10048583);// tt_size + 123);
}

static int estimate_capture_move_value(chess::Board& board, chess::Move& move)
{
	if (board.isCapture(move))
	{
		switch (move.typeOf())
		{
		case chess::Move::ENPASSANT:
			return SEE_PIECE_EVAL[(int)chess::PieceType::PAWN];
		case chess::Move::PROMOTION:
			return SEE_PIECE_EVAL[(int)move.promotionType()];
		default:
			return SEE_PIECE_EVAL[(int)board.at<chess::PieceType>(move.to())];
		}
	}
	else {
		return 0;
	}
}

static uint64_t allAttackersToSquare(chess::Board& board, uint64_t occupied, chess::Square sq)
{
	return
		(chess::attacks::pawn(chess::Color::WHITE, sq) & board.us(chess::Color::BLACK) & board.pieces(chess::PieceType::PAWN)) |
		(chess::attacks::pawn(chess::Color::BLACK, sq) & board.us(chess::Color::WHITE) & board.pieces(chess::PieceType::PAWN)) |
		(chess::attacks::knight(sq) & board.pieces(chess::PieceType::KNIGHT)) |
		(chess::attacks::bishop(sq, occupied) & (board.pieces(chess::PieceType::BISHOP) | board.pieces(chess::PieceType::QUEEN))) |
		(chess::attacks::rook(sq, occupied) & (board.pieces(chess::PieceType::ROOK) | board.pieces(chess::PieceType::QUEEN))) |
		(chess::attacks::king(sq) & board.pieces(chess::PieceType::KING));
}

//cedits to https://github.com/AndyGrant/Ethereal/blob/master/src/search.c#L916
bool leonardo_value_bot_2::static_exchange_evaluation_better_than_threshold(chess::Board& board, chess::Move& move, int threshold)
{
	uint64_t bishops, rooks, occupied, attackers, my_attackers;

	chess::Square from = move.from();
	chess::Square to = move.to();
	uint16_t type = move.typeOf();

	// Next victim is moved piece or promotion type
	//its type is a piecetype, but we need the ++ operator. touching the chess engine could break stome stuff xd
	int next_victim = (int)(type != chess::Move::PROMOTION
		? board.at<chess::PieceType>(move.from())
		: move.promotionType());

	// Balance is the value of the move minus threshold. Function
	// call takes care for Enpass, Promotion (and Castling moves.) castling moves are not captures?
	int balance = estimate_capture_move_value(board, move) - threshold;

	// Best case still fails to beat the threshold
	if (balance < 0) return false;

	// Worst case is losing the moved piece
	balance -= SEE_PIECE_EVAL[next_victim];

	// If the balance is positive even if losing the moved piece,
	// the exchange is guaranteed to beat the threshold.
	if (balance >= 0) return true;

	// Grab sliders for updating revealed attackers
	bishops = board.pieces(chess::PieceType::BISHOP) | board.pieces(chess::PieceType::QUEEN);
	rooks = board.pieces(chess::PieceType::ROOK) | board.pieces(chess::PieceType::QUEEN);

	// Let occupied suppose that the move was actually made
	occupied = board.occ();
	occupied = (occupied ^ (1ull << from)) | (1ull << to);
	if (type == chess::Move::ENPASSANT) occupied ^= (1ull << board.enpassantSq());

	// Get all pieces which attack the target square. And with occupied
	// so that we do not let the same piece attack twice
	attackers = allAttackersToSquare(board, occupied, to) & occupied;

	// Now our opponents turn to recapture
	chess::Color color = ~board.sideToMove();

	while (1) {

		// If we have no more attackers left we lose
		my_attackers = attackers & board.us(color);
		if (my_attackers == 0ull) break;

		// Find our weakest piece to attack with
		for (next_victim = (int)chess::PieceType::PAWN; next_victim <= (int)chess::PieceType::QUEEN; next_victim++)
			if (my_attackers & board.pieces((chess::PieceType)next_victim))
				break;

		// Remove this attacker from the occupied
		chess::builtin::lsb(my_attackers);
		occupied ^= (1ull << chess::builtin::lsb(my_attackers & board.pieces((chess::PieceType)next_victim)));

		// A diagonal move may reveal bishop or queen attackers
		if (next_victim == (int)chess::PieceType::PAWN || next_victim == (int)chess::PieceType::BISHOP || next_victim == (int)chess::PieceType::QUEEN)
			attackers |= chess::attacks::bishop(to, occupied) & bishops;

		// A vertical or horizontal move may reveal rook or queen attackers
		if (next_victim == (int)chess::PieceType::ROOK || next_victim == (int)chess::PieceType::QUEEN)
			attackers |= chess::attacks::rook(to, occupied) & rooks;

		// Make sure we did not add any already used attacks
		attackers &= occupied;

		// Swap the turn
		color = ~color;

		// Negamax the balance and add the value of the next victim
		balance = -balance - 1 - SEE_PIECE_EVAL[next_victim];

		// If the balance is non negative after giving away our piece then we win
		if (balance >= 0) {

			// As a slide speed up for move legality checking, if our last attacking
			// piece is a king, and our opponent still has attackers, then we've
			// lost as the move we followed would be illegal
			if (next_victim == (int)chess::PieceType::KING && (attackers & board.us(color)))
				color = ~color;

			break;
		}
	}

	// Side to move after the loop loses
	return board.sideToMove() != color;
}

void leonardo_value_bot_2::sort_move_list(chess::Movelist& moves, chess::Board& board, int ply_from_root, int depth)
{
	chess::Move tt_move = tt_get_move(board.hash());

	for (chess::Move& move : moves)
	{
		if (move == tt_move)
		{
			move.setScore(1000000);
		}
		else if (move.typeOf() == chess::Move::PROMOTION)
		{
			//1000 to 9000
			move.setScore(PIECE_EVAL[(int)move.promotionType()] * 10);
		}
		else if (board.isCapture(move))
		{
			move.setScore(
				static_exchange_evaluation_better_than_threshold(board, move, depth * -20) ?
				3000 : // good capture bonus
				-1000 //bad capture bonus
			);
		}
		else
		{
			int score = 0;
			if (board.isAttacked(move.to(), ~board.sideToMove()))
			{
				//discourages every move, that wants to move to a field, that is attacked
				chess::PieceType from_piece_type = board.at<chess::PieceType>(move.from());
				score -= PIECE_EVAL[(int)from_piece_type] / 10;
			}

			//if a move caused a cutoff previously, it is likely to be a good move now
			bool is_killer = ply_from_root < max_killer_ply && killer_moves[ply_from_root].match(move);
			score += is_killer ? 2000 : 0;

			score += history[(int)board.sideToMove()][move.from()][move.to()];

			move.setScore(score);
		}
	}

	moves.sort();
}

void leonardo_value_bot_2::setup_nnet_for_move(const chess::Board& board)
{
	if (nnet_mult == 0)
		return;

	const chess::Bitboard black_bb = board.us(chess::Color::BLACK);
	const chess::Bitboard white_bb = board.us(chess::Color::WHITE);
	const chess::Bitboard pawns_bb = board.pieces(chess::PieceType::PAWN);

	leonardo_util::encode_pawn_matrix(board, input_matrix);
	pawn_w_bb = white_bb & pawns_bb;
	pawn_b_bb = black_bb & pawns_bb;

	value_nnet.forward_propagation(input_matrix);
}

chess::Move leonardo_value_bot_2::get_move(chess::Board& board)
{
	std::string info = "";
	return get_move(board, ms_per_move, info);
}

chess::Move leonardo_value_bot_2::get_move(chess::Board& board, int ms_left, std::string& info)
{
	ms_per_move = ms_left;
	we_are_white = board.sideToMove() == chess::Color::WHITE;

	int opening_move_idx = get_opening_move(board.hash());
	if (opening_move_idx != -1)
	{
		return openings[opening_move_idx].second;
	}

	tt_inserts = 0;
	pruned = 0;
	nodes_visited = 0;

	leaf_nodes = 0;
	leaf_nodes_evaluated_nnet = 0;
	start_time = std::chrono::high_resolution_clock::now();
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	//std::cout << board << "\n";

	print_count = 0;
	transpositions_count = 0;

	//maybe reallocate this on the heap
	//could be faster
	for (int c = 0; c < 2; c++)
	{
		for (int i = 0; i < 64; i++)
		{
			for (int j = 0; j < 64; j++)
			{
				history[c][i][j] = 0;
			}
		}
	}
	for (int i = 0; i < max_killer_ply; i++)
	{
		killer_moves[i].reset();
	}

	bool maximizing = board.sideToMove() == chess::Color::WHITE;

	setup_nnet_for_move(board);

	int score = 0;


	int reached_depth = 0;
	chess::Move best_move = chess::Move::NULL_MOVE;
	int transpositions_last = 0;
	nmp_pruned = 0;
	bool partial = false;
	for (iterative_deepening_depth = 1; !search_cancelled() && iterative_deepening_depth < MAX_DEPTH; iterative_deepening_depth++)
	{

		searched_at_least_one_move = false;

		transpositions_count = 0;
		chess::Move tmp = chess::Move::NULL_MOVE;

		score = search(
			true,
			iterative_deepening_depth,
			0,
			board,
			-1000000000,
			1000000000,
			tmp,
			true);

		if (!search_cancelled())
		{
			reached_depth = iterative_deepening_depth;
			best_move = tmp;
			transpositions_last = transpositions_count;
			/*
			std::cout
				<< "d " << iterative_deepening_depth
				<< " best_move: " << chess::uci::moveToUci(best_move)
				<< " score: " << score << "\n";
			std::cout << "--------------d: " << iterative_deepening_depth << "\n";
			std::cout << "--------------\n";


			auto end = std::chrono::high_resolution_clock::now();
			long long ms_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			std::cout << "time: " << ms_taken << "ms\n";
			*/
		}
		else
		{
			if (searched_at_least_one_move)
			{
				best_move = tmp;
				partial = true;
				/*
				std::cout
					<< "partial search d " << iterative_deepening_depth
					<< " best_move: " << chess::uci::moveToUci(best_move)
					<< " score: " << score << "\n";
				std::cout << "--------------d: " << iterative_deepening_depth << "\n";
				std::cout << "score: " << score << "\n";
				std::cout << "--------------\n";
				*/
			}
		}
	}
	auto stop = std::chrono::high_resolution_clock::now();
	long long ms_taken = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start_time).count();

	info = "depth reached: " + std::to_string(iterative_deepening_depth) + (partial ? " partial\n" : "\n") +
		"nodes visited: " + std::to_string(nodes_visited) + "\n" +
		"time: " + std::to_string(ms_taken) + "ms\n";

	transpositions_count = transpositions_last;

	if (best_move == chess::Move::NULL_MOVE)
	{
		best_move = moves[0];
		std::cout << "move was null\n";
	}

	return best_move;
}
