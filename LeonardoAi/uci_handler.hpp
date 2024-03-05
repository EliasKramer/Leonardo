#pragma once
#include "leonardo.hpp"
#include "./NeuroFox/util.hpp"
#include "./leonardo_util.hpp"

class uci_handler
{
private:
	leonardo bot;
	chess::Board board;

	bool logging_enabled;

	std::string log_file_name = "leonardo_log.txt";

	void log(const char* msg);
	void log(std::string msg);
	chess::Move str_to_move(std::string move_str);
	uint32_t get_ms_to_think(
		int time_to_move,
		int time_remaining,
		int time_increment);
	void receive_command(std::string& msg);

public:
	uci_handler(bool logging_enabled);
	void uci_loop();
};