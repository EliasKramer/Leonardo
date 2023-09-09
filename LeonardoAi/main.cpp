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
#include "main.h"

#include "leonardo_value_bot.hpp"
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

int main(int argc, char* argv[])
{
	stockfish_interface::init();
	//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	std::cout << "this version has a test function for the ds test, that only works on matrices with a very specific format" << std::endl;

	//float evaluation = stockfish_interface::eval(STARTING_FEN.c_str(), 12);
	//sync_cout << "evaluation: " << evaluation << sync_endl;

	//neural_network nn("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\pre_calced_dataset_epoch_6200\\value.parameters");
	//std::cout << nn.parameter_analysis();
	//return 0;

	neural_network nnet("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\one_layer_epoch_2690200\\value.parameters");
	//neural_network nnet("C:\\Users\\Elias\\Desktop\\4small_epoch\\4small_epoch_2652200\\value.parameters");
	//neural_network nnet("C:\\Users\\Elias\\Desktop\\runin\\models\\reasonable_epoch_18200\\value.parameters");
	//std::cout << nnet.parameter_analysis();

	std::cout << "hab in die pawn pos werte rumgepuscht\n";

	write_to_file("stats.txt", "start");
	std::vector<std::unique_ptr<leonardo_value_bot>> best_players;
	//push back a good player

	best_players.push_back(
		std::make_unique<leonardo_value_bot>(
			nnet,
			4,
			0,
			1.0f,
			1.0f,
			1.0f,
			1.0f,
			1.0f,
			1.2f,
			1.3f
		)
	);

	leonardo_value_bot adversary = leonardo_value_bot(
		nnet,
		4,
		0,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.0f,
		1.2f,
		1.3f
	);

	while (true)
	{
		adversary.mutate();
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

		if (score > 0)
		{
			std::cout << "better\n";
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
	
	/*
	ChessGame game(
		//std::make_unique<HumanPlayer>(),
		//std::make_unique<AlphaBetaPruningBot>(4),
		std::make_unique<leonardo_value_bot>(
			nnet,
			4, //max capture depth
			0, //dropout>
			15.768575f,
			14.918061f,
			10.225163f,
			10.497873f,
			10.685966f,
			13.680205f,
			12.822161f
		),
		std::make_unique<leonardo_value_bot>(
			nnet,
			4, //max capture depth
			0, //dropout>
			4.662848f,
			4.214864f,
			1.079292f,
			3.812843f,
			3.853191f,
			1.530399f,
			3.609846f
		),
		STARTING_FEN);

	game.start();

	*/
	return 0;


	//leonardo_overlord overlord("reasonable");
	//overlord.train_on_dataset();
	/*
	overlord.test_eval_on_single_match(
	"g1f3 g8f6 c2c4 g7g6 g2g3 f8g7 f1g2 e8g8 d2d4 d7d6 b1c3 b8c6 e1g1 a7a6 h2h3 a8b8 c1g5 b7b5 c4b5 a6b5 d4d5 b5b4 g5f6 e7f6 d5c6 b4c3 b2c3 f6f5 d1d2 c8a6 f1b1 d8f6 f3d4 f6e5 e2e3 e5a5 b1b4 f8e8 a1b1 b8a8 a2a4 a5c5 d2c2 h7h5 h3h4 g7f6 c2b3 g8g7 g2d5 e8e7 d5f7 g7h6 f7c4 c5e5 c4a6 a8a6 b4b8 e5e4 b3b5 a6a7 b5c4 f6g7 b8b4 a7a8 d4e2 e4c2 e2d4 c2e4 d4b5 e4c2 b5d4 c2e4 b1d1 h6h7 c4d3 e4d5 b4b5 d5a2 "
	);
	*/

	return 0;
	matrix m;
	m.get_string();

	ChessBoard b(STARTING_FEN);
	b.getFen();
}