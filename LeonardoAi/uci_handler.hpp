#pragma once
#include "leonardo_value_bot_3.hpp"
#include "./NeuroFox/util.hpp"

class uci_handler
{
private:
	leonardo_value_bot_3 bot;
	chess::Board board;

	std::string log_file_name = "leonardo_log.txt";

	void log(const char* msg);
	void log(std::string msg);
	chess::Move str_to_move(std::string move_str);

	void receive_command(std::string& msg);

public:
	uci_handler();
	void uci_loop();
};