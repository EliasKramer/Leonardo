#pragma once
//#define DEBUG_PRINT

#include "chess_player.hpp"
#include "NeuroFox/neural_network.hpp"
#include "stockfish_interface.hpp"

//Credit to https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm

typedef enum _TT_ITEM_TYPE : uint8_t
{
	empty = 0,
	exact = 1,
	alpha = 2,
	beta = 3
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

class leonardo_value_bot_3 : public chess_player
{
private:
	int pruned = 0;
	int nodes_visited = 0;

	int leaf_nodes = 0;
	int leaf_nodes_evaluated_nnet = 0;

	bool use_nnet = false;
	int print_count = 0;

	int transpositions_count = 0;

	int ms_per_move = 100;
	std::chrono::steady_clock::time_point start_time;

	//transposition table
	std::vector<tt_item> tt;

	std::vector<std::pair<size_t, chess::Move>> openings;

	neural_network value_nnet;
	matrix input_matrix;

	int probe_tt(chess::U64 hash, int depth_from_root, int alpha, int beta, chess::Move& best_move);
	void record_tt(chess::U64 hash, int depth_from_root, int value, TT_ITEM_TYPE flags, const chess::Move& best_move);

	bool time_over();

	void load_openings();

	int eval(chess::Board& board, chess::Movelist& moves, int depth);

	int recursive_eval(
		int ply_remaining,
		int ply_from_root,
		chess::Board& board,
		float alpha,
		float beta,
		chess::Move& best_move);

	int get_opening_move(size_t hash);

	void sort_move_list(chess::Movelist& moves, chess::Board& board);
public:
	leonardo_value_bot_3();
	leonardo_value_bot_3(int ms_per_move, bool given_use_nnet);

	chess::Move get_move(chess::Board& board) override;
};
