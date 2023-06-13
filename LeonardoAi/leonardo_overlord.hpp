#pragma once
#include "NeuroFox/neural_network.hpp"
#include "neural_arena.hpp"
#include <memory>
class leonardo_overlord
{
private:
	std::vector<std::unique_ptr<neural_arena>> arenas;
	neural_network best_network;

public:
	leonardo_overlord(size_t arena_count);

	void start_arenas(
		size_t epochs, 
		size_t match_count, 
		float mutation_range);

	const neural_network& get_best_network() const;
};