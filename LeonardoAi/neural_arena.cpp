#include "neural_arena.hpp"

std::string neural_arena::get_name()
{
	return name;
}

void neural_arena::play_games(size_t amount)
{
	score = arena.play(amount, true);
}

neural_arena::neural_arena(
	std::string given_name,
	const neural_network& given_nn
) :
	name(given_name),
	default_network(given_nn),
	mutated_network(given_nn),
	//default is always white and mutated is always black - TODO CHANGE
	arena(
		given_name,
		std::make_unique<leonardo_bot>(default_network),
		std::make_unique<leonardo_bot>(mutated_network)
	)
{}

void neural_arena::set_parameters_from_src_and_mutate(
	const neural_network& other,
	float mutate_range)
{
	default_network.set_parameters(other);
	mutated_network.set_parameters(other);
	mutated_network.mutate(mutate_range);
}

const neural_network& neural_arena::get_mutated_nn() const
{
	return mutated_network;
}

void neural_arena::start_games_in_thread(size_t games)
{
	//to debug without thread
	//play_games(games);
	thread = std::thread(&neural_arena::play_games, this, games);
}

float neural_arena::join_and_get_score()
{
	thread.join();
	//this score assumes that the mutated network is black
	float tmp = score * -1;
	score = 0;
	return tmp;
}