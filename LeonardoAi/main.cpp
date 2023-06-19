#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
int main()
{
	leonardo_overlord overlord("helo");
	overlord.train();
}