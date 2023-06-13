#pragma once
#include <memory>
#include "../MockChessEngine/Player.h"

class chess_arena
{
private:
	std::string name;

	std::unique_ptr<Player> _whitePlayer;
	std::unique_ptr<Player> _blackPlayer;

	ChessBoard board;

	int play_game();
public:
	chess_arena(
		std::string given_name,
		std::unique_ptr<Player>&& player1,
		std::unique_ptr<Player>&& player2);

	int play(size_t number_of_games);
	int play(size_t number_of_games, bool print);
};

