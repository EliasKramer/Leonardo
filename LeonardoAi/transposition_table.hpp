#pragma once
#include "chess.hpp"
#include "chess_constants.hpp"
//Credit to https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm

namespace tt {
	typedef enum _entry_type : uint8_t
	{
		empty = 0,
		exact = 1,
		upper_bound = 2,
		lower_bound = 3
	} entry_type;

	constexpr int16_t unknown_eval = -28931;
}

class transposition_table
{
private:
	class entry
	{
	public:
		chess::U64 key = 0;
		uint8_t depth = 0;
		tt::entry_type flags = tt::entry_type::empty;
		int16_t value = tt::unknown_eval;
		int16_t best_move = 0;
	};

	uint32_t hash_table_size_mb;
	std::vector<entry> hash_table;

	bool is_mate_score(int16_t score) const;
	int32_t store_corrected_score(int16_t score, int32_t ply);
	int32_t retrieve_corrected_score(int16_t score, int32_t ply) const;
public:
	transposition_table(uint32_t hash_table_size_mb);

	int16_t get_stored_score(chess::U64 hash, int32_t ply, int32_t depth, int32_t alpha, int32_t beta) const;
	chess::Move get_move(chess::U64 hash) const;
	void store(chess::U64 hash, int32_t ply, int32_t depth, int16_t value, tt::entry_type flags, const chess::Move& best_move);
};
