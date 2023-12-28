#pragma once
//#define DEBUG_PRINT

#include "chess_player.hpp"
#include "NeuroFox/neural_network.hpp"
#include "stockfish_interface.hpp"
#include "nnet_table.hpp"
#include "leonardo_util.hpp"
//Credit to https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm

class leonardo_value_bot_2 : public chess_player
{
	static const int max_killer_ply = 32;
	typedef enum _TT_ITEM_TYPE : uint8_t
	{
		empty = 0,
		exact = 1,
		upper_bound = 2,
		lower_bound = 3
	} TT_ITEM_TYPE;

	class tt_item
	{
	public:
		static const int hashf_exact = 0;
		static const int hashf_alpha = 1;
		static const int hashf_beta = 2;

		static const int unknown_eval = -987654321;
		static const unsigned long long empty_key = 0xBEEF;

		unsigned long long key = empty_key;
		uint8_t depth = 0;
		TT_ITEM_TYPE flags = TT_ITEM_TYPE::empty;
		int value = unknown_eval;
		chess::Move best_move;
	};

	class killer_move
	{
	public:
		chess::Move moveA;
		chess::Move moveB;

		killer_move() :
			moveA(chess::Move::NULL_MOVE),
			moveB(chess::Move::NULL_MOVE)
		{}
		killer_move(chess::Move moveA, chess::Move moveB) : moveA(moveA), moveB(moveB) {}

		bool match(const chess::Move& other) const
		{
			return  moveA == other ||
				moveB == other;
		}

		void add(chess::Move& move)
		{
			if (move != moveA)
			{
				moveB = moveA;
				moveA = move;
			}
		}

		void reset()
		{
			moveA = chess::Move::NULL_MOVE;
			moveB = chess::Move::NULL_MOVE;
		}
	};

private:
	bool we_are_white = true;
	int pruned = 0;
	int nodes_visited = 0;
	int leaf_nodes = 0;
	int leaf_nodes_evaluated_nnet = 0;
	float nnet_mult = 0;
	int print_count = 0;
	int transpositions_count = 0;
	int tt_inserts = 0;
	int ms_per_move = -1;
	bool searched_at_least_one_move = false;
	int iterative_deepening_depth = 1;
	std::chrono::steady_clock::time_point start_time;
	int nmp_pruned = 0;
	killer_move killer_moves[max_killer_ply];
	//int nnet_mult = 1;
	int history[2][64][64] = { 0 };

	nnet_table pawn_nnet_table;
	chess::Bitboard pawn_w_bb;
	chess::Bitboard pawn_b_bb;

	//transposition table
	std::vector<tt_item> tt;

	std::vector<std::pair<size_t, chess::Move>> openings;

	neural_network value_nnet;
	matrix input_matrix;

	int probe_tt(chess::U64 hash, int ply, int depth, int alpha, int beta);
	const chess::Move& tt_get_move(chess::U64 hash);
	void record_tt(chess::U64 hash, int ply, int depth, int value, TT_ITEM_TYPE flags, const chess::Move& best_move);

	bool search_cancelled();

	void load_openings();

	int eval(chess::Board& board, chess::Movelist& moves, int ply, bool only_caputes_in_moves);

	bool stored_move_is_draw(chess::Board& board, int ply_from_root);

	void order_moves_quiescene(chess::Movelist& moves, chess::Board& board);
	int quiescene(chess::Board& board, int alpha, int beta);

	int search(
		bool is_pv,
		int ply_remaining,
		int ply_from_root,
		chess::Board& board,
		int alpha,
		int beta,
		chess::Move& best_move,
		bool allow_nmp
	);

	int get_opening_move(size_t hash);

	bool static_exchange_evaluation_better_than_threshold(chess::Board& board, chess::Move& move, int threshold);
	void sort_move_list(chess::Movelist& moves, chess::Board& board, int ply_from_root, int depth);

	void setup_nnet_for_move(const chess::Board& board);
public:
	leonardo_value_bot_2();
	leonardo_value_bot_2(int ms_per_move, float nnet_mult);

	chess::Move get_move(chess::Board& board) override;
	chess::Move get_move(chess::Board& board, int ms_left, std::string& info);
};
