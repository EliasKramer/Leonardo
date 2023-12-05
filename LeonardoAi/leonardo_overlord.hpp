#pragma once
#include "NeuroFox/neural_network.hpp"
#include <memory>
#include <thread>
#include "leonardo_util.hpp"
#include <unordered_map>
#include <unordered_set>
#include "leonardo_policy_bot.hpp"
#include <memory>
#include "./NeuroFox/data_space.hpp"
#include <filesystem>
#include "stockfish_interface.hpp"
#include "chess.hpp"

class leonardo_overlord
{
private:
	std::string name;

	neural_network best_policy_nnet;
	neural_network best_value_nnet;
	
	neural_network new_policy_nnet;
	neural_network new_value_nnet;

	const bool gpu_mode = false;

	void save_best_to_file(size_t epoch, bool value_nnet, bool policy_nnet);

public:
	leonardo_overlord(std::string name);

	void train_value_nnet();
	void train_duration_nnet();

	void test_value_nnet_single(std::string& game);
	void test_value_nnet();

	//pawngame
	void reinforcement_learning_pawns();
};