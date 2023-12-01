#pragma once

#include "chess.hpp"
#include "NeuroFox/neural_network.hpp"
#include <random>


class nnet_table
{
private:

	class nnet_table_entry
	{
	public:
		chess::U64 key = 0;
		chess::U64 w = 0;
		chess::U64 b = 0;
		bool white_to_move = false;
		int value = 0;
	};

	chess::U64 random_hash_w;
	chess::U64 random_hash_b;
	chess::U64 random_hash_turn_b;
	chess::U64 random_hash_turn_w;

	std::vector<nnet_table_entry> table;
	unsigned int table_item_count = 0;
	unsigned int inserted_items_count = 0;

	chess::U64 make_key(chess::Bitboard white_bb, chess::Bitboard black_bb, bool white_to_move) const;
	chess::U64 black_bb_from(chess::U64 hash, chess::Bitboard white_bb, bool white_to_move) const;
public:
	const static int not_found = -987654321;

	nnet_table(size_t table_size_mb);

	void insert(chess::Bitboard white_bb, chess::Bitboard black_bb, bool white_to_move, int value);
	int get(chess::Bitboard white_bb, chess::Bitboard black_bb, bool white_to_move) const;

	float percent_overridden() const;
	int get_table_item_count() const { return table_item_count; }
	int get_inserted_items_count() const { return inserted_items_count; }
};

