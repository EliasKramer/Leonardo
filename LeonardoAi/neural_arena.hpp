#pragma once
#include "chess_arena.hpp"
#include "leonardo_bot.hpp"
#include <thread>
class neural_arena
{
private:
	std::string name;
	std::thread thread;
	float score = 0;

public:
	neural_network default_network;
	neural_network mutated_network;
	chess_arena arena;

	std::string get_name();

	void play_games(size_t amount);

	neural_arena(std::string given_name, const neural_network& given_nn);

	void set_parameters_from_src_and_mutate(
		const neural_network& other,
		float mutate_range);

	const neural_network& get_mutated_nn() const;

	void start_games_in_thread(size_t games);
	float join_and_get_score();
};

