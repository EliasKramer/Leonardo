#include <iostream>
#include "Game.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
int main()
{
	Game game(
		std::make_unique<RandomPlayer>(),
		std::make_unique<RandomPlayer>(),
		STARTING_FEN
	);

	game.start();
}