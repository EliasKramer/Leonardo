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

std::vector<std::string> leonardo_overlord::read_all_lines(const std::string& filename) {
	std::vector<std::string> lines;
	std::ifstream file(filename);

	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			lines.push_back(line);
		}
		file.close();
	}

	return lines;
}

std::vector<std::string> leonardo_overlord::split(std::string str, char splitter)
{
	std::vector<std::string> substrings;
	std::size_t start = 0;
	std::size_t end = str.find(splitter);

	while (end != std::string::npos) {
		std::string substring = str.substr(start, end - start);
		substrings.push_back(substring);
		start = end + 1;
		end = str.find(splitter, start);
	}

	std::string lastSubstring = str.substr(start);
	if (lastSubstring != "")
	{
		substrings.push_back(lastSubstring);
	}

	return substrings;
}

leonardo_overlord::leonardo_overlord(
	std::string name,
	size_t arena_count
) : name(name)
{
	best_network.set_input_format(vector3(8, 8, 1));
	best_network.add_fully_connected_layer(16, sigmoid_fn);
	//best_network.add_fully_connected_layer(16, sigmoid_fn);
	//for learning reasons we set to sigmoid
	best_network.add_fully_connected_layer(vector3(64, 64, 1), sigmoid_fn);
	best_network.set_all_parameters(0.0f);
	best_network.apply_noise(1);
	//best_network.enable_gpu_mode();

	for (size_t i = 0; i < arena_count; i++)
	{
		arenas.push_back(std::make_unique<neural_arena>("arena nr " + std::to_string(i), best_network));
	}
}

void leonardo_overlord::start_arenas(
	size_t epochs,
	size_t match_count,
	float mutation_range,
	size_t epochs_for_ever_save)
{
	if (epochs_for_ever_save == 0)
	{
		epochs_for_ever_save = epochs;
	}

	for (int curr_epoch = 1; curr_epoch <= epochs; curr_epoch++)
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

		//wait for file save
		if (file_save_thread.joinable())
		{
			file_save_thread.join();
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

		if (curr_epoch % epochs_for_ever_save == 0)
		{
			save_best_to_file(curr_epoch);
		}
	}
	if (file_save_thread.joinable())
	{
		file_save_thread.join();
	}
}

const neural_network& leonardo_overlord::get_best_network() const
{
	return best_network;
}

void leonardo_overlord::learn_on_existing_games(const std::string& path)
{
	std::vector<std::string> all_lines = read_all_lines(path);

	leonardo_bot bot(best_network);

	for (size_t curr_line = 0; curr_line < all_lines.size(); curr_line++)
	{
		if (file_save_thread.joinable())
		{
			file_save_thread.join();
		}

		std::vector<std::string> split_lines = split(all_lines[curr_line], ' ');

		bot.train_on_game(split_lines);

		if (curr_line % 500 == 0)
		{
			std::cout << "saving model" << std::endl;
			save_best_to_file(curr_line);
		}
		std::cout << "learn epoch: " << curr_line << "\n";
	}
}