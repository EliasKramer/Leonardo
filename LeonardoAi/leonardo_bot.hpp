#pragma once

#include "../MockChessEngine/Player.h"
#include "NeuroFox/neural_network.hpp"
#include "leonardo_util.hpp"
class leonardo_bot : public Player
{
private:
	neural_network& nn;
	matrix input_board;
public:
	//TODO - ADD move style - max or distributed random
	leonardo_bot(neural_network& given_nn);

	int getMove(const ChessBoard& board, const UniqueMoveList& legal_moves) override;
};

