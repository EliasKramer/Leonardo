#include <iostream>
#include "../leonardo_overlord.hpp"
#include "../chess_arena.hpp"
#include "../../MockChessEngine/RandomPlayer.h"
#include "../../MockChessEngine/AlphaBetaPruningBot.h"
#include "../leonardo_bot.hpp"
#include "../../MockChessEngine/Game.h"
int main()
{
	leonardo_overlord overlord("BetaZero");
	overlord.train();
	//overlord.learn_on_existing_games("downloaded_games\\games.txt");
	
	//neural_network best_network = overlord.get_best_network();
	
	//neural_network n500("models\\learner_epoch_500.parameters");
	//neural_network n5000("models\\learner_epoch_5000.parameters");
	
	/*
	chess_arena arena(
		"arena of the giants",
		std::make_unique<leonardo_bot>(n5000),
		std::make_unique<AlphaBetaPruningBot>()
	);
	arena.play(100, true);
	*/
	/*
	ChessGame game (
		std::make_unique<AlphaBetaPruningBot>(),
		std::make_unique<leonardo_bot>(n5000),
		STARTING_FEN
	);
	game.start();
	*/
	return 0;
}