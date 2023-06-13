#pragma once
#include "NeuroFox/neural_network.hpp"
#include "neural_arena.hpp"
#include <memory>
class leonardo_overlord
{
private:
	std::string name;
	std::vector<std::unique_ptr<neural_arena>> arenas;
	neural_network best_network;

	std::thread file_save_thread;
	void save_best_to_file(size_t epoch);

	std::vector<std::string> read_all_lines(const std::string& filename);
	std::vector<std::string> split(std::string str, char splitter);
public:
	leonardo_overlord(std::string name, size_t arena_count);

	void start_arenas(
		size_t epochs, 
		size_t match_count, 
		float mutation_range,
		size_t epochs_for_ever_save);

	const neural_network& get_best_network() const;

	void learn_on_existing_games(const std::string& path);
};