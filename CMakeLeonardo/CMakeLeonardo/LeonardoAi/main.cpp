#include "leonardo_overlord.hpp"
#include "../MockChessEngine/AlphaBetaPruningBot.h"
#include "../MockChessEngine/RandomPlayer.h"
//#include <windows.h>
#include <signal.h>
#include "../MockChessEngine/Game.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <sched.h>
#include <unistd.h>
int main()
{
	//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
pid_t pid = getpid();

    // Create a scheduling parameter structure
    struct sched_param params;

    // Set the scheduling policy to SCHED_FIFO (First-In-First-Out)
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);

    // Set the process to the high priority
    if (sched_setscheduler(pid, SCHED_FIFO, &params) == -1) {
        std::cerr << "Failed to set high priority. Ensure you have sufficient privileges." << std::endl;
        return 1;
    }

	leonardo_overlord overlord("azure");
	overlord.train();
	/*
	return 0;

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
	m.get_string();
	*/
}