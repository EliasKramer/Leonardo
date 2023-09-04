#pragma once

#include "../MockChessEngine/Player.h"
#include "NeuroFox/neural_network.hpp"
#include "leonardo_util.hpp"
#include "stockfish_interface.hpp"

class leonardo_value_bot : public Player
{
private:
	neural_network value_net;
	matrix input_board;

	float nnet_influence;
	float hard_coded_influence;
	int depth;
	bool gpu_mode;

	float get_nnet_eval(const ChessBoard& board);
	float get_simpel_eval(const ChessBoard& board);

	float get_move_score_recursively(
		const ChessBoard& board,
		int depth,
		bool isMaximizingPlayer,
		float alpha,
		float beta,
		int& nodesSearched,
		int& endStatesSearched,
		int& maxCaptureDepthReached);

	void thread_task(
		int thread_id,
		const std::string& move_str,
		std::vector<float>& scores,
		ChessBoard board);
public:
	leonardo_value_bot(neural_network given_value_nnet);
	leonardo_value_bot(
		neural_network given_value_nnet,
		int depth,
		bool gpu_mode,
		float nnet_influence,
		float hard_coded_influence);

	int getMove(const ChessBoard& board, const UniqueMoveList& legal_moves) override;
};

