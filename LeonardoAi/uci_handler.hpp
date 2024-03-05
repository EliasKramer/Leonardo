#pragma once
#include "leonardo.hpp"
#include "./NeuroFox/util.hpp"
#include "./leonardo_util.hpp"

class uci_handler
{
private:
	leonardo bot;
	chess::Board board;

	neural_network duration_nnet;
	matrix duration_nnet_input;

	bool logging_enabled;

	std::string log_file_name = "leonardo_log.txt";

	void log(const char* msg);
	void log(std::string msg);
	chess::Move str_to_move(std::string move_str);

	void receive_command(std::string& msg);

public:
	uci_handler(bool logging_enabled);
	void uci_loop();
};