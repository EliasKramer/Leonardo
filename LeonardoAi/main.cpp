#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
#include <windows.h>
#include <signal.h>
#include "../MockChessEngine/Game.h"
/*
extern "C" void my_function_to_handle_aborts(int signal_number)
{
	int x = 0;
	/*Your code goes here. You can output debugging info.
	  If you return from this function, and it was called
	  because abort() was called, your program will exit or crash anyway
	  (with a dialog box on Windows).
}
*/	 

/*Do this early in your program's initialization */
int main()
{
	//signal(SIGABRT, &my_function_to_handle_aborts);
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	//leonardo_overlord overlord("test");
	//overlord.train_prediction();
	
	ChessBoard board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	neural_network nnet("C:\\Users\\krame\\Desktop\\all\\_coding\\Leonardo\\LeonardoAi\\models\\only_prediction_15100\\prediction.parameters");
	
	matrix in(leonardo_util::get_input_format());
	leonardo_util::set_matrix_from_chessboard(board, in);
	std::cout << in.get_string() << std::endl;
	nnet.forward_propagation(in);
	
	std::cout << "state: " << nnet.get_output().get_string() << std::endl;
	std::cout << "value: " << leonardo_util::get_prediction_output(nnet.get_output())<< std::endl;

	std::cout << nnet.parameter_analysis();
	
	matrix m(vector3(1, 1, 1));
	m.get_string();
}