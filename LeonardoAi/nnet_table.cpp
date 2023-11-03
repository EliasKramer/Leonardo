#include "nnet_table.hpp"

chess::U64 nnet_table::make_key(chess::Bitboard white_bb, chess::Bitboard black_bb) const
{
	return (black_bb ^ random_hash_b) ^ (white_bb ^ random_hash_w);
}

chess::U64 nnet_table::black_bb_from_key(chess::Bitboard white_bb, chess::U64 key) const
{
	return (key ^ (white_bb ^ random_hash_w)) ^ random_hash_b;
}

nnet_table::nnet_table(size_t table_size_mb)
	:random_hash_b(0), random_hash_w(0)
{
	//WTF if one calculates it like this, lots of values will be overridden for some reason
	table.resize(30000000);// table_size_mb * 1024 * 1024 / sizeof(nnet_table_entry));

	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<chess::U64> dis(0, 0xffffffffffffffff);

	random_hash_b = dis(gen);
	random_hash_w = dis(gen);
}

void nnet_table::insert(
	chess::Bitboard white_bb, 
	chess::Bitboard black_bb, 
	int value)
{
	chess::U64 key = make_key(white_bb, black_bb);
	int idx = key % table.size();

	if (table[idx].key == 0)
	{
		table_item_count++;
	}
	inserted_items_count++;

	table[idx].value = value;
	table[idx].key = key;
	table[idx].w = white_bb;
}

int nnet_table::get(chess::Bitboard white_bb, chess::Bitboard black_bb) const
{
	chess::U64 key = make_key(white_bb, black_bb);
	int idx = key % table.size();
	if (table[idx].key == key &&
		table[idx].w == white_bb &&
		black_bb_from_key(white_bb, key) == black_bb)
	{
		return table[idx].value;
	}
	else
	{
		return not_found;
	}
}

float nnet_table::percent_overridden() const
{
	if (table_item_count == 0 || inserted_items_count == 0)
		return 0.0f;

	return 1 - ((float)table_item_count / (float)inserted_items_count);
}