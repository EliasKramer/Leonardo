#include "AlphaBetaPruningBot.h"

AlphaBetaPruningBot::AlphaBetaPruningBot()
: Medicrius()
{}

AlphaBetaPruningBot::AlphaBetaPruningBot(int depth)
: Medicrius(depth)
{}

int AlphaBetaPruningBot::getMove(const ChessBoard& board, const UniqueMoveList& moves)
{
	auto begin = std::chrono::high_resolution_clock::now();

	bool isWhiteToMove = board.getCurrentTurnColor() == White;
	int colorMult = isWhiteToMove ? 1 : -1;

	int endPointsEvaluated = 0;
	int nodesSearched = 0;
	int branchesPruned = 0;

	int depth = _depth;

	int bestMoveScore = INT_MIN;
	int bestMoveIdx = 0;

	int moveIdx = 0;
	for (const std::unique_ptr<Move>& curr : moves)
	{
		ChessBoard boardCopy = board;
		boardCopy.makeMove(*curr);

		int endstatesBefore = endPointsEvaluated;

		int moveScore =
			getMoveScoreRecursively(
				boardCopy,
				depth - 1,
				!isWhiteToMove,
				BLACK_WIN_EVAL_VALUE,
				WHITE_WIN_EVAL_VALUE,
				nodesSearched,
				endPointsEvaluated,
				branchesPruned
			);

		//float small_rand = random_float_excl(1.00, 1.15);
		int currScore = colorMult * moveScore;

		/*
		std::cout
			<< "Move: " << curr.get()->getString()
			<< ", Score: " << currScore
			<< ", Endstates Evaluated: " << endPointsEvaluated - endstatesBefore
			<< std::endl;
		*/

		if (currScore > bestMoveScore)
		{
			bestMoveScore = currScore;
			bestMoveIdx = moveIdx;
		}

		moveIdx++;
	}

	auto end = std::chrono::high_resolution_clock::now();

	long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

	std::string additionalInfo = "Pruned Branches: " + std::to_string(branchesPruned);
	
	/*
	printSearchStatistics(
		"Minimax with Alpha Beta Pruning",
		nodesSearched,
		endPointsEvaluated,
		depth,
		*moves[bestMoveIdx].get(),
		bestMoveScore,
		duration,
		additionalInfo);
	*/

	return bestMoveIdx;
}

int AlphaBetaPruningBot::getMoveScoreRecursively(
	ChessBoard board,
	int depth,
	bool isMaximizingPlayer,
	int alpha,
	int beta,
	int& nodesSearched,
	int& endStatesSearched,
	int& prunedBranches)
{
	if (depth == 0)
	{
		endStatesSearched++;
		nodesSearched++;
		return evaluateBoard(board);
	}
	else
	{
		UniqueMoveList moves = board.getAllLegalMoves();

		//no more moves
		if (moves.size() == 0)
		{
			nodesSearched++;
			endStatesSearched++;
			//dont know if this works
			return board.isKingInCheck() ?
				(board.getCurrentTurnColor() == White ?
					//when the depth is very high, the checkmate can be done earlier
					//(when you search with depth 4, the function gets called with 3, 2, 1, 0 recursively)
					//therefore a higher depth in the argument means actually low depth.
					////finding a checkmate at a low depth is better, because it can be delivered earlier
					GAME_STATE_EVALUATION[BlackWon] - depth :
					GAME_STATE_EVALUATION[WhiteWon] + depth)
				: 0;
		}
		int bestEval = isMaximizingPlayer ? INT_MIN : INT_MAX;
		for (std::unique_ptr<Move>& curr : moves)
		{
			ChessBoard copyBoard = board;
			copyBoard.makeMove(*curr);

			nodesSearched++;
			int evaluation =
				getMoveScoreRecursively(
					copyBoard,
					depth - 1,
					!isMaximizingPlayer,
					alpha,
					beta,
					nodesSearched,
					endStatesSearched,
					prunedBranches
				);

			if (isMaximizingPlayer)
			{
				bestEval = std::max(bestEval, evaluation);
				alpha = std::max(alpha, evaluation);
				if (beta <= alpha)
				{
					prunedBranches++;
					break;
				}
			}
			else
			{
				bestEval = std::min(bestEval, evaluation);
				beta = std::min(beta, evaluation);
				if (beta <= alpha)
				{
					prunedBranches++;
					break;
				}
			}
		}
		return bestEval;
	}
}