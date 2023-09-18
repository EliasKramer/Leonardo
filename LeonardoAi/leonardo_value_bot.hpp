#pragma once

#include "../MockChessEngine/Player.h"
#include "NeuroFox/neural_network.hpp"
#include "leonardo_util.hpp"
#include "stockfish_interface.hpp"

#define PRINT_SEARCH_INFO

class leonardo_value_bot : public Player
{
protected:
	neural_network value_net;

	//discencourage draw
	const std::array<float, 2> draw_score = { -1000.0f, 1000.0f };

	float dropout = 0;
	float nnet_influence = 0;
	int ms_per_move = 1000;

	float piece_value_mult = 1;
	float piece_pos_value_mult = .1f;
	float pawn_same_color_bonus_mult = 0;
	float pawn_self_protection_mult = 0;
	float passed_pawn_mult = 0;
	float king_pos_mult = 0;
	float king_safety_mult = 0;

	bool print_tree = false;

	std::unordered_map<ChessBoard, std::vector<std::string>, chess_board_hasher> opening_positions;
	bool openings_loaded;

	float get_nnet_eval(const ChessBoard& board, matrix& input_board);
	float get_simpel_eval(const ChessBoard& board);
	float get_simpel_eval(const ChessBoard& board, bool print);
	float get_eval(const ChessBoard& board, matrix& input_board);
	//float get_eval(const ChessBoard& board, matrix& input_board, float nnet_inf, float hard_infl);

	float get_move_score_recursively(
		const ChessBoard& board, //1
		int curr_depth, //2
		int max_depth, //2
		bool is_maximizing_player, //3
		float alpha, //4
		float beta, //5
		int& nodes_searched, //6
		int& end_states_searched, //7
		matrix& input_board, //8
		long long allowed_time_ms, //9
		std::chrono::steady_clock::time_point& start_time, //10
		bool& search_cancelled, //11
		std::string& best_moves_str, //12
		std::string prefix); //13

	void thread_task(
		int thread_id,
		const std::string& move_str,
		std::vector<float>& scores,
		ChessBoard board);
	
	void add_opening_position(const ChessBoard& board, const std::string& move_str);
	void load_openings();
	bool position_is_known_opening(const ChessBoard& board);
	int get_random_opening_move(const ChessBoard& board, const UniqueMoveList& legal_moves);

	leonardo_value_bot(int ms_per_move, std::string name);
public:
	leonardo_value_bot(int ms_per_move);

	void mutate(float min, float max);
	void reroll_params();
	void get_params_from_other(const leonardo_value_bot& other);
	std::string param_string();

	int getMove(const ChessBoard& board, const UniqueMoveList& legal_moves) override;
	
	void print_eval(std::string fen);
};

