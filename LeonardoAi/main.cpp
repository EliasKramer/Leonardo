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

void some_encoding()
{
	neural_network nnet;
	nnet.set_input_format(leonardo_util::get_input_format_one_hot());
	//nnet.add_fully_connected_layer(200, leaky_relu_fn);
	nnet.add_fully_connected_layer(leonardo_util::get_input_format_one_hot(), identity_fn);
	nnet.xavier_initialization();

	chess::Board board = chess::Board(DEFAULT_FEN);
	matrix input(leonardo_util::get_input_format_one_hot());

	std::vector<matrix> inputs;

	while (true)
	{
		chess::Movelist moves;
		chess::movegen::legalmoves(moves, board);

		if (moves.size() == 0)
		{
			std::string stuff;
			std::cout << "no moves\n";
			std::cin >> stuff;
		}

		for (int i = 0; i < moves.size(); i++)
		{
			chess::Move move = moves[i];
			board.makeMove(move);

			leonardo_util::set_matrix_from_chessboard_one_hot(board, input);

			inputs.push_back(input);

			board.unmakeMove(move);
		}

		board.makeMove(moves[rand() % moves.size()]);

		if (board.isGameOver().second != chess::GameResult::NONE)
		{
			board = chess::Board(DEFAULT_FEN);

			data_space ds(
				leonardo_util::get_input_format_one_hot(),
				leonardo_util::get_input_format_one_hot(),
				inputs,
				inputs);

			std::cout << "testing\n";
			std::cout << nnet.test_on_ds(ds).to_string() << "\n";
			std::cout << "learning\n";
			nnet.learn_on_ds(ds, 1, 1, 0.0001f, false);
			std::cout << "paying games\n";
			inputs.clear();
			continue;
		}
	}
}
int main()
{
	leonardo_overlord ov("mini_only_pawn_eq");
	ov.train_value_nnet();

	return 0;
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