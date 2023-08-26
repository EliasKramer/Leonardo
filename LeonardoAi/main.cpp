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
	neural_network test("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\firestorm_epoch_2100\\value.parameters");
	std::cout << test.parameter_analysis();
	return 0;
	*/

	/*
	neural_network other = neural_network("C:\\Users\\Elias\\Desktop\\i5_overlord\\actual_overlord_epoch_0\\policy.parameters");
	neural_network local = neural_network("C:\\Users\\Elias\\Desktop\\i5_overlord\\actual_overlord_epoch_5\\policy.parameters");


	std::cout << "other: " << other.parameter_analysis() << std::endl;
	std::cout << "local: " << local.parameter_analysis() << std::endl;
	std::cout << "same format: " << local.nn_equal_format(other) << std::endl;
	std::cout << "same format: " << local.equal_parameter(other) << std::endl;
	*/
	std::cout << "this version has a test function for the ds test, that only works on matrices with a very specific format" << std::endl;
	leonardo_overlord overlord("better_value");
	//overlord.test_value_nnet();
	overlord.train_on_gm_games();
	return 0;
	ChessBoard b(STARTING_FEN);
	b.getFen();
	/*
	data_space mock;
	mock.to_string(); //remove - used to not get optimized away
	
	//board.getFen();
	ChessBoard board("r2q1rk1/pppb1ppp/3b1n2/3P4/3QP3/3P4/PP1B1PPP/RN2KB1R b KQ - 1 12");
	neural_network nnet("C:\\Users\\krame\\Desktop\\all\\_coding\\Leonardo\\LeonardoAi\\models\\xavier_momentum_2300\\value.parameters");
	
	matrix in(leonardo_util::get_input_format());
	leonardo_util::set_matrix_from_chessboard(board, in);
	//std::cout << in.get_string() << std::endl;
	nnet.forward_propagation(in);
	
	std::cout << "state: " << nnet.get_output().get_string() << std::endl;
	std::cout << "value: " << leonardo_util::get_value_nnet_output(nnet.get_output())<< std::endl;

	//std::cout << nnet.parameter_analysis();
	
	return 0;
	matrix m(vector3(1, 1, 1));
	m.get_string();*/
}