#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
int main()
{
	//leonardo_overlord overlord("helo");
	//overlord.train();
	chess_arena arena = chess_arena(
		"Leonardo Arena",
		std::make_unique<RandomPlayer>(),
		std::make_unique<AlphaBetaPruningBot>()
	);

	arena_result result = arena.play(100);

	std::cout << "Player 1 wins: " << result.player_1_won << std::endl;
	std::cout << "Player 2 wins: " << result.player_2_won << std::endl;
	std::cout << "Draws: " << result.draws << std::endl;


}