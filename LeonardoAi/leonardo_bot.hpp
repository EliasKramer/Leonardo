#pragma once

#include "../MockChessEngine/Player.h"
#include "NeuroFox/neural_network.hpp"
#include "leonardo_util.hpp"

enum _playstyle_t {
	max,
	distributed_random //stochastic
} typedef e_playstyle_t;

class leonardo_bot : public Player
{
private:
	neural_network& nn;
	matrix input_board;

	e_playstyle_t playstyle;
public:
	leonardo_bot(
		neural_network& given_nn, 
		e_playstyle_t given_playstyle);

	int getMove(const ChessBoard& board, const UniqueMoveList& legal_moves) override;
};

