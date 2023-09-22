#include "leonardo_value_bot_4.hpp"

bool leonardo_value_bot_4::time_up()
{
	auto current_time = std::chrono::steady_clock::now();
	auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
	return time_elapsed >= ms_per_move;
}

float leonardo_value_bot_4::eval_4(const ChessBoard& board, const UniqueMoveList& moves)
{
	float simpel_eval = get_simpel_eval(board, moves, false);
	leonardo_util::set_matrix_from_chessboard(board, input_m);
	value_net.forward_propagation(input_m);
	float value_net_eval = leonardo_util::get_value_nnet_output(value_net.get_output());
	value_net_eval *= board.getCurrentTurnColor() == White ? 1 : -1;

	float simpel_eval_weight = 100;
	float value_net_eval_weight = 10;

	return
		(simpel_eval * simpel_eval_weight) +
		(value_net_eval * value_net_eval_weight);
}

float leonardo_value_bot_4::recursive_eval(
	int start_depth,
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
		return eval_4(board, board.getAllLegalMoves());
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
			start_depth,
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

int leonardo_value_bot_4::get_best_move(const ChessBoard& board, const UniqueMoveList& legal_moves, int depth, bool is_maximizing)
{
	float best_eval = -FLT_MAX;
	float score_mult = is_maximizing ? 1.0f : -1.0f;
	int best_move_index = 0;
	int idx = 0;
#ifdef PRINT_SEARCH_INFO
	//std::cout << "depth " << depth << ":\n";
#endif
	for (const std::unique_ptr<Move>& m : legal_moves)
	{
		ChessBoard new_board = board;
		new_board.makeMove(*m);

		float eval = recursive_eval(
			depth - 1,
			depth - 1,
			-FLT_MAX,
			FLT_MAX,
			new_board,
			!is_maximizing);

		eval *= score_mult;

		if (eval > best_eval)
		{
			best_eval = eval;
			best_move_index = idx;
		}

		idx++;

		//minus because this is the score of the enemy

#ifdef PRINT_SEARCH_INFO
		float sf_eval = -stockfish_interface::eval(new_board.getFen(), depth);
		
		std::cout
			<< "move: " << m->getString()
			<< " eval: " << eval
			<< " sf eval: " << sf_eval
			<< "\n";
			
#endif // PRINT_SEARCH_INFO

	}

	return best_move_index;
}

leonardo_value_bot_4::leonardo_value_bot_4(int ms_per_move)
	: leonardo_value_bot(ms_per_move, "actual value")
{
	input_m = matrix(leonardo_util::get_input_format());
}

int leonardo_value_bot_4::getMove(const ChessBoard& board, const UniqueMoveList& legal_moves)
{
	search_cancelled = false;
	start_time = std::chrono::steady_clock::now();

	if (position_is_known_opening(board))
	{
		return get_random_opening_move(board, legal_moves);
	}

	bool is_maximizing = board.getCurrentTurnColor() == White;

	int depth = 1;
	int best_move_idx = 0;
	int done_depth = 0;
	while (!time_up())
	{
		int tmp_best = get_best_move(board, legal_moves, depth, is_maximizing);

		if (search_cancelled)
		{
			break;
		}
		best_move_idx = tmp_best;
		done_depth = depth;
		depth++;
	}


#ifdef PRINT_SEARCH_INFO
	ChessBoard cpy = board;
	cpy.makeMove(*legal_moves[best_move_idx]);
	//float sf_eval_before = stockfish_interface::eval(board.getFen(), done_depth);
	//float sf_eval = -stockfish_interface::eval(cpy.getFen(), done_depth);
	//auto sf_moves = stockfish_interface::get_best_moves(board.getFen(), done_depth);
	/*
	float sf_move_value = sf_eval - sf_eval_before;

	int sf_idx = -1;
	for (int i = 0; i < sf_moves.size(); i++)
	{
		if (sf_moves[i].move_str_uci == legal_moves[best_move_idx]->getString())
		{
			sf_idx = i;
			break;
		}
	}
	if (sf_idx == -1)
	{
		std::cout << "stockfish didn't know this move. move string was: " << legal_moves[best_move_idx]->getString() << "\n";
		for (int i = 0; i < sf_moves.size(); i++)
		{
			std::cout << sf_moves[i].move_str_uci << "\n";
		}
	}

	float accuracy = 100.0f - (((float)sf_idx + 1) / (std::max((float)sf_moves.size(), 1.0f)) * 100.0f);
	std::cout << "Depth: " << done_depth << " "
		<< "sf static eval: " << sf_eval << " "
		<< "stockfish move eval : " << sf_move_value << " "
		<< "stockfish's #" << (sf_idx + 1) << " choice. out of " << sf_moves.size()
		<< " (" << accuracy << "% accurate)"
		<< "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() << "ms "
		<< "Best move: " << legal_moves[best_move_idx]->getString() << std::endl;
	*/
	std::cout << "Depth: " << done_depth << " "
		<< "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() << "ms "
		<< "Best move: " << legal_moves[best_move_idx]->getString() << std::endl;

#endif // PRINT_SEARCH_INFO


	return best_move_idx;
}