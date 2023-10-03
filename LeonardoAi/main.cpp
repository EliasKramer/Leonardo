#include <iostream>
#include "stockfish_interface.hpp"
#include "chess_game.hpp"
#include "random_player.hpp"
#include "minimax_player.hpp"
#include "abp_player.hpp"
#include "human_player.hpp"
#include "chess.hpp"
#include "leonardo_value_bot.hpp"
#include "leonardo_value_bot_1.hpp"
#include "leonardo_overlord.hpp"

uint64_t perft(chess::Board& board, int depth) {
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	if (depth == 1) {
		return moves.size();
	}

	uint64_t nodes = 0;

	for (int i = 0; i < moves.size(); i++) {
		const auto move = moves[i];
		board.makeMove(move);
		nodes += perft(board, depth - 1);
		board.unmakeMove(move);
	}

	return nodes;
}

int main()
{
	stockfish_interface::init();

	leonardo_value_bot_1 player1(4); //4
	abp_player player2(5); //5

	chess_game game(
		&player1,
		&player2
	);

	game.play();


	std::cout << "Hello World!\n";
}