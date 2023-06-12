#pragma once
#include "Player.h"
#include <memory>
#include <iostream>
#include <string>
class ChessGame
{
private:
	std::unique_ptr<Player> _whitePlayer;
	std::unique_ptr<Player> _blackPlayer;

	ChessBoard _board;
public:
	ChessGame(
		std::unique_ptr<Player>&& player1,
		std::unique_ptr<Player>&& player2,
		std::string fen = STARTING_FEN);

	void start();
};