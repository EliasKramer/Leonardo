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
	neural_network best_network;
	neural_network new_network;

	std::thread file_save_thread;
	void save_best_to_file(size_t epoch);
public:
	leonardo_overlord(std::string name);
	~leonardo_overlord();

	const neural_network& get_best_network() const;
};