#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
#include <cassert>
int main()
{
	leonardo_overlord overlord("debuglesseses");
	overlord.train();
}