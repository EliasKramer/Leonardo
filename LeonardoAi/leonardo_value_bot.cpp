#include "leonardo_value_bot.hpp"

leonardo_value_bot::leonardo_value_bot(neural_network given_value_nnet)
	: leonardo_value_bot(given_value_nnet, 4, false, 0.5f, 0.5f)
{}

leonardo_value_bot::leonardo_value_bot(
	neural_network given_value_nnet,
	int given_depth,
	bool gpu_mode,
	float nnet_influence,
	float hard_coded_influence
)
	: Player("leonardo value bot " + std::to_string(nnet_influence)),
	value_net(given_value_nnet),
	input_board(leonardo_util::get_input_format()),
	depth(given_depth),
	gpu_mode(gpu_mode),
	nnet_influence(0.5f),
	hard_coded_influence(0.5f)
{
	if (gpu_mode)
	{
		value_net.enable_gpu_mode();
		input_board.enable_gpu_mode();
	}
}

float leonardo_value_bot::get_nnet_eval(const ChessBoard& board)
{
	// - if white wins return highest possible number
	// - if black wins return lowest possible number
	leonardo_util::set_matrix_from_chessboard(board, input_board);
	input_board.sync_device_and_host();
	value_net.forward_propagation(input_board);
	value_net.get_output().sync_device_and_host();
	float nnet_eval = leonardo_util::get_value_nnet_output(value_net.get_output());

	float color_mult = board.getCurrentTurnColor() == White ? 1.0f : -1.0f;

	return nnet_eval * 100.0f * color_mult;
}
float leonardo_value_bot::get_simpel_eval(const ChessBoard& board)
{
	//regulates how much the material (the pieces on the board) is worth

	BoardRepresentation boardRep = board.getBoardRepresentation();

	//get the obvious out the way 
	// - if white wins return highest possible number
	// - if black wins return lowest possible number
	// - stalemate and draw is 0
	GameState state = board.getGameState();
	int gameStatePoints = GAME_STATE_EVALUATION[state];
	//if the game is still ongoing the value will be -1 and thus should be continued evaluating
	if (gameStatePoints != -1)
	{
		if (state == Draw || state == Stalemate)
		{
			if (board.getCurrentTurnColor() == White)
			{
				return -1000; //disencourages stalemates
			}
			else
			{
				return 1000;
			}
		}
		else
		{
			return gameStatePoints;
		}
	}

	//now calculate the score
	int score = 0;

	for (int i = A1; i <= H8; i++)
	{
		BitBoard idxBB = BB_SQUARE[i];

		//iterate over every type except king
		for (int typeIdx = 0; typeIdx <= Queen; typeIdx++)
		{
			//if current square overlaps with the current piece type
			if (bitboardsOverlap(idxBB, boardRep.PiecesOfType[typeIdx]))
			{
				//get the material value of the piece type
				int materialValue = PIECETYPE_VALUE[typeIdx];

				bool currPieceIsBlack = bitboardsOverlap(idxBB, boardRep.PiecesOfColor[Black]);
				ChessColor currPieceColor = currPieceIsBlack ? Black : White;

				materialValue += POSITION_VALUE[currPieceColor][typeIdx][i];

				//if pieces are black, negate the value
				if (currPieceIsBlack)
				{
					materialValue *= -1;
				}
				//add the value to the total score multiplied by material weight
				score += materialValue;
			}
		}
	}

	//Evaluating the king position
	//usually the king is worth 20k and the position value is added to that.
	//this is not done here, since these values cancel out. - could be done for clarity
	score +=
		(POSITION_VALUE_KING[White][board.getGameDurationState()][boardRep.KingPos[White]] -
			POSITION_VALUE_KING[Black][board.getGameDurationState()][boardRep.KingPos[Black]]);

	//TODO
	//to encourage pawns for more structure, it would be useful to look if they get protected
	//by another pawn. and increase the value if it is that way

	return score;
}

float leonardo_value_bot::get_move_score_recursively(
	const ChessBoard& board,
	int depth,
	bool isMaximizingPlayer,
	float alpha,
	float beta,
	int& nodesSearched,
	int& endStatesSearched,
	int& maxCaptureDepthReached)
{
	if (depth == 0)
	{
		endStatesSearched++;
		nodesSearched++;
		/*
		int currCaptureMovesDepth = 0;
		int allCaptureMovesValue =
			-getAllCaputureMoveScoreRecursively(
				board,
				-alpha,
				-beta,
				nodesSearched,
				endStatesSearched,
				currCaptureMovesDepth);

		//std::cout << "Max depth: " << currCaptureMovesDepth << std::endl;
		maxCaptureDepthReached = std::max(maxCaptureDepthReached, currCaptureMovesDepth);
		return allCaptureMovesValue;*/

		float hard_coded_eval = hard_coded_influence != 0 ? get_simpel_eval(board) : 0;
		float nnet_eval = nnet_influence != 0 ? get_nnet_eval(board) : 0;

		//std::cout << "Hard coded eval: " << hard_coded_eval << " Nnet eval: " << nnet_eval << std::endl;

		return
			hard_coded_eval * hard_coded_influence +
			nnet_eval * nnet_influence;
	}
	else
	{
		std::vector<std::unique_ptr<Move>> moves = board.getAllLegalMoves();

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
		float bestEval = isMaximizingPlayer ? INT_MIN : INT_MAX;
		for (std::unique_ptr<Move>& curr : moves)
		{
			ChessBoard copyBoard = board.getCopyByValue();
			copyBoard.makeMove(*curr);

			nodesSearched++;
			float evaluation =
				get_move_score_recursively(
					copyBoard,
					depth - 1,
					!isMaximizingPlayer,
					-alpha,
					-beta,
					nodesSearched,
					endStatesSearched,
					maxCaptureDepthReached);

			if (isMaximizingPlayer)
			{
				bestEval = std::max(bestEval, evaluation);
				alpha = std::max(alpha, evaluation);
				if (beta <= alpha)
				{
					break;
				}
			}
			else
			{
				bestEval = std::min(bestEval, evaluation);
				beta = std::min(beta, evaluation);
				if (beta <= alpha)
				{
					break;
				}
			}
		}
		return bestEval;
	}
}

void leonardo_value_bot::thread_task(
	int thread_id,
	const std::string& move_str,
	std::vector<float>& scores,
	ChessBoard board)
{
	bool isWhiteToMove = board.getCurrentTurnColor() == White;
	int colorMult = isWhiteToMove ? 1 : -1;

	int endPointsEvaluated = 0;
	int nodesSearched = 0;
	int maxCaptureDepthReached = 0;

	float move_score =
		get_move_score_recursively(
			board,
			depth,
			!isWhiteToMove,
			BLACK_WIN_EVAL_VALUE,
			WHITE_WIN_EVAL_VALUE,
			nodesSearched,
			endPointsEvaluated,
			maxCaptureDepthReached);

	scores[thread_id] = move_score;

	std::cout
		<< "Move: " + move_str
		+ ", Score: " + std::to_string(move_score)
		+ ", Max Capture Depth: " + std::to_string(maxCaptureDepthReached)
		+ ", Endstates Evaluated: " + std::to_string(endPointsEvaluated)
		+ "\n";
}

int leonardo_value_bot::getMove(const ChessBoard& board, const UniqueMoveList& legal_moves)
{
	auto begin = std::chrono::high_resolution_clock::now();

	int moveIdx = 0;
	std::vector<float> move_scores(legal_moves.size());
	std::vector<std::thread> threads;

	for (const std::unique_ptr<Move>& curr : legal_moves)
	{
		ChessBoard boardCopy = board;
		boardCopy.makeMove(*curr);


		threads.push_back(std::thread(
			&leonardo_value_bot::thread_task,
			this,
			moveIdx,
			curr.get()->getString(),
			std::ref(move_scores),
			boardCopy
		));

		moveIdx++;
	}

	for (std::thread& curr : threads)
	{
		if (curr.joinable())
		{
			curr.join();
		}
	}

	float bestMoveScore = INT_MIN;
	int bestMoveIdx = 0;
	for (int i = 0; i < move_scores.size(); i++)
	{
		if (move_scores[i] > bestMoveScore)
		{
			bestMoveScore = move_scores[i];
			bestMoveIdx = i;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();

	long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

	std::cout << "done. took " << ms_to_str(duration) << "\n";
	std::cout << "curren position according to stockfish: " << stockfish_interface::eval(board.getFen(), 8) << "\n";

	return bestMoveIdx;
}