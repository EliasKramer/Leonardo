#pragma once

#include "../MockChessEngine/Player.h"
#include "NeuroFox/neural_network.hpp"
#include "leonardo_util.hpp"
#include "stockfish_interface.hpp"

//#define PRINT_SEARCH_INFO

class leonardo_value_bot : public Player
{
private:
	neural_network value_net;

	int max_capture_depth = 4;
	float dropout = 0;
	long ms_per_move = 150;

	//simpel eval stats
	float piece_value_mult = 3.0f;
	float piece_pos_value_mult = 1.0f;
	float pawn_same_color_bonus_mult = 0.01f;
	float pawn_self_protection_mult = 0.1f;
	float passed_pawn_mult = 1.0f;
	float king_pos_mult = 1.2f;
	float king_safety_mult = 1.3f;

	float get_nnet_eval(const ChessBoard& board, matrix& input_board);
	float get_simpel_eval(const ChessBoard& board);
	float get_eval(const ChessBoard& board, matrix& input_board);
	//float get_eval(const ChessBoard& board, matrix& input_board, float nnet_inf, float hard_infl);


	float get_capture_move_score_recursively(
		const ChessBoard& board,
		int curr_depth,
		bool is_maximizing_player,
		float alpha,
		float beta,
		int& nodesSearched,
		int& endStatesSearched,
		int& maxDepthReached,
		matrix& input_board,
		long long allowed_time_ms,
		std::chrono::steady_clock::time_point& start_time,
		bool& search_finished
	);

	float get_move_score_recursively(
		const ChessBoard& board,
		int depth,
		bool isMaximizingPlayer,
		float alpha,
		float beta,
		int& nodesSearched,
		int& endStatesSearched,
		int& maxCaptureDepthReached,
		matrix& input_board,
		long long allowed_time_ms,
		std::chrono::steady_clock::time_point& start_time,
		bool& search_finished);

	void thread_task(
		int thread_id,
		const std::string& move_str,
		std::vector<float>& scores,
		ChessBoard board);
public:
	leonardo_value_bot(neural_network given_value_nnet);
	leonardo_value_bot(
		neural_network given_value_nnet,
		int max_capture_depth,
		float dropout);

	void mutate();
	void get_params_from_other(const leonardo_value_bot& other);
	std::string param_string();

	int getMove(const ChessBoard& board, const UniqueMoveList& legal_moves) override;
};

