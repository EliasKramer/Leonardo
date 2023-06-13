#include "leonardo_overlord.hpp"

leonardo_overlord::leonardo_overlord(size_t arena_count)
{
	best_network.set_input_format(vector3(8, 8, 1));
	best_network.add_fully_connected_layer(16, sigmoid_fn);
	//best_network.add_fully_connected_layer(16, sigmoid_fn);
	best_network.add_fully_connected_layer(vector3(64, 64, 1), relu_fn);
	best_network.set_all_parameters(0.0f);
	best_network.apply_noise(1);
	best_network.enable_gpu_mode();

	for (size_t i = 0; i < arena_count; i++)
	{
		arenas.push_back(std::make_unique<neural_arena>("arena nr " + std::to_string(i), best_network));
	}
}

void leonardo_overlord::start_arenas(
	size_t epochs, 
	size_t match_count, 
	float mutation_range)
{
	for (int curr_epoch = 0; curr_epoch < epochs; curr_epoch++)
	{
		std::cout << "start epoch " << curr_epoch << std::endl;
		for (size_t i = 0; i < arenas.size(); i++)
		{
			arenas[i]->start_games_in_thread(match_count);
		}
		std::cout << "started threads" << std::endl;

		int best_idx = 0;
		float best_score = 0;
		for (size_t i = 0; i < arenas.size(); i++)
		{
			float score = arenas[i]->join_and_get_score();
			if (score > best_score)
			{
				best_score = score;
				best_idx = i;
			}
		}
		std::cout << "joined threads" << std::endl;

		if (best_score > 0)
		{
			best_network.set_parameters(arenas[best_idx]->get_mutated_nn());
			std::cout
				<< "\n-----------------------------\n"
				<< arenas[best_idx]->get_name()
				<< " performed best. with a "
				<< best_score
				<< " score."
				<< "\n-----------------------------\n";
		}

		arenas[best_idx]->set_parameters_from_src_and_mutate(
			best_network,
			mutation_range);

		//save network
	}
}

const neural_network& leonardo_overlord::get_best_network() const
{
	return best_network;
}
