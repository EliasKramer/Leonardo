#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
int main()
{
	leonardo_overlord overlord("run_p100_g100_mpg200");
	overlord.train();
}