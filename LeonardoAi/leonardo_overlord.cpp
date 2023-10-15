#include "leonardo_overlord.hpp"
#include "./NeuroFox/util.hpp"
#include <cmath>
void leonardo_overlord::save_best_to_file(size_t epoch, bool value_nnet, bool policy_nnet)
{
	std::cout << "saving best network to file " << epoch << "\n";
	//check if folder exists
	if (!std::filesystem::exists("models"))
	{
		std::filesystem::create_directory("models");
	}
	std::string folder_name = "models/" + name + "_epoch_" + std::to_string(epoch);
	if (!std::filesystem::exists(folder_name))
	{
		std::filesystem::create_directory(folder_name);
	}
	if (policy_nnet)
	{
		std::string policy_path = folder_name + "/policy.parameters";
		best_policy_nnet.save_to_file(policy_path);
	}
	if (value_nnet)
	{
		std::string value_nnet_path = folder_name + "/value.parameters";
		best_value_nnet.save_to_file(value_nnet_path);
	}
	std::cout << "saved best network to file\n";
}

leonardo_overlord::leonardo_overlord(
	std::string name
) : name(name)
{
	std::cout << "initalizing overlord " << name << "\n";
	//print curret directory
	std::filesystem::path p = std::filesystem::current_path();
	std::cout << "looking for nnets in " << p << '\n';

	//best_value_nnet = neural_network("value.parameters");
	//best_policy_nnet = neural_network("policy.parameters");



	best_value_nnet.set_input_format(leonardo_util::get_input_format());
	//best_value_nnet.add_fully_connected_layer(96, leaky_relu_fn);
	//best_value_nnet.add_fully_connected_layer(24, leaky_relu_fn);
	//best_value_nnet.add_fully_connected_layer(24, leaky_relu_fn);
	//best_value_nnet.add_fully_connected_layer(12, leaky_relu_fn);
	//best_value_nnet.add_fully_connected_layer(12, leaky_relu_fn);
	//best_value_nnet.add_fully_connected_layer(6, leaky_relu_fn);
	//best_value_nnet.add_fully_connected_layer(6, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(leonardo_util::get_value_nnet_output_format(), identity_fn);
	best_value_nnet.xavier_initialization();

	//best_value_nnet = neural_network("C:\\Users\\Elias\\Desktop\\4small_epoch\\4small_epoch_2652200\\value.parameters");

	//best_value_nnet = neural_network("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\pre_calced_dataset_epoch_178200\\value.parameters");

	best_policy_nnet.set_input_format(leonardo_util::get_input_format());
	best_policy_nnet.add_fully_connected_layer(200, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(200, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(100, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(100, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(50, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(50, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(leonardo_util::get_policy_output_format(), leaky_relu_fn);
	best_policy_nnet.xavier_initialization();

	//best_value_nnet = neural_network("value.parameters");

	new_policy_nnet = neural_network(best_policy_nnet);
	new_value_nnet = neural_network(best_value_nnet);

	if (gpu_mode)
	{
		best_policy_nnet.enable_gpu_mode();
		best_value_nnet.enable_gpu_mode();
		new_policy_nnet.enable_gpu_mode();
		new_value_nnet.enable_gpu_mode();
	}
}
static std::vector<std::string> read_file_lines(const std::string& filename) {
	std::vector<std::string> lines;
	std::ifstream file(filename);

	if (!file.is_open()) {
		std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
		return lines;
	}

	std::string line;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}

	file.close();
	return lines;
}

static std::vector<std::string> split_string(const std::string& input, char separator) {
	std::vector<std::string> result;
	std::string current;

	for (char c : input) {
		if (c == separator) {
			if (!current.empty()) {
				result.push_back(current);
				current.clear();
			}
		}
		else {
			current += c;
		}
	}

	if (!current.empty()) {
		result.push_back(current);
	}

	return result;
}
static void convert_dataset(
	std::vector<std::string>& lines,
	std::vector <std::vector<std::string>>& game_moves,
	std::vector <std::vector<float>>& game_values)
{
	std::cout << "convert dataset\n";
	for (int i = 0; i < lines.size(); i += 2)
	{
		std::string line_m = lines[i];
		std::string line_v = lines[i + 1];
		auto m_vec = split_string(line_m, ';');
		auto v_vec = split_string(line_v, ';');

		if (m_vec.size() != 2)
		{
			std::cout << "error in dataset line " << i << " master moves size is not 2\n";
			continue;
		}
		if (m_vec.size() != v_vec.size())
		{
			std::cout << "error in dataset line " << i << " sizes are not equal\n";
			return;
		}
		if (m_vec[0] != v_vec[0])
		{
			std::cout << "error in dataset line " << i << " master moves are not equal\n";
			continue;
		}

		auto moves = split_string(m_vec[1], ' ');
		auto values_s = split_string(v_vec[1], ' ');

		if (moves.size() != values_s.size())
		{
			std::cout << "error in dataset line " << i << " sizes are not equal\n";
			continue;
		}

		game_moves.push_back(moves);
		std::vector<float> values;
		for (auto v : values_s)
		{
			float value = std::stof(v);
			value = std::clamp(value, -10.0f, 10.0f);
			values.push_back(value);
		}
		game_values.push_back(values);
	}
	std::cout << "convertion done\n";
}

void leonardo_overlord::train_value_nnet()
{
	best_value_nnet = neural_network(); //reset the best nnet
	best_value_nnet.set_input_format(leonardo_util::get_pawn_input_format()); //2880
	best_value_nnet.add_fully_connected_layer(200, leaky_relu_fn); //900
	best_value_nnet.add_fully_connected_layer(100, leaky_relu_fn); //900
	best_value_nnet.add_fully_connected_layer(50, leaky_relu_fn); //900
	best_value_nnet.add_fully_connected_layer(leonardo_util::get_value_nnet_output_format(), identity_fn);
	best_value_nnet.xavier_initialization();

	std::cout << "read data\n";
	std::vector<std::string> lines = read_file_lines("dataset.txt");
	std::cout << "finished reading data\n";
	std::vector <std::vector<std::string>> game_moves;
	std::vector <std::vector<float>> game_values;

	convert_dataset(lines, game_moves, game_values);
	lines.clear();

	if (game_moves.size() != game_values.size())
	{
		std::cout << "error in dataset\n";
		return;
	}

	const int games_per_training = 1000;

	matrix input(leonardo_util::get_pawn_input_format());
	matrix label(leonardo_util::get_value_nnet_output_format());

	std::vector<matrix> inputs;
	std::vector<matrix> labels;
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < game_moves.size(); i++)
	{
		std::vector<std::string>& moves_uci = game_moves[i];
		std::vector<float>& values = game_values[i];

		chess::Board board = chess::Board(DEFAULT_FEN);

		for (int j = 0; j < moves_uci.size(); j++)
		{
			std::string uci_move = moves_uci[j];
			float value = values[j];
			if (board.sideToMove() == chess::Color::BLACK)
			{
				value = -value;
			}

			chess::Movelist moves;
			chess::movegen::legalmoves(moves, board);
			bool move_found = false;
			for (int m = 0; m < moves.size(); m++)
			{
				chess::Move move = moves[m];
				if (uci_move == chess::uci::moveToUci(move))
				{
					board.makeMove(move);

					if (leonardo_util::board_material_equal(board))
					{
						value /= 10;
						leonardo_util::encode_pawn_matrix(board, input);
						inputs.push_back(input);

						leonardo_util::set_pawn_matrix_value(label, value, board.sideToMove());
						labels.push_back(label);
					}
					move_found = true;
					break;
				}
			}
			if (!move_found)
			{
				std::cout << "error in dataset line " << i << " move not found\n";
			}
		}

		if (inputs.size() == 0 || labels.size() == 0)
		{
			std::cout << "error in dataset line " << i << " no data\n";
			continue;
		}

		if ((i + 1) % games_per_training == 0)
		{
			data_space ds(
				leonardo_util::get_pawn_input_format(),
				leonardo_util::get_value_nnet_output_format(),
				inputs,
				labels);

			std::cout << "testing\n";
			test_result res = best_value_nnet.test_on_ds(ds);
			std::cout << res.to_string() << "\n";
			std::cout << "learning \n";
			best_value_nnet.learn_on_ds(ds, 1, 128, 0.0001f, false);

			inputs.clear();
			labels.clear();

			save_best_to_file(i, true, false);

			auto stop = std::chrono::high_resolution_clock::now();
			long duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
			long remaining_ms = remaining_time(duration_ms, i, game_moves.size());

			std::cout << (i + 1) << "/" << game_moves.size() << " games  " << ((((float)i + 1) / (float)game_moves.size()) * 100.0f) << "%\n";
			std::cout << "elapsed: " << ms_to_str(duration_ms) << " remaining: " << ms_to_str(remaining_ms) << "\n";
			std::cout << "---------------\n";
		}
	}

	return;
}

void leonardo_overlord::test_value_nnet_single(std::string& game)
{
	std::vector<std::string> master_moves = split_string(game, ' ');
	matrix input = leonardo_util::get_input_format();

	chess::Board board = chess::Board(DEFAULT_FEN);
	float diff_sum = 0;
	for (size_t i = 0; i < master_moves.size(); i++)
	{
		chess::Movelist moves;
		chess::movegen::legalmoves(moves, board);

		bool exists = false;
		for (size_t j = 0; j < moves.size(); j++)
		{
			if (chess::uci::moveToUci(moves[j]) == master_moves[i])
			{
				exists = true;

				float sf_eval = stockfish_interface::eval(board.getFen(), 4);
				float nn_eval = leonardo_util::get_value_nnet_eval(best_value_nnet, input, board, true);

				nn_eval = nn_eval * nn_eval * nn_eval;

				std::cout
					<< "sf_eval: " << sf_eval << "\t nn_eval: " << nn_eval
					<< "\t diff: " << std::abs(sf_eval - nn_eval) << std::endl;

				diff_sum += std::abs(sf_eval - nn_eval);

				board.makeMove(moves[j]);
				break;
			}
		}
		if (!exists)
		{
			std::cout << chess::uci::moveToUci(moves[i]) << " move not found: " << master_moves[i] << std::endl;
			continue;
		}
	}
	std::cout << "avg diff: " << diff_sum / master_moves.size() << std::endl;

}
void leonardo_overlord::test_value_nnet()
{
	std::vector<std::string> games = {
		"g1f3 c7c5 g2g3 g7g6 f1g2 f8g7 c2c4 b8c6 b1c3 e7e6 d2d3 g8e7 c1d2 e8g8 a2a3 b7b6 a1b1 c8b7 b2b4 c5b4 a3b4 c6e5 f3h4 b7g2 h4g2 d7d5 f2f4 e5c6 b4b5 c6a5 c4d5 e7d5 c3d5 d8d5 e1g1 a5b7 d1b3 d5d7 f1c1 f8c8 b3a4 b7c5 a4a3 g7f8 a3a1 c5e4 d2e3 f8g7 c1c8 a8c8 a1a6 e4c3 b1e1 c3b5 a6a4 c8c7 g1f2 b5c3 a4d7 c7d7 g2h4 c3d5 h4f3 d5e3 f2e3 d7c7 e1b1 g8f8 d3d4 f8e8 e3d3 e8d8 e2e4 d8c8 b1b3 c8b7 f3e5 f7f6 e5c4 c7d7 c4a5 b7a8 a5c4 d7d8 h2h3 g7f8 d3e3 d8b8 e4e5 f6e5 f4e5 b6b5 c4a5 b5b4 e3e4 b8b5 b3f3 b5a5 f3f8 a8b7 f8f7 b7c8 f7f8 c8c7 f8f7 c7c6 f7e7 b4b3 e7e6 c6d7 e6d6 d7c7 d6f6 b3b2 f6f7 c7b6 f7f6 b6b5 f6f1 a5a1",
		"g1f3 g8f6 b2b3 g7g6 c1b2 f8g7 g2g3 e8g8 f1g2 c7c5 c2c4 d7d6 e1g1 e7e5 d2d3 b8c6 b1c3 c8e6 f3d2 d8d7 d2e4 e6h3 e4f6 g7f6 c3d5 f6g7 f2f4 a8e8 f4e5 c6e5 d1d2 h3g2 g1g2 f7f5 h2h3 e5c6 a1e1 c6e7 b2g7 e7d5 g7h6 d5f6 h6f8 e8f8 e2e4 d7c6 g2h2 f5e4 d3e4 g8g7 d2c3 c6a6 e1e2 a6b6 e2f2 b6d8 e4e5 d6e5 c3e5 h7h6 e5c5 h6h5 c5e5 b7b6 b3b4 f8f7 c4c5 b6c5 b4c5 f7f8 c5c6 f8f7 f2f6 f7f6 e5f6 d8f6 f1f2 f6f4 f2f4",
		"c2c4 g7g6 g2g3 f8g7 f1g2 g8f6 b1c3 e8g8 d2d3 c7c5 e2e3 b8c6 g1e2 d7d6 e1g1 d8c7 e2f4 c8d7 a1b1 e7e6 a2a3 c6e7 c1d2 d7c6 g2c6 c7c6 d1a4 c6c8 b2b4 b7b6 f2f3 f8d8 f1c1 f6d7 a4d1 d7e5 d1e2 c8b7 c1f1 a8c8 b1b3 b7c6 b4b5 c6b7 e3e4 a7a6 f1b1 a6b5 c3b5 d6d5 c4d5 e6d5 b5c3 d5e4 d3e4 d8d2",
		"c2c4 e7e5 g2g3 g8f6 f1g2 b8c6 b1c3 f8b4 g1f3 e8g8 e1g1 d7d6 d2d3 b4c3 b2c3 e5e4 f3d4 e4d3 e2d3 c6e5 f2f4 e5d7 a1b1 c7c5 d4c2 d7b6 f4f5 d6d5 c4d5 b6d5 b1b3 a7a5 c1d2 a8a7 g3g4 a5a4 b3b2 h7h6 c3c4 d5e7 d2c3 e7c6 c2e3 c6d4 e3d5 a4a3 b2f2 c8d7 c3d4 c5d4 f2f4 d7c6 d5f6 d8f6 d1a1 c6g2 g1g2 f8d8 f4e4 a7a6 f1f2 a6b6 a1c1 b6a6 c1f4 f6d6 f4d6 a6d6 e4e5 d6b6 e5a5 b6a6 a5a6 b7a6 c4c5 d8b8 g2f3 g8f8 c5c6 f8e7 c6c7 b8c8 f2c2 e7d7 c2c4 c8c7 c4c7 d7c7 f3e4 c7c6 e4d4 c6b5 d4c3 g7g6 f5g6 f7g6 h2h4 g6g5 h4g5 h6g5 d3d4 a6a5 c3d3 b5c6 d3e4 c6d6 d4d5 a5a4 e4d4 d6d7 d4e5 d7e7 d5d6 e7d7 e5d5 d7d8 d5e6 d8e8 e6f6 e8d7 f6g5 d7d6 g5f6 d6d5 g4g5 d5d4 g5g6 d4c3 g6g7",
		"g1f3 g8f6 c2c4 g7g6 b1c3 d7d5 d1a4 c7c6 c4d5 b7b5 a4d4 c6d5 e2e4 b8c6 f1b5 c8d7 b5c6 d7c6 f3e5 c6b7 d4a4 f6d7 e4d5 f8g7 d2d4 f7f6 e5c6 d8c7 e1g1 e8g8 c6e7 g8h8 e7c6 d7b6 a4a5 b6d5 a5c7 d5c7 c6a5 b7a6 f1d1 a8d8 c1e3 c7d5 a5c6 d5e3 f2e3 d8d7 c6b4 a6b7 b4d3 g7h6 g1f2 d7e7 d1e1 f8e8 d3c5 h6e3 e1e3 e7e3 c5b7 e3e7 b7c5 e8b8 b2b3 b8d8 a1d1 f6f5 d4d5 e7e5 b3b4 h8g8 d5d6 g8f7 d6d7 f7e7 c3b5",
		"c2c4 c7c5 g2g3 g8f6 f1g2 b8c6 g1f3 e7e6 e1g1 h7h5 e2e3 d7d5 d2d3 f8d6 b1c3 a7a6 e3e4 d5d4 c3e2 f6d7 c1f4 d6f4 g3f4 e6e5 d1d2 d8f6 g1h1 e8f8 a2a3 e5f4 e2f4 d7e5 f3e5 c6e5 f4d5 f6d6 f2f4 e5g4 e4e5 d6d8 d2e1 c8e6 a1c1 e6d5 c4d5 g4e3 c1c5 e3f1 e1f1 g7g6 f1f2 a8c8 f2d4 c8c5 d4c5 f8g7 d5d6 d8h4 c5c3 h8c8 c3d2 c8c2 d2d1 h4f2 d1f1 f2d4 h2h3 c2b2 f1f3 b2f2 f3e4 f2f4 e4d4 f4d4 g2b7 d4d3 b7a6 d3h3 h1g2 h3a3 d6d7 a3a2 g2f3 a2d2 a6b5 g7f8 f3e3 d2d1 b5d3",
		"c2c4 e7e6 b1c3 d7d5 e2e3 g8f6 g1f3 f8e7 b2b3 c7c5 c1b2 b8c6 c4d5 e6d5 d2d4 d8a5 d1d2 e8g8 f1e2 c5d4 f3d4 c6d4 e3d4 c8f5 e1g1 a8c8 d2f4 f5g6 a1c1 e7b4 f4g3 b4c3 b2c3 a5a2 c3b4 a2e2 b4f8 c8f8 g3d6 f8e8 d6b4 e2g4 b4b7 g6d3 f1e1 d3e4 f2f3 e4f3 e1e8 f6e8 c1c2 g4d4 c2f2 f3h5 b7d7 f7f6 h2h3 h5g6 b3b4 h7h5 b4b5 g8h7 h3h4 d4c5 d7c6 c5d4 c6d7 d4b6 d7d5 e8d6 d5c6 b6d4",
		"g1f3 b7b6 e2e4 c8b7 b1c3 e7e6 d2d4 f8b4 f1d3 g8f6 e4e5 f6e4 e1g1 e4c3 b2c3 b4c3 a1b1 b8c6 d3e4 b7a6 b1b3 a6f1 d1f1 c3a5 c1a3 f7f5 e5f6 d8f6 f1c4 e8c8 f3e5 c6b8 b3f3 d7d5 c4a4 f6e5 d4e5 d5e4",
		"g1f3 g8f6 g2g3 b7b6 f1g2 c8b7 c2c4 c7c5 b1c3 g7g6 d2d4 c5d4 d1d4 b8c6 d4f4 f8g7 e1g1 a8c8 a1b1 d7d6 b2b3 e8g8 f4h4 c6b8 c1e3 a7a6 b1c1 b8d7 g2h3 c8c7 g3g4 h7h5 g4h5 g6h5 f3d4 e7e6 f2f3 d7e5 h4g3 g8h7 h3g2 f8g8 e3g5 g7h6 h2h4 c7c5 f3f4 b7g2 g1g2 e5g4 g3d3 h7h8 d4f3 d8c7 g2h3 f6e8 c3e4 c5f5 e4g3 f5f4 g3h5 f4f5 h3g4 f7f6 h5g3 f6g5 h4g5 f5g5 f3g5 g8g5 g4h3 c7g7 f1f3 g5g6 c1f1 e8f6 d3d6 h6g5 f1h1 g5h4 d6b8 h8h7 h3g2 g6h6 b8f4 h4g5 h1h6 h7h6 f4e5 g7e7 g3f5",
		"c2c4 g7g6 g1f3 f8g7 g2g3 c7c5 f1g2 b8c6 e1g1 d7d6 b1c3 e7e6 b2b3 g8e7 c1b2 e8g8 d2d3 d6d5 c4d5 e6d5 d1d2 d5d4 c3e4 b7b6 h2h4 h7h6 d2c2 c8g4 a2a3 a7a5 f1e1 d8d7 a1b1 a8d8 b1d1 f7f5 e4d2 f5f4 d2e4 e7d5 b2c1 d7c7 g1h1 c6e5 f3h2 c7e7 e1f1 g6g5 h4g5 h6g5 h2g4 e5g4 g2f3 g4e5 f3g2 g5g4 g3f4 e7h4 h1g1 d5f4 c1f4 f8f4 b3b4 f4f5 e4g3 f5f6 b4c5 d8f8 c2b3 g8h8 b3d5 f6h6 f1e1 h4h2 g1f1 h2g3 d5d4 h6h2",
		"g1f3 g8f6 g2g3 d7d5 f1g2 g7g6 b2b3 f8g7 c1b2 e8g8 e1g1 c7c5 c2c4 d5d4 b3b4 b8c6 b4c5 f6d7 d2d3 d7c5 b1d2 f8e8 b2a3 d8a5 a3c5 a5c5 d1b3 a8b8 a1b1 c8d7 d2e4 c5a5 b3b5 a5c7 e4c5 c6e5 c5d7 e5d7 f3d2 a7a6 b5b4 g7f8 d2e4 e7e5 b4a4 f7f5 a4d7",
		"g1f3 d7d5 g2g3 g8f6 f1g2 c8g4 e1g1 c7c6 d2d3 b8d7 c2c4 e7e6 c1e3 f8d6 d1b3 d8c7 b1c3 e8g8 a1c1 a8c8 c4d5 e6d5 c3b5 c7b8 b5d6 b8d6 b3b7 g4f3 g2f3 d7e5 e3c5 e5f3 e2f3 d6e6 c5f8 c8f8 b7c6 e6h3 f1e1 h3h5 g1g2 h5g6 d3d4 g6g5 c6d6 g5d2 d6f8",
		"c2c4 e7e5 b1c3 d7d6 g2g3 f8e7 d2d4 e5d4 d1d4 g8f6 f1g2 b8c6 d4d2 c6e5 b2b3 c7c6 g1f3 e5f3 g2f3 c8h3 c1b2 e8g8 e1c1 h3e6 g3g4 f6d7 c1b1 a7a5 c3e4 d6d5 c4d5 c6d5 e4g3 a5a4 f3d5 e6g4 h2h3 a4b3 a2b3 e7g5 f2f4 g5f6 h3g4 f6b2 d2b2 d8c7 g3f5 d7f6 g4g5 c7f4 f5e7",
		"c2c4 e7e5 b1c3 g8f6 g1f3 b8c6 e2e4 f8b4 d2d3 d7d6 a2a3 b4c3 b2c3 e8g8 g2g3 f6d7 f3h4 d7c5 f1g2 c6e7 c1e3 c8e6 e1g1 d8d7 f2f4 e5f4 g3f4 f7f5 e4e5 c5a4 h4f3 a4c3 d1b3 c3a4 f3g5 a4b6 a3a4 a8b8 a4a5 b6c8 d3d4 h7h6 g5e6 d7e6 d4d5 e6f7 g1h1 e7g6 e3d4 a7a6 b3g3 c8e7 e5e6 f7e8 g2f3 c7c5 d4c3 b7b5 a5b6 b8b6 a1a3 e8b8 f3h5 b6b1 a3a1 b1a1 c3a1 g8h7 f1g1 b8e8 h2h4 f8g8 a1c3 g8f8 h5d1 f8g8 h4h5 g6e5 f4e5 d6e5 g3e5 g7g5 d5d6 e8c6 h1h2 e7c8 e5f5",
		"c2c4 e7e5 b1c3 b8c6 g2g3 f8c5 f1g2 d7d6 e2e3 h7h5 h2h4 a7a6 g1e2 c5a7 d2d4 g8f6 e1g1 e8g8 d4e5 c6e5 b2b3 c8g4 g2b7 c7c6 b7a8 g4f3 d1c2 d8d7 e2f4 f8a8 e3e4 d7g4 c1e3 a7e3 f2e3 g4g3 f4g2 f6g4 f1f3 e5f3 g1f1 g4h2 f1e2 g3g2",
		"b1c3 c7c5 g1f3 b8c6 d2d4 c5d4 f3d4 g8f6 c1g5 e7e6 d1d3 f8e7 e2e4 e8g8 e1c1 d7d6 f2f4 c8d7 d3g3 a8c8 f1e2 g8h8 c1b1 a7a6 e4e5 f6e8 e5d6 e8d6 d4c6 c8c6 g5e7 d8e7 g3d3 b7b5 e2f3 c6b6 h1e1 e7f6 d3d4 f6d4 d1d4 d7e8 e1d1 d6f5 d4d8 h8g8 d8a8 f7f6 d1d8 g8f7 c3e4 b6c6 e4g5 f6g5 f3c6 e8c6 d8f8 f7g6 a8a6 c6d5 g2g4 f5d4 f4f5 g6h6 f8d8",
		"g1f3 g7g6 h2h4 f8g7 h4h5 d7d5 d2d4 c8g4 h5h6 g7f8 f3e5 g4e6 c2c4 d5c4 e2e4 c7c6 b1c3 g8f6 c1g5 b7b5 f2f3 f6d7 e5c6 b8c6 d4d5 d7c5 d5c6 d8d1 a1d1 a8c8 c3b5 c8c6 b5d4 c6c8 g5e3 a7a6 d4e6 c5e6 d1c1 c8b8 f1c4 b8b2 c4e6",
		"b2b3 e7e5 c1b2 b8c6 e2e3 g7g6 d2d3 f8g7 b1d2 d7d5 a2a3 g8f6 f1e2 e8g8 g2g4 f8e8 h2h4 d8d7 g4g5 f6g4 h4h5 d7f5 g1h3 g6h5 d2f3 f5e6 f3h4 c6e7 h4g2 e7f5 d1d2 e6b6 d2b4 b6c6 b4d2 h7h6 g5h6 g7h6 h3g1 d5d4 e2f3 c6g6 f3e4 d4e3 f2e3 g4e3 g2e3 h6e3 d2h2 g6g3 e1e2 g3h2 h1h2 e3f4 h2g2 g8f8 a3a4 f5e3 b2a3 e8e7 a3e7 f8e7 g2g7 e3c2 a1f1 c8g4 e4f3 c2d4 e2f2 d4f3 g1f3 e7f6 g7g4 h5g4 f3e1 a8h8 e1g2 h8h3 f1d1 h3f3 f2e2 f3g3 e2f2 g3f3 f2e2 f3h3 d3d4 h3h2 e2f2 e5e4 f2g1 f4d6 d1e1 f6f5 e1f1 f5e6 d4d5 e6e7 g2e3 h2b2 e3f5 e7e8 f1e1 d6c5 g1h1 e4e3 f5e3 c5e3 e1e3 e8d7 h1g1 f7f5 e3e5 f5f4 e5f5 f4f3 f5g5 b2g2 g1f1 d7d6 b3b4 g4g3 a4a5 a7a6 g5f5 g2f2 f1g1 d6e7 f5g5 f2g2 g1f1 e7f6 g5g8 f6e5 g8g7 e5d5 g7c7 g2f2 f1g1 f2b2",
		"c2c4 c7c5 g2g3 g8f6 f1g2 b8c6 b1c3 g7g6 d2d3 f8g7 d1d2 d7d6 b2b3 e7e5 e2e3 c8e6 c1b2 d6d5 c4d5 f6d5 c3e4 b7b6 g1f3 h7h6 a2a3 a8c8 h2h4 e8g8 h4h5 g6g5 a1d1 f7f5 e4c3 d5c3 d2c3 d8e7 f3d2 f8d8 d2c4 c6d4 e1f1 d4b5 c3c2 e6f7 g3g4 f5g4 g2e4 b5c7 c2e2 c7e8 e4f5 c8c7 f5g4 b6b5 c4d2 e8f6 g4f5 e7e8 f5g6 f7g6 h5g6 e8g6 d2f3 f6g4 h1g1 h6h5 f1g2 c7f7 e3e4 d8f8 d1d2 f7f3 e2f3 f8f3 g2f3 g6f7 f3g2 f7f4 d2e2 f4h2 g2f1 h2h3 f1e1 h3d3 e2d2 d3e4 e1d1 e4b1",
		"g1f3 b7b6 g2g3 c8b7 f1g2 g7g6 e1g1 f8g7 a2a4 e7e5 a4a5 g8e7 c2c4 e8g8 b1c3 d7d6 d2d4 e5d4 f3d4 b7g2 g1g2 b8c6 d4c6 e7c6 a5a6 g7c3 b2c3 c6e5 d1d4 d8e7 c1h6 f8e8 a1d1",
		"g2g3 g8f6 g1f3 b7b6 f1g2 c8b7 e1g1 g7g6 d2d4 f8g7 c2c4 e8g8 d4d5 d8e8 b1c3 e7e5 e2e4 d7d6 a2a3 a7a5 a1b1 b7a6 f3d2 c7c5 d5c6 b8c6 c3d5 f6d5 e4d5 c6d4 b2b3 b6b5 d2e4 e8d8 c4c5 b5b4 f1e1 d6c5 e4c5 d8d6 c5e4 d6b6 a3b4 a5b4 c1e3 f8d8 b1c1 a6b7 d5d6 f7f5 e4c5 b7g2 g1g2 b6d6 e3d4 d6d4 d1f3 d4d5 f3d5 d8d5 c5e6 a8a7 c1c8 g8f7 e6g5 f7e7 g5h7 g7h6 h2h4 e7e6 c8c6 d5d6 c6d6 e6d6 h7g5 h6g5 h4g5 a7a3 e1e3 d6d5 e3d3 d5e4 d3d6 a3b3 d6g6 f5f4 g6b6 f4f3 g2h3 e4f5 h3h4 b3b1 b6f6 f5e4 g5g6 b1a1 g6g7 a1a8 f6f8 a8a6 g7g8q",
		"g1f3 d7d5 g2g3 g7g6 f1g2 f8g7 c2c4 d5c4 b1a3 b8c6 a3c4 e7e5 e1g1 e5e4 f3e1 g8f6 b2b3 e8g8 c1b2 f8e8 d2d3 c8f5 a1c1 e4d3 e1d3 d8e7 d3f4 a8d8 d1e1 c6d4 e2e3 d4c2 e1e2 c2b4 c1d1 g6g5 a2a3 g5f4 a3b4 f4e3 c4e3 f5e6 d1d8 e8d8 e3f5 e7b4 f5g7 d8d2 e2e5 f6g4 e5c3 b4c3 b2c3 d2c2 g7e6 c2c3 e6d4 c7c5 d4f5 c3b3 f1d1 h7h5 d1d8 g8h7 g2d5 b3b1 g1g2 b1b2 d5e4 g4f2 g2f3 f2e4 f3e4 h7g6 f5e7 g6f6 e7d5 f6g7 h2h4 b7b5 d5f4 c5c4 f4h5 g7h7 d8c8 a7a5 g3g4 a5a4 g4g5 a4a3 h5f6",
		"g1f3 g8f6 c2c4 g7g6 b2b3 f8g7 c1b2 e8g8 g2g3 c7c5 f1g2 d7d6 d2d4 d8a5 b2c3 a5c7 b1d2 c5d4 f3d4 a7a6 e1g1 b8c6 d1c1 e7e5 d4c2 c8e6 c2e3 c6e7 c1b2 a8c8 a1c1 c7b8 g1h1 b7b5 b2a1 b5b4 c3b2 f6d7 d2e4 d7c5 e4g5 e6d7 c1d1 d7c6 e3d5 e7d5 g2d5 h7h6 g5f3 c6d5 d1d5 c8c6 f1d1 f8e8 h1g1 a6a5 a1b1 b8c7 f3e1 c5e6 e1g2 f7f5 g2e3 e8d8 b1c2 c7e7 d5a5 e7c7 a5b5 c6a6 e3d5 c7f7 b5b4 a6a2 b4a4 a2a4 b3a4 f5f4 a4a5 e6c5 d1a1 g6g5 a5a6 d8a8 b2c3 a8a6 a1a6 c5a6 c2a2 a6c5 a2a8 g8h7 a8c8 f4f3 c8g4 e5e4 c3g7 h7g7 e2f3 e4f3 h2h4 f7g6 h4g5 g6g5 g4f3 g5c1 g1g2 c1c4 f3f6 g7h7 f6f7 h7h8 f7f8 h8h7 f8f5 h7g7 f5f6 g7h7 d5f4 c4e4 g2h2 c5d3 f6f7 h7h8 f4g6",
		"g1f3 g8f6 c2c4 c7c5 d2d4 c5d4 f3d4 b8c6 b1c3 e7e6 d4c2 f8e7 e2e4 e8g8 f1e2 b7b6 e1g1 c8b7 f2f4 d7d6 b2b3 a8c8 c1b2 a7a6 g1h1 d8c7 a1c1 c6b8 e2d3 b8d7 b3b4 f8d8 d1e2 e7f8 c1e1 g7g6 a2a3 f8g7 c2e3 c7b8 e3d1 b8a8 f1f3 d8e8 f3h3 c8d8 d1f2 e6e5 f4e5 d7e5 d3b1 b7c8 h3g3 a8c6 b1a2 f6h5 g3e3 g7h6 c3d5 h6e3 e2e3 c8e6 e1c1 e6d5 c4d5 c6b5 g2g4 h5f6 g4g5 f6g4 f2g4 e5g4 e3d4 g4e5 a2c4 b5d7 c4a6 d7g4 a6f1 d8c8 c1c6 g4g5 f1h3 c8d8 c6b6 g5h4 h1g2 h4f4 g2h1 g6g5 b6c6 f4h4 h3f1 f7f6 c6c2 e5g4 h1g1 d8c8 c2g2 g4e5 g1h1 h4e1 d4f2 e1e4 f2f6 e8f8 f6g5 e5g6 h1g1 f8f1 g1f1 c8f8 g2f2 f8f2 f1f2 e4c2 f2e1 c2b2 g5d8 g8g7 d8d7 g7h6 d7d6 b2a3 e1d2 a3b3 d6c5 g6f4 d5d6 b3d3 d2c1 d3e2 c5d4 f4d3 c1b1 e2d1",
		"g1f3 e7e6 c2c4 c7c5 b1c3 b8c6 g2g3 g8f6 f1g2 f8e7 e1g1 e8g8 b2b3 a7a6 c1b2 d8c7 a1c1 b7b6 d2d4 c5d4 f3d4 c8b7 c3d5 c7d8 d5e7 d8e7 e2e4 f8d8 d1e2 d7d6 f1d1 c6d4 d1d4 b6b5 b2a3 b5c4 d4c4 f6e8 e2e1 e7g5 h2h4 g5h5 c4b4 a8b8 b4b6 h7h6 e1b4 a6a5 b4b5 h5b5 b6b5 e8f6 b5a5 b7e4 a5a7 e4g2 g1g2 b8a8 c1c7 a8a7 c7a7 f6e4 g2f3 d6d5 a3b2 d8c8 b2d4 e4d6 d4e5 d6e4 a7c7 c8a8 c7c2 f7f6 e5c7 a8c8 f3e3 e6e5 b3b4 e4d6 c2c5 d6b7 c5c6 c8a8 c6b6 a8a3 e3e2 a3a2 e2f1 a2a7 f1g2 d5d4 g2f3 b7c5 b6b8 g8f7 c7e5 c5d7 b4b5"
	};

	//std::vector<std::pair<

	for (int i = 0; i < games.size(); i++)
	{
		leonardo_overlord::test_value_nnet_single(games[i]);
	}

}

static bool get_pawn_moves(chess::Board& board, chess::Movelist& moves)
{
	chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN);
	chess::Bitboard kings = board.pieces(chess::PieceType::KING);
	chess::Bitboard black_bb = board.us(chess::Color::BLACK);
	chess::Bitboard white_bb = board.us(chess::Color::WHITE);

	chess::Bitboard allowed_squares = 0xffff00000000ffffULL;
	chess::Bitboard black_pawns = black_bb & pawns;
	chess::Bitboard white_pawns = white_bb & pawns;
	allowed_squares = allowed_squares & ~(black_pawns | white_pawns);
	allowed_squares = allowed_squares & ~(black_pawns >> 8 | white_pawns << 8);

	bool king_is_on_allowed_square = ((1ULL << board.kingSq(board.sideToMove())) & allowed_squares) != 0;

	bool has_moves = false;

	if (king_is_on_allowed_square)
	{
		for (chess::Move& m : moves)
		{
			if (((1ULL << m.from()) & pawns) != 0)
			{
				if (m.typeOf() == chess::Move::PROMOTION)
				{
					if (m.promotionType() == chess::PieceType::QUEEN)
					{
						m.setScore(2.0f);
						has_moves = true;
					}
					else
					{
						m.setScore(0.0f);
					}
				}
				else
				{
					m.setScore(1);
					has_moves = true;
				}
			}
			else
			{
				m.setScore(0);
			}
		}
	}
	if (!has_moves)
	{

		for (chess::Move& m : moves)
		{
			bool is_king = ((1ULL << m.from()) & kings) != 0;
			bool is_allowed = ((1ULL << m.to()) & allowed_squares) != 0;
			if (is_king && is_allowed) //pacifist king moves
			{
				m.setScore(3);
				has_moves = true;
			}
			else
			{
				m.setScore(0);
			}
		}
	}


	return has_moves;
}

static float pawn_eval(chess::Board& board, int depth)
{
	chess::Bitboard black_bb = board.us(chess::Color::BLACK);
	chess::Bitboard white_bb = board.us(chess::Color::WHITE);

	chess::Bitboard queen_bb = board.pieces(chess::PieceType::QUEEN);

	if (queen_bb != 0) // there is a winner
	{
		if ((queen_bb & white_bb) != 0)
		{
			return 20 - depth; //white won
		}
		else
		{
			return -20 + depth; //black won
		}
	}

	float score = 0;

	chess::Bitboard pawn_bb = board.pieces(chess::PieceType::PAWN);

	while (pawn_bb)
	{
		unsigned int sq = chess::builtin::poplsb(pawn_bb);
		chess::Bitboard curr_bb = chess::Bitboard(1) << sq;
		if ((curr_bb & black_bb) != 0)
		{
			score -= 1;
		}
		else
		{
			score += 1;
		}
	}

	return score;
}

static bool queen_on_board(chess::Board& board)
{
	return board.pieces(chess::PieceType::QUEEN) != 0;
}

float best_pawn_move_rec(
	chess::Board& board,
	chess::Move& best_move,
	int curr_depth,
	int max_depth,
	float alpha,
	float beta)
{
	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);
	bool pawn_moves_available = get_pawn_moves(board, moves);


	if (curr_depth >= max_depth || queen_on_board(board))
	{
		return pawn_eval(board, curr_depth);
	}

	if (!pawn_moves_available)
	{
		return 0;
	}

	bool white_to_move = board.sideToMove() == chess::Color::WHITE;

	float best_score = white_to_move ? -FLT_MAX : FLT_MAX;

	for (chess::Move& m : moves)
	{
		if (m.score() == 0)
			continue;

		board.makeMove(m);
		float score = best_pawn_move_rec(board, best_move, curr_depth + 1, max_depth, alpha, beta);
		board.unmakeMove(m);

		if (white_to_move)
		{
			if (score > best_score)
			{
				if (curr_depth == 0)
					best_move = m;
				best_score = score;
			}

			alpha = std::max(alpha, score);
		}
		else
		{
			if (score < best_score)
			{
				if (curr_depth == 0)
					best_move = m;
				best_score = score;
			}
			beta = std::min(beta, score);
		}
		//if (curr_depth == 0)
		//{
		//	std::cout << "move: " << chess::uci::moveToUci(m) << " score: " << score << "\n";
		//}

		if (beta <= alpha)
			break;
	}

	return best_score;
}

static void play_pawn_game(
	std::string fen,
	std::vector<matrix>& input_matrices,
	std::vector<matrix>& output_matrices)
{
	chess::Board board(fen);

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	get_pawn_moves(board, moves);

	chess::Move best_move;

	matrix board_matrix(leonardo_util::get_pawn_input_format());
	std::vector<float> scores;
	do
	{
		best_move = chess::Move::NULL_MOVE;

		best_pawn_move_rec(
			board,
			best_move,
			0,
			8,
			-FLT_MAX,
			FLT_MAX);

		leonardo_util::encode_pawn_matrix(board, board_matrix);
		float board_eval = pawn_eval(board, 0);

		input_matrices.push_back(board_matrix);
		scores.push_back(board_eval);

		if (best_move != chess::Move::NULL_MOVE)
			board.makeMove(best_move);

		//std::cout << "best move: " << chess::uci::moveToUci(best_move) << " score: " << board_eval << "\n";
		//std::cout << board.getFen() << std::endl;
		//std::cout << board << std::endl;
	} while (best_move != chess::Move::NULL_MOVE);

	float discount_factor = 0.9f;
	std::vector<float> discounted_scores;
	discounted_scores.resize(scores.size());
	discounted_scores[scores.size() - 1] = scores[scores.size() - 1];
	for (int i = scores.size() - 2; i >= 0; i--)
	{
		float curr_score = scores[i];
		float next_score = discounted_scores[i + 1];

		discounted_scores[i] = curr_score + discount_factor * next_score;
	}

	matrix label(leonardo_util::get_value_nnet_output_format());
	for (int i = 0; i < scores.size(); i++)
	{
		leonardo_util::set_pawn_matrix_value(label, discounted_scores[i] / 20); // normalize
		output_matrices.push_back(label);

		std::cout << "score: " << scores[i] <<
			" discounted score: " << discounted_scores[i] / 20 << "\n";
	}
	//important. the last move is always a win loss or a draw
	//win loss is a queen on either board. a pawn matrix cannot represent this
	output_matrices.pop_back();
	input_matrices.pop_back();

}

static chess::Board random_pawn_board()
{
	chess::Board board("4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1");

	std::random_device rd;
	std::mt19937 gen(rd());

	chess::Move chosen_move = chess::Move::NULL_MOVE;

	for (int i = 0; i < 10; i++)
	{
		chosen_move = chess::Move::NULL_MOVE;

		chess::Movelist moves;
		chess::movegen::legalmoves(moves, board);
		get_pawn_moves(board, moves);

		std::vector<int> move_indices;
		for (int i = 0; i < moves.size(); i++)
		{
			if (moves[i].score() == 0)
				continue;
			move_indices.push_back(i);
		}

		if (move_indices.size() > 0)
		{
			std::uniform_int_distribution<> dis(0, move_indices.size() - 1);
			int index = dis(gen);
			chosen_move = moves[move_indices[index]];
		}

		if (chosen_move == chess::Move::NULL_MOVE)
			break;

		board.makeMove(chosen_move);
	}
	std::cout << board << std::endl;

	return board;
}

void leonardo_overlord::reinforcement_learning_pawns()
{
	best_value_nnet = neural_network(); //reset the best nnet
	best_value_nnet.set_input_format(leonardo_util::get_pawn_input_format());
	best_value_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(32, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(16, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(leonardo_util::get_value_nnet_output_format(), identity_fn);
	best_value_nnet.xavier_initialization();

	const int games_per_learning = 1;

	for (int iteration = 0; ; iteration++)
	{
		std::vector<matrix> input_matrices;
		std::vector<matrix> output_matrices;

		for (int i = 0; i < games_per_learning; i++)
		{
			chess::Board random_board = random_pawn_board();
			chess::Movelist moves;
			chess::movegen::legalmoves(moves, random_board);
			bool pawn_moves_possible = get_pawn_moves(random_board, moves);

			if (!pawn_moves_possible)
				continue;

			play_pawn_game(
				random_board.getFen(),
				input_matrices,
				output_matrices);
		}

		if (input_matrices.size() > 0 && output_matrices.size() > 0)
		{

			data_space ds{
				leonardo_util::get_pawn_input_format(),
				leonardo_util::get_value_nnet_output_format(),
				input_matrices,
				output_matrices
			};

			test_result test_res = best_value_nnet.test_on_ds(ds);
			std::cout << "test_result: \n" << test_res.to_string();

			best_value_nnet.learn_on_ds(ds, 1, 1, 0.0001f, false);

			save_best_to_file(iteration, true, false);
		}
	}
}