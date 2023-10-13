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
#include "leonardo_value_bot_3.hpp"
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
void test_pawn_encoding()
{
	int promotion_count = 0;
	int en_passant_count = 0;
	while (true)
	{
		chess::Board board(DEFAULT_FEN);
		matrix board_m(leonardo_util::get_pawn_input_format());
		matrix slow_board_m(leonardo_util::get_pawn_input_format());
		leonardo_util::encode_pawn_matrix(board, board_m);
		std::cout << "\n\n";
		std::vector<chess::Move> move_history;

		while (board.isGameOver().second == chess::GameResult::NONE)
		{
			chess::Movelist moves;
			chess::movegen::legalmoves(moves, board);

			int random_idx = rand() % moves.size();
			chess::Move chosen_move = chess::Move::NULL_MOVE;
			chosen_move = moves[random_idx];

			leonardo_util::make_move(board, board_m, chosen_move);

			leonardo_util::encode_pawn_matrix(board, slow_board_m);
			move_history.push_back(chosen_move);
			
			if(chosen_move.typeOf() == chess::Move::PROMOTION)
				promotion_count++;
			if (chosen_move.typeOf() == chess::Move::ENPASSANT)
				en_passant_count++;


			if (slow_board_m != board_m)
			{
				std::cout << "error " << chess::uci::moveToUci(chosen_move) << "\n";
				std::cout << "slow_m: \n" << slow_board_m.get_string() << "\n";
				std::cout << "fast_m: \n" << board_m.get_string() << "\n";
				std::cout << "diff_m: \n" << matrix::get_difference_string(slow_board_m, board_m) << "\n";

				return;
			}

			//std::cout << chess::uci::moveToUci(chosen_move) << "\n";
		}
		for (int i = move_history.size() - 1; i >= 0; i--)
		{
			chess::Move move = move_history[i];
			leonardo_util::unmake_move(board, board_m, move);
			leonardo_util::encode_pawn_matrix(board, slow_board_m);

			if (slow_board_m != board_m)
			{
				std::cout << "error " << chess::uci::moveToUci(move) << "\n";
				std::cout << "slow_m: \n" << slow_board_m.get_string() << "\n";
				std::cout << "fast_m: \n" << board_m.get_string() << "\n";
				std::cout << "diff_m: \n" << matrix::get_difference_string(slow_board_m, board_m) << "\n";

				return;
			}
		}
		chess::Board starting_board(DEFAULT_FEN);
		matrix m_start(leonardo_util::get_pawn_input_format());
		leonardo_util::encode_pawn_matrix(starting_board, m_start);
		std::cout << "promotion count: " << promotion_count << "\n";
		std::cout << "en passant count: " << en_passant_count << "\n";
		std::cout << "---------------------\n";

		if (m_start != board_m || m_start != slow_board_m)
		{
			std::cout << "start " << "\n";
			std::cout << "slow_m: \n" << slow_board_m.get_string() << "\n";
			std::cout << "fast_m: \n" << board_m.get_string() << "\n";
			std::cout << "diff_m: \n" << matrix::get_difference_string(slow_board_m, board_m) << "\n";
			return;
		}
	}
}
void train()
{
	leonardo_overlord ov("nanopawn");
	ov.train_value_nnet();
}
int main()
{
	test_pawn_encoding();
	//train();
	return 0;
	stockfish_interface::init();

	leonardo_value_bot_3 player1(5);
	abp_player player2(5);

	chess_game game(
		&player1,
		&player2
	);

	game.play();


	std::cout << "Hello World!\n";
}