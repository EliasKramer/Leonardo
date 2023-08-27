#pragma once
#include <iostream>
#include "../StockFish/misc.h"

namespace stockfish_interface
{
	void init();
	float eval(const std::string& fen, int depth);
}