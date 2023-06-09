#pragma once
#include "NeuroFox/neural_network.hpp"
#include <memory>
#include <thread>
#include "../MockChessEngine/ChessBoard.h"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
#include "leonardo_util.hpp"
#include <unordered_map>
#include <unordered_set>
#include "chess_arena.hpp"
#include "leonardo_bot.hpp"
#include <memory>
#include "NeuroFox/data_space.hpp"
#include <filesystem>

class leonardo_overlord
{
private:
	std::string name;

	neural_network best_policy_nnet;
	neural_network best_value_nnet;
	
	neural_network new_policy_nnet;
	neural_network new_value_nnet;

	bool gpu_mode = true;

	std::thread file_save_thread;
	void save_best_to_file(size_t epoch, bool value_nnet, bool policy_nnet);

	float search(
		const ChessBoard& game,
		neural_network& given_policy_nnet,
		neural_network& given_value_nnet,
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& n,
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& p,
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& q,
		std::unordered_set<ChessBoard, chess_board_hasher>& visited
	);

	void policy(
		long long epoch,
		matrix& output_matrix, 
		neural_network& given_policy_nnet,
		neural_network& given_value_nnet,
		const ChessBoard& game);

	void self_play(
		long long epoch,
		size_t& progression,
		std::mutex& progression_mutex,
		int first_game_idx,
		int last_game_idx,
		size_t number_of_moves_per_game,
		data_space& policy_training_ds,
		data_space& given_value_nnet
	);

	void get_training_data(
		long long epoch,
		size_t thread_count,
		size_t games_per_thread,
		size_t number_of_moves_per_game,
		data_space& policy_training_ds,
		data_space& value_training_ds
	);
	void upgrade(long long epoch);
public:
	leonardo_overlord(std::string name);
	~leonardo_overlord();

	void train();
	void train_policy();

private:
	void train_value_nnet_thread_fn(
		size_t id,
		std::chrono::high_resolution_clock::time_point training_start,
		size_t& epoch,
		size_t& total_moves_made,
		size_t& total_games_played,
		size_t& black_won,
		size_t& white_won,
		size_t& draw,
		std::mutex& trainings_mutex);
public: 
	void train_value_nnet();
	void test_value_nnet();
	const neural_network& get_best_network() const;
};