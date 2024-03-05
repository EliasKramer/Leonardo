#include "uci_handler.hpp"
#include <string>
#include <iostream>

std::vector<std::string> split(std::string str, std::string token = " ")
{
	std::vector<std::string>result;
	while (str.size())
	{
		int index = str.find(token);
		if (index != std::string::npos)
		{
			result.push_back(str.substr(0, index));
			str = str.substr(index + token.size());
			if (str.size() == 0)result.push_back(str);
		}
		else
		{
			result.push_back(str);
			str = "";
		}
	}
	return result;
}

uci_handler::uci_handler(bool logging_enabled)
	:bot(3000, 1),//1.5),
	board(DEFAULT_FEN),
	duration_nnet("duration.parameters"),
	duration_nnet_input(leonardo_util::get_input_format_duration_nnet()),
	logging_enabled(logging_enabled)
{}

void uci_handler::log(const char* msg)
{
	std::string str(msg);
	log(str);
}

void uci_handler::log(std::string msg)
{
	if (!logging_enabled)
	{
		return;
	}
	std::ofstream log_file;
	log_file.open(log_file_name, std::ios_base::app);
	log_file << get_current_time_str() << ": ";
	log_file << msg << "\n";
	log_file.close();
}

chess::Move uci_handler::str_to_move(std::string move_str)
{
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	for (int i = 0; i < moves.size(); i++)
	{
		if (chess::uci::moveToUci(moves[i]) == move_str)
		{
			return moves[i];
		}
	}

	log("Error: move not found: " + move_str);
	log("fen: " + board.getFen());
	log("moves:");
	for (int i = 0; i < moves.size(); i++)
	{
		log(chess::uci::moveToUci(moves[i]));
	}
	return chess::Move::NULL_MOVE;
}

/*uci
uciok
ucinewgame
isready
readyok
position startpos
go wtime 60000 btime 60000
bestmove e2e4
position startpos moves e2e4 g8f6
go wtime 59000 btime 58000
bestmove d2d4*/

static bool is_digit(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}
static int get_ms_to_make_move(int time, int inc)
{
	int result = 1;

	if (time == -1)
	{
		return -1;
	}


	if (time < 1000)
	{
		result = time / 2;
	}
	else if (time < 2000)
	{
		result = time / 5;
	}
	else if (time < 5000)
	{
		result = time / 8;
	}
	else if (time < 60000)
	{
		result = time / 10;
	}
	else if (time < 300000)
	{
		result = time / 40;
	}
	else if (time < 600000)
	{
		result = time / 60;
	}
	else if (time < 1800000)
	{
		result = time / 80;
	}
	else
	{
		result = time / 100;
	}


	if (inc != -1)
	{
		result += inc;
	}

	return result;
}

void uci_handler::receive_command(std::string& message)
{
	//Console.WriteLine(message);
	//std::cout << "Command received: " + message;
	//message = message.Trim();

	std::vector<std::string> tokens = split(message);
	if (tokens.size() == 0)
	{
		log("Error: empty command");
		return;
	}

	std::string messageType = tokens[0];

	if (messageType == "uci")
	{
		std::cout << "uciok\n";
	}
	else if (messageType == "isready")
	{
		std::cout << "readyok\n";
	}
	else if (messageType == "ucinewgame")
	{
		board = chess::Board(DEFAULT_FEN);
		log("setup default position");
	}
	else if (messageType == "position")
	{
		//position startpos moves e2e4 e7e5 g1f3
		if (tokens.size() <= 1)
		{
			log("Error: position command has no arguments");
			return;
		}
		if (tokens[1] == "startpos")
		{
			board = chess::Board(DEFAULT_FEN);
			if (tokens.size() > 2)
			{
				for (int i = 3; i < tokens.size(); i++)
				{
					chess::Move move = str_to_move(tokens[i]);
					board.makeMove(move);
				}
			}
			log("set pos: " + board.getFen());
		}
		/*
		else if (tokens[1] == "fen")
		{
			std::string fen = "";
			for (int i = 2; i < 8; i++)
			{
				fen += tokens[i] + " ";
			}
			fen = fen.substr(0, fen.size() - 1);
			board = chess::Board(fen);
			if (tokens.size() > 8)
			{
				for (int i = 9; i < tokens.size(); i++)
				{
					chess::Move move = chess::uci::uciToMove(tokens[i], board);
					board.makeMove(move);
				}
			}
		}
		*/
		else
		{
			log("unrecognized position command: " + message);
		}
	}
	else if (messageType == "go")
	{
		//go wtime 60000 btime 60000
		int move_time = -1;
		int time = -1;
		int increment = -1;
		for (int i = 1; i < tokens.size(); i++)
		{
			if (i + 1 < tokens.size() && is_digit(tokens[i + 1]))
			{
				if (tokens[i] == "wtime" && board.sideToMove() == chess::Color::WHITE)
				{
					time = std::stoi(tokens[i + 1]);
				}
				else if (tokens[i] == "btime" && board.sideToMove() == chess::Color::BLACK)
				{
					time = std::stoi(tokens[i + 1]);
				}
				else if (tokens[i] == "winc" && board.sideToMove() == chess::Color::WHITE)
				{
					increment = std::stoi(tokens[i + 1]);
				}
				else if (tokens[i] == "binc" && board.sideToMove() == chess::Color::BLACK)
				{
					increment = std::stoi(tokens[i + 1]);
				}
				else if (tokens[i] == "movetime")
				{
					move_time = std::stoi(tokens[i + 1]);
				}
			}
		}


		std::string log_output = "";
		int ms_given = leonardo_util::get_ms_to_think(
			duration_nnet,
			duration_nnet_input,
			board,
			move_time,
			time,
			increment
		);

		ms_given = std::min(ms_given, 1000);

		log("time log: time:" + std::to_string(time) + " move_time:" + std::to_string(move_time) + " increment:" + std::to_string(increment) + " chosen_time:" + std::to_string(ms_given));
		
		chess::Move gotten_move = bot.get_move(board, ms_given, log_output);
		log("engine log: " + log_output);

		std::string selected_move = chess::uci::moveToUci(gotten_move);
		std::cout << "bestmove " << selected_move << "\n";
		log("made move. fen: " + board.getFen() + " move: " + selected_move);
	}
	else if (messageType == "stop")
	{
		log("stop command received");
	}
	else if (messageType == "quit")
	{
		log("quitting");
		log("------------------------");
	}
	else
	{
		log("unrecognized command: " + message);
	}
}
void uci_handler::uci_loop()
{
	log("Started Uci Loop");
	std::string command;
	do
	{
		std::getline(std::cin, command);
		log("Command received: " + command);
		receive_command(command);
	} while (command != "quit");

	log("Ended Uci Loop");
}