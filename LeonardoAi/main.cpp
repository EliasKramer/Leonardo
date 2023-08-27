#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
#include <windows.h>
#include <signal.h>
#include "../MockChessEngine/Game.h"

#include "stockfish_interface.hpp"
#include <mutex>
// Define mutex for synchronized output
#include <iostream>

int main(int argc, char* argv[])
{
	stockfish_interface::init();
	//float evaluation = stockfish_interface::eval(STARTING_FEN.c_str(), 12);
	//sync_cout << "evaluation: " << evaluation << sync_endl;
	
	std::cout << "this version has a test function for the ds test, that only works on matrices with a very specific format" << std::endl;
	leonardo_overlord overlord("better_value");
	overlord.train_on_gm_games();
	

	return 0;

	ChessBoard b(STARTING_FEN);
	b.getFen();
}