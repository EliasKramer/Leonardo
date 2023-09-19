#include "leonardo_value_bot_2.hpp"

bool leonardo_value_bot_2::time_up()
{
	auto current_time = std::chrono::steady_clock::now();
	auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
	return time_elapsed >= ms_per_move;
}
/*
	function minimax(node, depth, alpha, beta, maximizingPlayer) is
	if depth ==0 or node is a terminal node then
	return static evaluation of node

	if MaximizingPlayer then      // for Maximizer Player
	   maxEva= -infinity
	   for each child of node do
	   eva= minimax(child, depth-1, alpha, beta, False)
	  maxEva= max(maxEva, eva)
	  alpha= max(alpha, maxEva)
	   if beta<=alpha
	 break
	 return maxEva

	else                         // for Minimizer player
	   minEva= +infinity
	   for each child of node do
		eva= minimax(child, depth-1, alpha, beta, true)
		minEva= min(minEva, eva)
	   beta= min(beta, eva)
		if beta<=alpha
	  break
	 return minEva
*/
float leonardo_value_bot_2::recursive_eval(
	int depth,
	float alpha,
	float beta,
	const ChessBoard& board,
	bool is_maximizing)
{
	if (time_up())
	{
		search_cancelled = true;
		return 0.0f;
	}

	if (depth <= 0)
	{
		return get_simpel_eval(board);
	}

	UniqueMoveList legal_moves = board.getAllLegalMoves();
	GameState game_state = board.getGameState(legal_moves);

	if (game_state != Ongoing)
	{
		if (game_state == Draw || game_state == Stalemate)
		{
			return GAME_STATE_EVALUATION[game_state];
		}
		else
		{
			return draw_score[board.getCurrentTurnColor()];
		}
	}

	float best_score = is_maximizing ? -FLT_MAX : FLT_MAX;

	for (int i = 0; i < legal_moves.size(); i++)
	{
		ChessBoard new_board = board;
		new_board.makeMove(*legal_moves[i]);

		float score = recursive_eval(
			depth - 1,
			alpha,
			beta,
			new_board,
			!is_maximizing
		);

		if (is_maximizing)
		{
			best_score = std::max(best_score, score);
			alpha = std::max(alpha, score);
			if (beta <= alpha)
			{
				break;
			}
		}
		else
		{
			best_score = std::min(best_score, score);
			beta = std::min(beta, score);
			if (beta <= alpha)
			{
				break;
			}
		}
	}

	return best_score;
}

int leonardo_value_bot_2::get_best_move(const ChessBoard& board, const UniqueMoveList& legal_moves, int depth, bool is_maximizing)
{
	float best_eval = is_maximizing ? -FLT_MAX : FLT_MAX;
	int best_move_index = 0;
	int idx = 0;
	for (const std::unique_ptr<Move>& m : legal_moves)
	{
		ChessBoard new_board = board;
		new_board.makeMove(*m);

		float eval = recursive_eval(
			depth, 
			-FLT_MAX,
			FLT_MAX,
			new_board, 
			!is_maximizing);

		if (is_maximizing)
		{
			if (eval > best_eval)
			{
				best_eval = eval;
				best_move_index = idx;
			}
		}
		else
		{
			if (eval < best_eval)
			{
				best_eval = eval;
				best_move_index = idx;
			}
		}
		idx++;
	}

	return best_move_index;
}

leonardo_value_bot_2::leonardo_value_bot_2(int ms_per_move)
	: leonardo_value_bot(ms_per_move, "alpha beta pruning")
{}

int leonardo_value_bot_2::getMove(const ChessBoard& board, const UniqueMoveList& legal_moves)
{
	search_cancelled = false;
	start_time = std::chrono::steady_clock::now();

	if (position_is_known_opening(board))
	{
		return get_random_opening_move(board, legal_moves);
	}

	bool is_maximizing = board.getCurrentTurnColor() == White;

	int depth = 0;
	int best_move_idx = 0;
	while (!time_up())
	{
		int tmp_best = get_best_move(board, legal_moves, depth, is_maximizing);

		if (search_cancelled)
		{
			break;
		}
		best_move_idx = tmp_best;
		depth++;
	}

#ifdef PRINT_SEARCH_INFO
	std::cout << "Depth: " << depth << " "
		<< sf_eval(board, legal_moves[best_move_idx]) << " "
		<< "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() << "ms "
		<< "Best move: " << legal_moves[best_move_idx]->getString() << std::endl;


#endif // PRINT_SEARCH_INFO


	return best_move_idx;
}