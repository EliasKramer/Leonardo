#include "transposition_table.hpp"

bool transposition_table::is_mate_score(int16_t score) const
{
	return std::abs(score) >= chess_constants::MATE_SCORE - chess_constants::MAX_DEPTH;
}

int32_t transposition_table::store_corrected_score(int16_t score, int32_t ply)
{
	if (is_mate_score(score)) // is mate 
	{
		//ply from root to checkmate node
		int all_ply = std::abs(chess_constants::MATE_SCORE - std::abs(score));

		//ply from current node to checkmate node
		int corrected_ply = all_ply - ply;

		//we need to save the difference
		int sign = score > 0 ? 1 : -1;
		score = sign * (chess_constants::MATE_SCORE - corrected_ply);
	}

	return score;
}

int32_t transposition_table::retrieve_corrected_score(int16_t score, int32_t ply) const
{
	if (is_mate_score(score)) // is mate 
	{
		//ply difference from curr node to checkmate node
		int ply_before_mate_found = std::abs(chess_constants::MATE_SCORE - std::abs(score));

		//we saved the difference. now we need to add it to the current ply
		int actual_ply = ply_before_mate_found + ply;

		//we add the difference to the score
		int sign = score > 0 ? 1 : -1;
		score = sign * (chess_constants::MATE_SCORE - actual_ply);
	}

	return score;
}

transposition_table::transposition_table(uint32_t hash_table_size_mb)
    :hash_table_size_mb(hash_table_size_mb)
{
    float mb_per_entry = sizeof(entry) / (1024.0f  * 1024.0f);
    hash_table.resize(hash_table_size_mb / mb_per_entry);
}

int16_t transposition_table::get_stored_score(chess::U64 hash, int32_t ply, int32_t depth, int32_t alpha, int32_t beta) const
{
	const entry& item = hash_table[hash % hash_table.size()];

	if (item.key == hash) //entry is acutally the same
	{
		if (item.depth >= depth)
		{
			int corrected_value = retrieve_corrected_score(item.value, ply);

			if (item.flags == tt::entry_type::exact)
			{
				return corrected_value;
			}

			// We have stored the upper bound of the eval for this position. If it's less than alpha then we don't need to
			// search the moves in this position as they won't interest us; otherwise we will have to search to find the exact value
			// the current position is less worth, than the best available move - skip
			if ((item.flags == tt::entry_type::upper_bound) && (corrected_value <= alpha))
			{
				return alpha;
			}

			// We have stored the lower bound of the eval for this position. Only return if it causes a beta cut-off.
			// beta cut-off means, that the opponent will not allow us to reach this position, because they have a better move
			if ((item.flags == tt::entry_type::lower_bound) && (corrected_value >= beta))
			{
				return beta;
			}
		}
	}

	return tt::unknown_eval;
}

chess::Move transposition_table::get_move(chess::U64 hash) const
{
    const entry& item = hash_table[hash % hash_table.size()];

    if (item.key == hash)
    {
        return chess::Move(item.best_move);
    }

    return chess::Move::NULL_MOVE;
}

void transposition_table::store(chess::U64 hash, int32_t ply, int32_t depth, int16_t value, tt::entry_type flags, const chess::Move& best_move)
{
    entry& item = hash_table[hash % hash_table.size()];

    item.key = hash;
    item.best_move = best_move.move();
    item.value = store_corrected_score(value, ply);
    item.flags = flags;
    item.depth = depth;
}