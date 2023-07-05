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

	leonardo_overlord overlord("test");
	overlord.train_prediction();
	
	matrix m(vector3(1, 1, 1));
	m.get_string();
}