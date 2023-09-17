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
			nnet,
			4,
			0,
			1.231605f,
			1.000000f,
			2.698990f,
			1.000000f,
			0.736647f,
			1.000000f,
			2.038531f
		)
	);

	leonardo_value_bot adversary = leonardo_value_bot(
		nnet,
		4,
		0,
		1.231605f,
		1.000000f,
		2.698990f,
		1.000000f,
		0.736647f,
		1.000000f,
		2.038531f
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
	neural_network nnet("C:\\Users\\Elias\\Desktop\\reasonable_best\\reasonable_epoch_2690200\\value.parameters");

	ChessGame game(
		//std::make_unique<HumanPlayer>(),
		//std::make_unique<AlphaBetaPruningBot>(4),
		std::make_unique<leonardo_value_bot>(
			nnet,
			0, //max capture depth
			0.0f, //dropout>
			4.231605f,
			1.500000f,
			0.698990f,
			0.0100000f,
			0.736647f,
			.0f,
			.0f
		),
		std::make_unique<leonardo_value_bot>(
			nnet,
			0, //max capture depth
			0, //dropout>
			4.231605f,
			1.500000f,
			0.698990f,
			0.0100000f,
			0.736647f,
			.0f,
			.0f
		),
		"1rb1k1nr/1p3p1p/pNp5/3pN1B1/3PP2Q/P1P3P1/4qP1P/1R3RK1 w k - 9 23");

	game.start();
}

static void eval_pos(std::string fen)
{
	neural_network nnet;
	leonardo_value_bot bot(
		nnet,
		0, //max capture depth
		0, //dropout>
		4.231605f,
		1.500000f,
		0.698990f,
		0.0100000f,
		0.736647f,
		.0f,
		.0f);

	bot.print_eval(fen);
}

int main(int argc, char* argv[])
{
	stockfish_interface::init();
	//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	std::cout << "this version has a test function for the ds test, that only works on matrices with a very specific format" << std::endl;
	std::cout << "hab in die pawn pos werte rumgepuscht\n";

	//brute_force_good_params();
	play_game();
	//eval_pos("r1bqkb1r/pp3ppp/1n2p3/3pP3/7P/3B4/PP1NNPP1/R1BQK2R b KQkq - 0 10");

	return 0;
	matrix m;
	m.get_string();

	ChessBoard b(STARTING_FEN);
	b.getFen();
}