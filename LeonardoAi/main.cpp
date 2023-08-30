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
#include "main.h"

int main(int argc, char* argv[])
{
	stockfish_interface::init();
	//float evaluation = stockfish_interface::eval(STARTING_FEN.c_str(), 12);
	//sync_cout << "evaluation: " << evaluation << sync_endl;
	
	//neural_network nn("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\lil_stockfish_small_fast_epoch_16800\\value.parameters");
	//std::cout << nn.parameter_analysis();
	//return 0;
	std::cout << "this version has a test function for the ds test, that only works on matrices with a very specific format" << std::endl;
	leonardo_overlord overlord("ds_training");
	//overlord.train_on_gm_games();
	overlord.train_on_dataset();

	return 0;
	matrix m;
	m.get_string();

	ChessBoard b(STARTING_FEN);
	b.getFen();
}