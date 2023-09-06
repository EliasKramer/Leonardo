#include "leonardo_value_bot.hpp"

leonardo_value_bot::leonardo_value_bot(neural_network given_value_nnet)
	: leonardo_value_bot(given_value_nnet, 4, 0, false, 0.5f, 0.5f, 0.5f)
{}

leonardo_value_bot::leonardo_value_bot(
	neural_network given_value_nnet,
	int given_depth,
	int max_capture_depth,
	bool gpu_mode,
	float nnet_influence,
	float hard_coded_influence,
	float dropout
)
	: Player("leonardo value bot (nnet: " + std::to_string(nnet_influence) + " hard coded: " + std::to_string(hard_coded_influence) + ")"),
	value_net(given_value_nnet),
	input_board(leonardo_util::get_input_format()),
	depth(given_depth),
	max_capture_depth(max_capture_depth),
	gpu_mode(gpu_mode),
	nnet_influence(nnet_influence),
	hard_coded_influence(hard_coded_influence),
	dropout(dropout)
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

	return nnet_eval * color_mult;
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
	float gameStatePoints = GAME_STATE_EVALUATION[state];
	//if the game is still ongoing the value will be -1 and thus should be continued evaluating
	if (gameStatePoints != -1)
	{
		if (state == Draw || state == Stalemate)
		{
			if (board.getCurrentTurnColor() == White)
			{
				return -1; //disencourages stalemates
			}
			else
			{
				return 1;
			}
		}
		else
		{
			return gameStatePoints;
		}
	}

	//now calculate the score
	float score = 0;

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
				float materialValue = PIECETYPE_VALUE[typeIdx];

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

	return score / 100;
}

float leonardo_value_bot::get_eval(const ChessBoard& board)
{
	float hard_coded_eval = hard_coded_influence != 0 ? get_simpel_eval(board) : 0;
	float nnet_eval = nnet_influence != 0 ? get_nnet_eval(board) : 0;

	//std::cout << "Hard coded eval: " << hard_coded_eval << " Nnet eval: " << nnet_eval << std::endl;

	return
		hard_coded_eval * hard_coded_influence +
		nnet_eval * nnet_influence;
}

float leonardo_value_bot::get_capture_move_score_recursively(
	const ChessBoard& board,
	int curr_depth,
	bool is_maximizing_player,
	float alpha,
	float beta,
	int& nodes_searched,
	int& end_states_searched,
	int& max_depth_reached)
{
	UniqueMoveList possible_capture_moves = board.getAllLegalCaptureMoves();

	if (possible_capture_moves.size() == 0 || curr_depth >= max_capture_depth)
	{
		nodes_searched++;
		end_states_searched++;
		max_depth_reached = std::max(max_depth_reached, curr_depth);
		float eval = get_eval(board);
		return eval;
	}

	float best_eval = is_maximizing_player ? -FLT_MAX : FLT_MAX;

	for (std::unique_ptr<Move>& curr : possible_capture_moves)
	{
		ChessBoard copyBoard = board.getCopyByValue();
		copyBoard.makeMove(*curr);

		nodes_searched++;
		float evaluation = get_capture_move_score_recursively(
			copyBoard,
			curr_depth + 1,
			!is_maximizing_player,
			alpha,
			beta,
			nodes_searched,
			end_states_searched,
			max_depth_reached);

		if (is_maximizing_player)
		{
			best_eval = std::max(best_eval, evaluation);
			alpha = std::max(alpha, evaluation);
			if (beta <= alpha)
			{
				break;
			}
		}
		else
		{
			best_eval = std::min(best_eval, evaluation);
			beta = std::min(beta, evaluation);
			if (beta <= alpha)
			{
				break;
			}
		}
	}

	return best_eval;
}

float leonardo_value_bot::get_move_score_recursively(
	const ChessBoard& board,
	int depth,
	bool is_maximizing_player,
	float alpha,
	float beta,
	int& nodes_searched,
	int& end_states_searched,
	int& max_capture_depth_reached)
{
	if (depth <= 0)
	{
		end_states_searched++;
		nodes_searched++;
		
		return get_capture_move_score_recursively(
			board,
			0,
			is_maximizing_player,
			alpha,
			beta,
			nodes_searched,
			end_states_searched,
			max_capture_depth_reached);
	}
	else
	{
		std::vector<std::unique_ptr<Move>> moves = board.getAllLegalMoves();

		//no more moves
		if (moves.size() == 0)
		{
			nodes_searched++;
			end_states_searched++;
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
		float bestEval = is_maximizing_player ? INT_MIN : INT_MAX;
		for (std::unique_ptr<Move>& curr : moves)
		{
			ChessBoard copyBoard = board.getCopyByValue();
			copyBoard.makeMove(*curr);

			nodes_searched++;
			float evaluation = get_move_score_recursively(
					copyBoard,
					depth - 1,
					!is_maximizing_player,
					alpha,
					beta,
					nodes_searched,
					end_states_searched,
					max_capture_depth_reached);

			if (is_maximizing_player)
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
	bool is_white_to_move = board.getCurrentTurnColor() == White;
	int colorMult = is_white_to_move ? -1 : 1;

	int endPointsEvaluated = 0;
	int nodesSearched = 0;
	int max_capture_depth_reached = 0;

	float move_score =
		get_move_score_recursively(
			board,
			depth-1,
			is_white_to_move,
			BLACK_WIN_EVAL_VALUE,
			WHITE_WIN_EVAL_VALUE,
			nodesSearched,
			endPointsEvaluated,
			max_capture_depth_reached);

	scores[thread_id] = move_score * colorMult;

	std::cout
		<< "Move: " + move_str
		+ ", Score: " + std::to_string(move_score * colorMult)
		+ ", Max Capture Depth reached: " + std::to_string(max_capture_depth_reached)
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

	float bestMoveScore = -FLT_MAX;
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
	std::cout << "current position according to stockfish: " << stockfish_interface::eval(board.getFen(), 8) << "\n";
	std::cout << "current position according to leonardo after searching: " << bestMoveScore << "\n";
	std::cout << "leonardo static eval: " << get_eval(board) << "\n";
	return bestMoveIdx;
}