#include "leonardo_overlord.hpp"

void leonardo_overlord::save_best_to_file(size_t epoch)
{
	//best_network.save_to_file(name);
	std::string path = "models\\" + name + "_epoch_" + std::to_string(epoch) + ".parameters";

	//start thread and save once
	file_save_thread = std::thread(
		[this, path]()
		{
			best_network.save_to_file(path);
		}
	);
}

leonardo_overlord::leonardo_overlord(
	std::string name
) : name(name)
{
	//best_network = neural_network("models\\learner_epoch_500.parameters");

	best_network.set_input_format(vector3(8, 8, 1));
	best_network.add_fully_connected_layer(16, sigmoid_fn);
	best_network.add_fully_connected_layer(16, sigmoid_fn);
	best_network.add_fully_connected_layer(vector3(64, 64, 1), sigmoid_fn);
	best_network.set_all_parameters(0.0f);
	best_network.apply_noise(1);

	best_network.enable_gpu_mode();
}

leonardo_overlord::~leonardo_overlord()
{
	if (file_save_thread.joinable())
	{
		file_save_thread.join();
	}
}

const neural_network& leonardo_overlord::get_best_network() const
{
	return best_network;
}
