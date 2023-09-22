#pragma once
#include <iostream>
#include "../StockFish/misc.h"

namespace stockfish_interface
{
	void init();
	float eval(const std::string& fen, int depth);

	class sf_move
	{
	public:
		std::string move_str_uci = "";
		float value = 0.0f;

		sf_move(std::string move_str_uci, float value) : move_str_uci(move_str_uci), value(value) {}
	};

	std::vector<sf_move> get_best_moves(const std::string& fen, int depth);
}