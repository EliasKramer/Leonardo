#include <iostream>
#include "stockfish_interface.hpp"
#include "chess_game.hpp"
#include "random_player.hpp"
#include "minimax_player.hpp"
#include "abp_player.hpp"
#include "human_player.hpp"
#include "chess.hpp"
#include "leonardo.hpp"
#include "leonardo_overlord.hpp"
#include "uci_handler.hpp"

int main()
{
	uint32_t hashtable_size_mb = 500;

	uci_handler uci(false);
	uci.uci_loop();
}