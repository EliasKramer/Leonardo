#pragma once

#include "../MockChessEngine/Player.h"
#include "NeuroFox/neural_network.hpp"
#include "leonardo_util.hpp"
#include "stockfish_interface.hpp"

#define PRINT_SEARCH_INFO

class leonardo_value_bot : public Player
{
private:
	neural_network value_net;

	int max_capture_depth = 4;
	float dropout = 0;
	long ms_per_move = 300; //DEBUG

	float piece_value_mult;
	float piece_pos_value_mult;
	float pawn_same_color_bonus_mult;
	float pawn_self_protection_mult;
	float passed_pawn_mult;
	float king_pos_mult;
	float king_safety_mult;

	std::unordered_map<ChessBoard, std::vector<std::string>, chess_board_hasher> opening_positions;
	bool openings_loaded;

	float get_nnet_eval(const ChessBoard& board, matrix& input_board);
	float get_simpel_eval(const ChessBoard& board);
	float get_simpel_eval(const ChessBoard& board, bool print);
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
		bool& search_finished,
		std::string& best_moves_str
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
		bool& search_finished,
		std::string& best_moves_str);

	void thread_task(
		int thread_id,
		const std::string& move_str,
		std::vector<float>& scores,
		ChessBoard board);
	
	void add_opening_position(const ChessBoard& board, const std::string& move_str);
	void load_openings();
	bool position_is_known_opening(const ChessBoard& board);
	int get_random_opening_move(const ChessBoard& board);

public:
	leonardo_value_bot(neural_network given_value_nnet);
	leonardo_value_bot(
		neural_network given_value_nnet,
		int max_capture_depth,
		float dropout,
		float piece_value_mult,
		float piece_pos_value_mult,
		float pawn_same_color_bonus_mult,
		float pawn_self_protection_mult,
		float passed_pawn_mult,
		float king_pos_mult,
		float king_safety_mult
	);

	void mutate(float min, float max);
	void reroll_params();
	void get_params_from_other(const leonardo_value_bot& other);
	std::string param_string();

	int getMove(const ChessBoard& board, const UniqueMoveList& legal_moves) override;
	
	void print_eval(std::string fen);
};

