#pragma once
#include "leonardo_value_bot.hpp"

class leonardo_value_bot_2 : public leonardo_value_bot
{
private:
	bool search_cancelled = false;
	std::chrono::steady_clock::time_point start_time;

	bool time_up();

	float recursive_eval(
		int depth,
		float alpha,
		float beta,
		const ChessBoard& board,
		bool is_maximizing
	);

	int get_best_move(
		const ChessBoard& board,
		const UniqueMoveList& legal_moves,
		int depth,
		bool is_maximizing
	);

public:
	leonardo_value_bot_2(int ms_per_move);

	//get move is without alpha beta pruning
	int getMove(const ChessBoard& board, const UniqueMoveList& legal_moves) override;
};