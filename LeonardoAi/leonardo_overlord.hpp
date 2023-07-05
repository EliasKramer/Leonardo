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
	neural_network best_prediction_nnet;
	
	neural_network new_policy_nnet;
	neural_network new_prediction_nnet;

	bool gpu_mode = true;

	std::thread file_save_thread;
	void save_best_to_file(size_t epoch, bool prediction, bool policy);

	float search(
		const ChessBoard& game,
		neural_network& given_policy_nnet,
		neural_network& given_prediction_nnet,
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& n,
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& p,
		std::unordered_map<ChessBoard, matrix, chess_board_hasher>& q,
		std::unordered_set<ChessBoard, chess_board_hasher>& visited
	);

	void policy(
		long long epoch,
		matrix& output_matrix, 
		neural_network& given_policy_nnet,
		neural_network& given_prediction_nnet,
		const ChessBoard& game);

	void self_play(
		long long epoch,
		size_t& progression,
		std::mutex& progression_mutex,
		int first_game_idx,
		int last_game_idx,
		size_t number_of_moves_per_game,
		data_space& policy_training_ds,
		data_space& prediction_training_ds
	);

	void get_training_data(
		long long epoch,
		size_t thread_count,
		size_t games_per_thread,
		size_t number_of_moves_per_game,
		data_space& policy_training_ds,
		data_space& prediction_training_ds
	);
	void upgrade(long long epoch);
public:
	leonardo_overlord(std::string name);
	~leonardo_overlord();

	void train();
	void train_policy();
	void get_data_for_prediction(
		size_t id,
		size_t& epoch,
		std::mutex& trainings_mutex
	);
	void train_prediction();

	const neural_network& get_best_network() const;
};