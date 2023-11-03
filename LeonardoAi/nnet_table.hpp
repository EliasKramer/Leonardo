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
		int value = 0;
	};

	chess::U64 random_hash_w;
	chess::U64 random_hash_b;
	std::vector<nnet_table_entry> table;
	unsigned int table_item_count = 0;
	unsigned int inserted_items_count = 0;

	chess::U64 make_key(chess::Bitboard white_bb, chess::Bitboard black_bb) const;

public:
	const static int not_found = -987654321;

	nnet_table(size_t table_size_mb);

	void insert(chess::Bitboard white_bb, chess::Bitboard black_bb, int value);
	int get(chess::Bitboard white_bb, chess::Bitboard black_bb) const;

	float percent_overridden() const;
	int get_table_item_count() const { return table_item_count; }
	int get_inserted_items_count() const { return inserted_items_count; }
};

