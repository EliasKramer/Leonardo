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
#include "leonardo_value_bot_3.hpp"
#include "leonardo_value_bot_4.hpp"
#include "../MockChessEngine/HumanPlayer.h"

static void write_to_file(std::string filename, std::string content)
{
	std::ofstream file(filename);
	file << content;
	file.close();
}

static void play_against_thread(
	std::mutex& mutex,
	int& score,
	leonardo_value_bot& player,
	leonardo_value_bot& adversary)
{
	chess_arena arena(
		"the pit",
		std::make_unique<leonardo_value_bot>(player),
		std::make_unique<leonardo_value_bot>(adversary)
	);
	arena_result res = arena.play(1);
	std::lock_guard<std::mutex> lock(mutex);
	if (res.player_2_won == 1)
	{
		score++;
	}
	else if (res.player_1_won == 1)
	{
		score--;
	}
}

static void brute_force_good_params()
{

	//float evaluation = stockfish_interface::eval(STARTING_FEN.c_str(), 12);
	//sync_cout << "evaluation: " << evaluation << sync_endl;

	//neural_network nn("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\pre_calced_dataset_epoch_6200\\value.parameters");
	//std::cout << nn.parameter_analysis();
	//return 0;

	neural_network nnet;// ("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\one_layer_epoch_2690200\\value.parameters");
	//neural_network nnet("C:\\Users\\Elias\\Desktop\\4small_epoch\\4small_epoch_2652200\\value.parameters");
	//neural_network nnet("C:\\Users\\Elias\\Desktop\\runin\\models\\reasonable_epoch_18200\\value.parameters");
	//std::cout << nnet.parameter_analysis();


	//write_to_file("stats.txt", "start");

	std::vector<std::unique_ptr<leonardo_value_bot>> best_players;
	//push back a good player

	best_players.push_back(
		std::make_unique<leonardo_value_bot>(
			1000
		)
	);

	leonardo_value_bot adversary = leonardo_value_bot(
		1000
	);

	while (true)
	{
		adversary.mutate(-0.3f, 1.5f);
		adversary.mutate(-0.3f, 1.5f);
		adversary.mutate(-0.3f, 1.5f);
		std::cout << "mutating\n";
		std::cout << "++\n";
		std::cout << adversary.param_string();
		std::cout << "++\n";


		std::cout << "playing against best players (count: " + std::to_string(best_players.size()) + ")\n";
		int score = 0;
		std::mutex mutex;
		std::vector<std::thread> threads;
		for (int i = 0; i < best_players.size(); i++)
		{
			threads.push_back(std::thread(play_against_thread, std::ref(mutex), std::ref(score), std::ref(adversary), std::ref(*(best_players[i]))));
		}
		for (int i = 0; i < threads.size(); i++)
		{
			if (threads[i].joinable())
			{
				threads[i].join();
			}
		}
		std::cout << "score: " << score << "\n";
		float minimum_requirement = (float)best_players.size() * 0.75f;
		std::cout << "minimum requirement: " << minimum_requirement << "\n";

		if ((float)score > minimum_requirement)
		{
			std::cout << "+++++++\n";
			std::cout << "better\n";
			std::cout << "+++++++\n";
			//new one is better - set best to new
			best_players.push_back(
				std::make_unique<leonardo_value_bot>(adversary)
			);
			std::string param_string = adversary.param_string();
			std::cout << param_string << "\n";
			write_to_file("stats.txt", param_string);
		}
		else
		{
			std::cout << "equal or worse\n";
			//new is equal or worse. reset
			adversary.get_params_from_other(*(best_players[best_players.size() - 1]));
		}
		std::cout << "---------------\n";
	}
}

static void play_game()
{
	neural_network nnet("value.parameters");

	ChessGame game(
		std::make_unique<leonardo_value_bot_4>(
			4000
		),
		std::make_unique<leonardo_value_bot_4>(
			4000
		),
		STARTING_FEN);

	game.start();
}

static void eval_pos(std::string fen)
{
	neural_network nnet;
	leonardo_value_bot bot(1000);

	bot.print_eval(fen);
}

static int move_count(ChessBoard& board, int depth, int& nodes, int& end_states)
{
	if (depth == 0)
	{
		nodes++;
		end_states++;
		return 1;
	}
	UniqueMoveList moves = board.getAllLegalMoves();
	int count = 0;
	for (int i = 0; i < moves.size(); i++)
	{
		nodes++;
		ChessBoard cpy = board;
		cpy.makeMove(*moves[i]);
		count += move_count(cpy, depth - 1, nodes, end_states);
	}
	return count;
}

int main(int argc, char* argv[])
{
	stockfish_interface::init();
	//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	std::cout << "this version has a test function for the ds test, that only works on matrices with a very specific format" << std::endl;
	std::cout << "hab in die pawn pos werte rumgepuscht\n";

	leonardo_overlord overlord("ov");
	overlord.create_dataset_policy();
	return 0;
	overlord.test_eval_on_single_match(
		//"e2e4 c7c6 d2d4 d7d5 b1c3 d5e4 c3e4 b8d7 e4g5 d7f6 d4d5 d8a5 c2c3 c6d5 g1e2 e7e5 f2f4 c8g4 g5h3 e5e4 h3f2 g4h5 g2g4 f6g4 f2e4 d5e4 e2g1 g4h2 g1e2 h2f1 e1f1 h5f3 h1h3 f3g4 h3g3 a8d8 d1d8 a5d8 g3g4 d8d1 f1f2 g8f6 g4g3 f8c5 e2d4 d1c2 f2g1 c2d1 g1g2 e8g8 f4f5 f6d5 g3g7 g8h8 g7g5 h7h6 g5g3 c5d6 g3g7 h8g7 c1h6 g7h6 a1d1 d5e3 g2g1 e3d1 d4b5 d6c5 b5d4 d1b2 g1h2 b2d1 d4e2 c5d6 h2g1 h6g5 c3c4 d1e3 e2d4 d6e5 d4b3 e3c4 a2a3 c4a3 g1g2 a3c4 b3c5 c4d6 c5d7 f8e8 d7e5 e8e5 f5f6 e5e8 g2g1 e4e3 g1g2 e3e2 g2g3 e2e1q g3h3 e1e2 h3g3 a7a6 g3h3 a6a5 h3g3 a5a4 g3h3 a4a3 h3g3 a3a2 g3h3 a2a1q h3g3 b7b6 g3h3 b6b5 h3g3 b5b4 g3h3 b4b3 h3g3 b3b2 g3h3 b2b1q h3g3 d6f5 g3h3 f5g7 h3g3 g7h5 g3h3 h5g3 h3g3 e8f8 g3h3 f8g8 h3g3 g8g7 f6g7 a1a3"
		//"e2e4 g8h6 e4e5 h8g8 f1a6 g7g5 b1c3 h6g4 a6b7 d7d5 e5e6 g8h8 g1h3 f8h6 d2d3 a7a6 d1g4 h6f8 c3b1 c7c5 b1c3 f7e6 g4g3 d8d7 h1g1 f8g7 g3d6 a8a7 f2f3 h8f8 h3f2 h7h5 d6b6 f8h8 b7c6 g7e5 f2e4 e5d6 b6b5 d5d4 c3a4 d6h2 e1d2 g5g4 b5a6 e8f7 e4c3 h2c7 a4c5 g4f3 c6a4 b8a6 g2f3 c7d6 g1g7 f7g7 c5e6 d7e6 a4b5 h8g8 c3e4 d6h2 e4g3 g8h8 b5c6 g7g6 a2a3 h2g1 b2b3 a7a8 a1b1 g6h6 c6e8 g1f2 g3h1 f2g3 f3f4 a6b8 h1g3 h5h4 e8a4 e6g6 d2e1 c8h3 e1d2 g6g7 g3e2 h3g2 b1a1 h8f8 c1b2 g7g8 a1c1 g8b3 e2c3 b3b7 a4b3 b7d5 b3c4 b8a6 c3a2 d5e5 c2c3 a6b8 f4e5 a8a3 d2c2 g2e4 b2a1 h6g6 c4f7 g6h6 f7b3 f8f4 c1b1 h6g6 a1b2 f4f2 c2d1 e4f3 d1c1 e7e6 b3c4 f2d2 c4e6 d2g2 e6c8 b8a6 c3d4 a6c7 b1a1 a3a4 c8f5 g6g7 f5d7 f3c6 b2c3 g2e2 e5e6 e2e1 c1c2 a4a6 a1d1 e1e4 c2b1 c6d7 c3e1 a6a3 b1b2 a3c3 d1d2 e4e3 d2c2 e3e5 b2c1 d7a4 c1b2 c3c2 b2a3 c2c1 e1f2 c1d1 a3b4 a4b3 f2e1 d1c1 e1h4 c7e8 d4e5 c1c2 b4b3 c2c7 h4f2 c7c1 f2h4 c1d1 a2b4 g7f8 b3a2 e8d6 h4e7 f8e8 b4a6 e8e7 a2b2 e7d8 b2c3 d1f1 c3c2 d6b5 c2b2 f1e1 e6e7 d8e8 a6c5 e1e3 b2c2 e3d3 c5a4 d3d4 c2b2 d4a4 b2c2 b5a7 e5e6 a4b4 c2c1 b4b1 c1c2 b1d1 c2b2 e8e7 b2c2 e7f6 c2c3 d1e1 c3c4 f6f5 e6e7 e1e2 e7e8n e2h2 e8c7 a7c8 c4c5 c8a7 c7a6 a7c6 c5d5 c6d8 d5c5 f5g5 c5c4 d8f7 c4c5 f7d8 a6b4 g5h6 b4a6 h2f2 a6b4 f2c2 c5d6 c2c1 b4c2 c1c2 d6d7 c2a2 d7d6 a2a5 d6c7 h6h5 c7d7 h5h6 d7e8 h6h7 e8f8 h7h8 f8e7 a5c5 e7e8 h8h7 e8e7 c5c6 e7d8 h7g6 d8e8 g6g5 e8d7 c6c2 d7e6 c2c4 e6d5 c4c3 d5e6 c3c6 e6f7 c6d6 f7f8 d6f6 f8g8 g5h5 g8h7 f6f1 h7g7 h5h4 g7h6 f1f3 h6g6 f3f7 g6f7"
		//"d2d4 d7d5 c2c4 c7c6 g1f3 g8f6 b1c3 a7a6 c4c5 h8g8 c1f4 b8d7 e2e4 d5e4 f1c4 e4f3 d1b3 e7e6 b3c2 f3g2 h1g1 e6e5 c4f7 e8f7 c2b3 f7e8 b3e6 f8e7 f4e3 a6a5 g1g2 g8f8 g2g7 a8b8 g7e7 d8e7 e6d6 e7d6 c5d6 e5d4 e3d4 c6c5 d4e3 f6g4 e1c1 g4e3 f2e3 f8f6 c1b1 f6f7 e3e4 d7f6 d1f1 c8g4 e4e5 f6d7 f1g1 g4f5 b1c1 f7f8 g1f1 f5e6 f1f8 e8f8 c1b1 d7e5 c3e4 e6f5 b1c2 f5e4 c2c3 b8d8 d6d7 d8d7 c3b3 d7d3 b3c2 d3h3 c2d2 h3h2 d2c1 e4c2 a2a3 e5d3"
		"e2e4 c7c5 g1f3 d7d6 f1b5 c8d7 b5d7 d8d7 e1g1 b8c6 c2c3 g8f6 d2d4 f6e4 b1d2 d6d5 d2e4 d5e4 f3e5 c6e5 d4e5 a8d8 c1g5 d7d1 a1d1 h8g8 g5e7 e8e7 d1d8 e7d8 f1e1 d8c7 e1e4 f8e7 e4f4 g8f8 e5e6 f7e6 f4f8 e7f8 h2h4 h7h5 c3c4 c7b8 g2g3 a7a5 g1g2 f8e7 g2h2 a5a4 h2h1 a4a3 b2a3 e6e5 a3a4 e5e4 h1g2 b7b6 g2h3 e4e3 f2e3 e7f6 g3g4 g7g6 h3g3 f6e5 g3f3 h5g4 f3g4 e5f6 a2a3 b8c7 h4h5 g6h5 g4h5 c7d6 h5g6 d6e5 e3e4 f6h4 g6h5 h4e1 h5g6 e1a5 g6f7 e5e4 f7g7 e4d3 g7g6 d3c4 g6g7 c4b3 g7g8 b3a3 g8f7 c5c4 f7g7 c4c3 g7h6 c3c2 h6h5 c2c1q h5h4 c1f4 h4h5 a3a4 h5g6 a5d2 g6g7 a4b4 g7g6 d2c1 g6g7 f4h6 g7f7 b4b3 f7e7 b6b5 e7d7 b3a4 d7c7 a4a3 c7d7 b5b4 d7d8 b4b3 d8e8 b3b2 e8e7 b2b1q e7e8 b1b7 e8d8 c1d2 d8e8 d2e3 e8d8 e3f4 d8e8 f4g3 e8d8 g3h4 d8e8 h4g3 e8d8 g3h4 d8e8 h4f2 e8d8 f2g1 d8e8 g1h2 e8d8 h2g1 d8e8 g1h2 e8d8 h2f4 d8e8 f4e3 e8d8 e3g5 d8e8 g5d2 e8d8 d2g5 d8e8 g5c1 e8d8 c1b2 d8e8 b2c3 e8d8 c3d4 d8e8 d4e5 e8d8 e5f6 d8e8 h6h8"
	);
	//eval_pos("r1bqkb1r/pp3ppp/1n2p3/3pP3/7P/3B4/PP1NNPP1/R1BQK2R b KQkq - 0 10");

	return 0;
	matrix m;
	m.get_string();

	ChessBoard b(STARTING_FEN);
	b.getFen();
}