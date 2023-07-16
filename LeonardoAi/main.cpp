#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
#include <windows.h>
#include <signal.h>
#include "../MockChessEngine/Game.h"

int main()
{
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	
	/*
	ChessGame game(
		std::make_unique<AlphaBetaPruningBot>(4), 
		std::make_unique<AlphaBetaPruningBot>(4),
		STARTING_FEN);
	game.start();
	return 0;
	*/
	leonardo_overlord overlord("test_run");
	overlord.train_value_nnet();
	return 0;

	data_space mock;
	mock.to_string(); //remove - used to not get optimized away
	
	//board.getFen();
	ChessBoard board("r2q1rk1/pppb1ppp/3b1n2/3P4/3QP3/3P4/PP1B1PPP/RN2KB1R b KQ - 1 12");
	neural_network nnet("C:\\Users\\krame\\Desktop\\all\\_coding\\Leonardo\\LeonardoAi\\models\\double_alpha_400\\value.parameters");
	
	matrix in(leonardo_util::get_input_format());
	leonardo_util::set_matrix_from_chessboard(board, in);
	//std::cout << in.get_string() << std::endl;
	nnet.forward_propagation(in);
	
	std::cout << "state: " << nnet.get_output().get_string() << std::endl;
	std::cout << "value: " << leonardo_util::get_value_nnet_output(nnet.get_output())<< std::endl;

	//std::cout << nnet.parameter_analysis();
	
	return 0;
	matrix m(vector3(1, 1, 1));
	m.get_string();
}