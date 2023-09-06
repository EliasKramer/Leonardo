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

#include "leonardo_value_bot.hpp"

int main(int argc, char* argv[])
{
	stockfish_interface::init();
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	std::cout << "this version has a test function for the ds test, that only works on matrices with a very specific format" << std::endl;

	//float evaluation = stockfish_interface::eval(STARTING_FEN.c_str(), 12);
	//sync_cout << "evaluation: " << evaluation << sync_endl;

	//neural_network nn("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\pre_calced_dataset_epoch_6200\\value.parameters");
	//std::cout << nn.parameter_analysis();
	//return 0;
	neural_network nnet("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\one_layer_epoch_2690200\\value.parameters");

	std::cout << "hab in die pawn pos werte rumgepuscht\n";

	ChessGame game(
		std::make_unique<leonardo_value_bot>(nnet, 3, 0, true, 0.00f, 1.0f),
		std::make_unique<leonardo_value_bot>(nnet, 4, 0, true, 0.00f, 1.0f),
		STARTING_FEN);

	game.start();


	//leonardo_overlord overlord("4small");
	//overlord.train_on_dataset();
	/*
	overlord.test_eval_on_single_match(
		"d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 d1c2 d7d5 c4d5 e6d5 g1f3 c7c5 d4c5 f6e4 c1d2 e4d2 c2d2 d8f6 e2e3 b8d7 f1b5 e8g8 b5d7 c8d7 a2a3 b4c3 d2c3 f6c3 b2c3 f8c8 a1b1 a8b8 e1d2 c8c5 f3d4 b7b6 b1b3 g8f8 h1c1 f8e7 b3b4 b8c8 d4b3 c5c4 f2f3 h7h5 h2h4 e7d6 b3d4 f7f6 b4c4 d5c4 e3e4 c8c5 d2e3 c5a5 c1a1 g7g6 d4e2 d6e7 e3d4 d7e6 e2f4 e6f7 g2g3 a5b5 a3a4 b5b2 a4a5 b6b5 a5a6 b5b4 c3b4 b2b4 a1a5 b4b3 f4d5 f7d5 e4d5 b3f3 a5c5 f3g3 c5c7 e7d6 c7a7 g3g4 d4c3 g4g1 a7a8 g1a1 a6a7 d6c7 d5d6 c7b7 d6d7 b7a8 d7d8q a8a7 d8e7 a7a8 e7f6 a1g1 f6c6 "
	);
	*/
	return 0;
	matrix m;
	m.get_string();

	ChessBoard b(STARTING_FEN);
	b.getFen();
}