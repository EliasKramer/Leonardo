#pragma once
#include "Medicrius.h"
#include "../LeonardoAi/NeuroFox/util.hpp"

class AlphaBetaPruningBot : public Medicrius
{
public:
	AlphaBetaPruningBot();
	AlphaBetaPruningBot(int depth);

	int getMove(const ChessBoard& board, const UniqueMoveList& moves) override;
private:
	int getMoveScoreRecursively(
		ChessBoard board,
		int depth,
		bool isMaximizingPlayer,
		int alpha,
		int beta,
		int& nodesSearched,
		int& endStatesSearched,
		int& prunedBranches
	);
};