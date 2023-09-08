#pragma once
#include <memory>
#include "../MockChessEngine/Player.h"

class arena_result {
public:
	int player_1_won;
	int player_2_won;
	int draws;
};

class chess_arena
{
private:
	std::string name;

	ChessBoard board;

	bool player1_plays_white = true;

	void play_game(arena_result& result);
public:
	std::unique_ptr<Player> player1;
	std::unique_ptr<Player> player2;

	chess_arena(
		std::string given_name,
		std::unique_ptr<Player>&& player1,
		std::unique_ptr<Player>&& player2);

	arena_result play(size_t number_of_games);
};

