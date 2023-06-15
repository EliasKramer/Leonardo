#pragma once
#include "NeuroFox/neural_network.hpp"
#include <memory>
#include <thread>
#include "../MockChessEngine/ChessBoard.h"
#include "leonardo_util.hpp"
class leonardo_overlord
{
private:
	std::string name;

	neural_network best_policy_nnet;
	neural_network best_prediction_nnet;
	
	neural_network new_policy_nnet;
	neural_network new_prediction_nnet;

	std::thread file_save_thread;
	void save_best_to_file(size_t epoch);

	float search(const ChessBoard& game);
	void policy(matrix& output_matrix, const ChessBoard& game);
	void get_training_data();
	void upgrade();
public:
	leonardo_overlord(std::string name);
	~leonardo_overlord();

	void train();

	const neural_network& get_best_network() const;
};