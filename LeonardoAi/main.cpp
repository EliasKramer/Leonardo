/*
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

#include "leonardo_value_bot.hpp"
#include "leonardo_value_bot_1.hpp"
#include "leonardo_value_bot_2.hpp"
#include "leonardo_value_bot_3.hpp"
#include "../MockChessEngine/HumanPlayer.h"

int main(int argc, char* argv[])
{
	stockfish_interface::init();
	//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	std::cout << "this version has a test function for the ds test, that only works on matrices with a very specific format" << std::endl;
	std::cout << "hab in die pawn pos werte rumgepuscht\n";

	//stockfish_interface::get_best_moves(KIWIPETE_FEN, 4);

	leonardo_overlord overlord("policy_gm_training");
	overlord.train_policy_on_gm_games();

	//brute_force_good_params();
	//play_game();
	//eval_pos("r1bqkb1r/pp3ppp/1n2p3/3pP3/7P/3B4/PP1NNPP1/R1BQK2R b KQkq - 0 10");

	return 0;
}*/

#include <iostream>
#include "stockfish_interface.hpp"
#include "chess_game.hpp"
#include "random_player.hpp"

int main()
{
	stockfish_interface::init();

	stockfish_interface::eval("r1bqkb1r/pp3ppp/1n2p3/3pP3/7P/3B4/PP1NNPP1/R1BQK2R b KQkq - 0 10", 4);

	random_player* player1 = new random_player;
	random_player* player2 = new random_player;

	chess_game game(
		player1,
		player2
	);

	game.play();


	std::cout << "Hello World!\n";
}