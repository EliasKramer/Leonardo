#include "leonardo_overlord.hpp"
#include <cmath>
void leonardo_overlord::save_best_to_file(size_t epoch, bool value_nnet, bool policy_nnet)
{
	std::cout << "\nsaving best network to file " << epoch << "\n";
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
	std::cout << "\nsaved best network to file\n";
}
float leonardo_overlord::search(
	const ChessBoard& game,
	neural_network& given_policy_nnet,
	neural_network& given_value_nnet,
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& n,
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& p,
	std::unordered_map<ChessBoard, matrix, chess_board_hasher>& q,
	std::unordered_set<ChessBoard, chess_board_hasher>& visited)
{
	if (game.getGameState() != GameState::Ongoing)
	{
		return game.getGameState() == GameState::WhiteWon ? 1.0f : -1.0f;
	}

	//current game state has not been visited
	if (visited.find(game) == visited.end())
	{
		visited.insert(game);

		//feed the input matrix into the policy network
		matrix input_matrix(leonardo_util::get_input_format());
		leonardo_util::set_matrix_from_chessboard(game, input_matrix); //cpu
		if (gpu_mode)
		{
			input_matrix.enable_gpu_mode();
		}
		given_policy_nnet.forward_propagation(input_matrix);

		//see how "promising" the current position is for every move
		given_policy_nnet.get_output().sync_device_and_host();
		p[game] = given_policy_nnet.get_output(); //cpu


		//feed the input matrix into the value network
		given_value_nnet.forward_propagation(input_matrix);
		given_value_nnet.get_output().sync_device_and_host();
		//the value network outputs a positive value if it thinks, that the current player is winning
		//it does not matter if the current player is white or black
		return -1 * leonardo_util::get_value_nnet_output_format(given_value_nnet.get_output()); //cpu
	}

	std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();

	float max_utility = FLT_MIN;
	int best_move_idx = 0; // replace with random idx
	for (int i = 0; i < legal_moves.size(); i++)
	{
		const Move& move = *legal_moves[i].get();

		//if c is high - lots of exploration
		//if c is low - lots of exploitation
		float c = 2;

		float n_sum = leonardo_util::matrix_map_sum(n, game, legal_moves);
		float n_at_move = leonardo_util::matrix_map_get_float(n, game, move);

		float utility =
			leonardo_util::matrix_map_get_float(q, game, move) +
			c * leonardo_util::matrix_map_get_float(p, game, move) *
			sqrt(n_sum) / (1 + n_at_move);

		if (utility > max_utility)
		{
			max_utility = utility;
			//best_move = move;
			best_move_idx = i;
		}
	}
	const Move& best_move = *legal_moves[best_move_idx].get();

	ChessBoard new_game = game.getCopyByValue();
	new_game.makeMove(best_move);

	float evaluation = search(new_game, given_policy_nnet, given_value_nnet, n, p, q, visited);

	//calculate the new average evaulation for the current move
	leonardo_util::matrix_map_set_float(q, game, best_move,
		(leonardo_util::matrix_map_get_float(n, game, best_move) * leonardo_util::matrix_map_get_float(q, game, best_move) + evaluation)
		/
		(leonardo_util::matrix_map_get_float(n, game, best_move) + 1)
	);

	//add 1 to the number of times the current move was explored
	leonardo_util::matrix_map_set_float(n, game, best_move,
		leonardo_util::matrix_map_get_float(n, game, best_move) + 1); //all on cpu

	return -evaluation;
}

void leonardo_overlord::policy(
	long long epoch,
	matrix& output_matrix,
	neural_network& given_policy_nnet,
	neural_network& given_value_nnet,
	const ChessBoard& game
)
{
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> n;
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> p;
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> q;
	std::unordered_set<ChessBoard, chess_board_hasher> visited;

	//1600 in openai
	for (int i = 0; i < 100; i++)
	{
		search(game, given_policy_nnet, given_value_nnet, n, p, q, visited);
	}

	std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();
	matrix& n_matrix = n[game];
	n_matrix.sync_device_and_host();
	for (int i = 0; i < legal_moves.size(); i++)
	{
		float value = leonardo_util::get_move_value(*legal_moves[i].get(), n_matrix, game.getCurrentTurnColor());
		leonardo_util::set_move_value(*legal_moves[i].get(), output_matrix, value, game.getCurrentTurnColor());
	}
}

void leonardo_overlord::self_play(
	long long epoch,
	size_t& progression,
	std::mutex& progression_mutex,
	int first_game_idx,
	int last_game_idx,
	size_t number_of_moves_per_game,
	data_space& policy_training_ds,
	data_space& value_nnet_training_ds)
{
	//we make a copy of the networks, because this makes it absolutely threadsafe
	neural_network policy_nnet_copy = neural_network(new_policy_nnet);
	neural_network value_nnet_copy = neural_network(new_value_nnet);

	for (int game_idx = first_game_idx; game_idx < last_game_idx; game_idx++)
	{
		ChessBoard game(STARTING_FEN);

		size_t move_idx = 0;
		size_t data_space_game_start_idx = game_idx * number_of_moves_per_game;
		std::string moves_str = "";
		while (true)
		{
			//std::cout << "(" + std::to_string(game_idx) + ")";

			matrix output_matrix(leonardo_util::get_policy_output_format());
			matrix input_matrix(leonardo_util::get_input_format());
			leonardo_util::set_matrix_from_chessboard(game, input_matrix); //all on cpu

			policy(epoch, output_matrix, policy_nnet_copy, value_nnet_copy, game);

			if (move_idx < number_of_moves_per_game)
			{
				if (gpu_mode)
				{
					input_matrix.enable_gpu_mode();
					output_matrix.enable_gpu_mode();
				}
				//TODO - promotion is not working rn
				size_t ds_idx = data_space_game_start_idx + move_idx;
				policy_training_ds.set_data(input_matrix, ds_idx);
				policy_training_ds.set_label(output_matrix, ds_idx);
				value_nnet_training_ds.set_data(input_matrix, ds_idx);
			}

			//make move
			std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();
			int chosen_move_idx = leonardo_util::get_random_best_move(output_matrix, legal_moves, game.getCurrentTurnColor());
			game.makeMove(*legal_moves[chosen_move_idx].get());
			moves_str += legal_moves[chosen_move_idx]->getString() + " ";

			if (game.getGameState() != GameState::Ongoing)
			{
				//get current index of data space
				size_t end_idx = data_space_game_start_idx + std::min(move_idx, number_of_moves_per_game - 1);

				//set the win matrix - 0 0 if draw - 1 0 for white winning and 0 1 for black winning
				matrix final_game_state_w(leonardo_util::get_value_nnet_output_format());
				leonardo_util::set_value_nnet_output(final_game_state_w, game, White);
				matrix final_game_state_b(leonardo_util::get_value_nnet_output_format());
				leonardo_util::set_value_nnet_output(final_game_state_b, game, Black);
				if (gpu_mode)
				{
					final_game_state_w.enable_gpu_mode();
					final_game_state_b.enable_gpu_mode();
				}

				bool white_turn = true;
				//the whole game was saved, but we do not know the outcome, so we add that now
				for (size_t back_track_idx = data_space_game_start_idx; back_track_idx <= end_idx; back_track_idx++)
				{
					//invalid argument throw in cuda here - TODO FIX
					value_nnet_training_ds.set_label(
						white_turn ? final_game_state_w : final_game_state_b,
						back_track_idx);
					white_turn = !white_turn;
				}


				std::cout
					<<
					"\ngame " + std::to_string(game_idx) +
					" finished (" + std::to_string(move_idx + 1) + ") " +
					(game.getGameState() == GameState::Draw ? "draw" : game.getGameState() == GameState::WhiteWon ? "white won" : "black won") +
					" " + moves_str +
					"\n";

				std::lock_guard<std::mutex> lock(progression_mutex);
				progression++;

				break;
			}

			move_idx++;
		}
	}
}

void leonardo_overlord::get_training_data(
	long long iteration,
	size_t thread_count,
	size_t games_per_thread,
	size_t number_of_moves_per_game,
	data_space& policy_training_ds,
	data_space& value_nnet_training_ds)
{
	std::cout << "starting " << std::to_string(games_per_thread * thread_count) << " games\n";

	std::vector<std::thread> threads;

	size_t progression = 0;
	std::mutex progression_mutex;

	std::thread timer_thread = std::thread(
		&leonardo_util::update_thread,
		std::ref(progression),
		thread_count * games_per_thread,
		30000);

	for (int i = 0; i < thread_count; i++)
	{
		std::thread t(
			&leonardo_overlord::self_play, this,
			iteration,
			std::ref(progression),
			std::ref(progression_mutex),
			i * games_per_thread,
			i * games_per_thread + games_per_thread,
			number_of_moves_per_game,
			std::ref(policy_training_ds),
			std::ref(value_nnet_training_ds));

		threads.push_back(std::move(t));
	}

	for (auto& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}

	if (timer_thread.joinable())
	{
		timer_thread.join();
	}
}

void leonardo_overlord::upgrade(
	long long epoch
)
{
	//az has 25000 games
	size_t selfplay_thread_count = 8;
	size_t number_of_games_per_thread = 10;
	size_t number_of_selfplay_games = number_of_games_per_thread * selfplay_thread_count;
	size_t number_of_moves_per_game = 200;

	std::cout << "initalizing data space\n";
	data_space policy_training_ds(
		number_of_selfplay_games * number_of_moves_per_game,
		leonardo_util::get_input_format(),
		leonardo_util::get_policy_output_format()
	);
	std::cout << "policy training ds is " << byte_size_to_str(policy_training_ds.byte_size()) << "\n";

	data_space value_nnet_training_ds(
		number_of_selfplay_games * number_of_moves_per_game,
		leonardo_util::get_input_format(),
		leonardo_util::get_value_nnet_output_format()
	);
	std::cout << "value training ds is " << byte_size_to_str(value_nnet_training_ds.byte_size()) << "\n";

	if (gpu_mode)
	{
		policy_training_ds.copy_to_gpu();
		value_nnet_training_ds.copy_to_gpu();
	}

	std::cout
		<< "policy network is " << byte_size_to_str(new_policy_nnet.get_param_byte_size())
		<< " (" << selfplay_thread_count << " threads -> " << byte_size_to_str(new_policy_nnet.get_param_byte_size() * selfplay_thread_count) << ")"
		<< "\n";
	std::cout << "value network is " << byte_size_to_str(new_value_nnet.get_param_byte_size())
		<< " (" << selfplay_thread_count << " threads -> " << byte_size_to_str(new_value_nnet.get_param_byte_size() * selfplay_thread_count) << ")"
		<< "\n";

	std::cout << "starting selfplay to get data \n";
	auto start = std::chrono::high_resolution_clock::now();
	//get training data through selfplay
	get_training_data(
		epoch,
		selfplay_thread_count,
		number_of_games_per_thread,
		number_of_moves_per_game,
		policy_training_ds,
		value_nnet_training_ds);

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "selfplay done. took: " << ms_to_str(duration.count()) << std::endl;

	std::cout << "start training\n";
	start = std::chrono::high_resolution_clock::now();

	std::string s1 = policy_training_ds.to_string();
	std::string s2 = value_nnet_training_ds.to_string();

	new_policy_nnet.learn_on_ds(
		policy_training_ds,
		32,
		64,
		0.1f,
		true
	);

	new_value_nnet.learn_on_ds(
		value_nnet_training_ds,
		32,
		64,
		0.1f,
		true
	);

	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "training done. took: " << ms_to_str(duration.count()) << std::endl;
}

leonardo_overlord::leonardo_overlord(
	std::string name
) : name(name)
{
	//print curret directory
	std::filesystem::path p = std::filesystem::current_path();
	std::cout << "looking for nnets in " << p << '\n';

	//best_value_nnet = neural_network("value.parameters");
	//best_policy_nnet = neural_network("policy.parameters");

	//best_value_nnet = neural_network("value.parameters");

	best_value_nnet.set_input_format(leonardo_util::get_input_format());
	best_value_nnet.add_fully_connected_layer(128, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(leonardo_util::get_value_nnet_output_format(), identity_fn);
	best_value_nnet.xavier_initialization();

	//best_value_nnet = neural_network("C:\\Users\\Elias\\Desktop\\all\\coding\\c_c++\\Leonardo\\x64\\Release\\models\\firestormV2_epoch_6600\\value.parameters");

	best_policy_nnet.set_input_format(leonardo_util::get_input_format());
	/*
	best_policy_nnet.add_fully_connected_layer(2048, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(2048, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(1024, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(1024, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(512, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(512, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(128, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(128, leaky_relu_fn);
	*/
	//best_policy_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	//best_policy_nnet.add_fully_connected_layer(64, leaky_relu_fn);
	/*
	*/
	best_policy_nnet.add_fully_connected_layer(leonardo_util::get_policy_output_format(), leaky_relu_fn);
	best_policy_nnet.xavier_initialization();

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

static void write_file_lines(const std::string& filename, std::vector<std::string> lines)
{
	std::ofstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
		return;
	}
	for (std::string line : lines) {
		file << line << std::endl;
	}
	file.close();
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

static void train_new_on_move(neural_network& policy_nnet, const ChessBoard& board, const std::unique_ptr<Move>& move)
{
	matrix input(leonardo_util::get_input_format());
	leonardo_util::set_matrix_from_chessboard(board, input);
	input.enable_gpu_mode();
	matrix label(leonardo_util::get_policy_output_format());
	leonardo_util::set_move_value(*move.get(), label, 1.0f, board.getCurrentTurnColor());
	label.enable_gpu_mode();
	policy_nnet.back_propagation(input, label);
}

static float get_curr_reward(const ChessBoard& board, const std::unique_ptr<Move>& move, bool last_move)
{
	ChessColor color = board.getCurrentTurnColor();

	if (last_move)
	{
		return 0; //is set by the data afterwards
		/*
		ChessBoard copy_board = board.getCopyByValue();
		copy_board.makeMove(*move.get());
		GameState state = copy_board.getGameState();
		if ((state == WhiteWon && color == White) || (state == BlackWon && color == Black))
		{
			//won
			/*
				8*1
				2*3,3
				2*3,2
				1*9
				2*5

			return 50;
		}
		else if ((state == WhiteWon && color == Black) || (state == BlackWon && color == White))
		{
			//lost
			return -50;
		}
		else
		{
			return 0;
		}*/
	}

	BoardRepresentation rep = board.getBoardRepresentation();
	BitBoard enemy_color = color == White ? rep.PiecesOfColor[Black] : rep.PiecesOfColor[White];
	Square destination_square = move.get()->getDestination();
	if (bitboardsOverlap(BB_SQUARE[destination_square], enemy_color))
	{
		ChessPiece captured_piece = rep.getPieceAt(destination_square);
		float value = (float)PIECETYPE_VALUE[captured_piece.getType()] / 100.0f;
		return value;
	}
	else
	{
		//move without any progress
		return -0.1f;
	}
}

void leonardo_overlord::train_on_gm_games()
{
	std::cout << "reading games file\n";
	std::vector<std::string> games = read_file_lines("games.txt");
	std::cout << "done. " << games.size() << " games loaded\n";
	matrix input(leonardo_util::get_input_format());
	matrix label(leonardo_util::get_value_nnet_output_format());

	int batch_size = 100;
	int ds_idx = 0;
	std::unique_ptr<data_space> ds;
	int start_idx = 16900;

	int depth = 4;

	for (int g = start_idx; g < games.size(); g++)
	{
		if (g % batch_size == 0)
		{
			if (g != start_idx)
			{
				std::cout << get_current_time_str() << "\n";
				if (gpu_mode)
					ds->copy_to_gpu();

				new_value_nnet.sync_device_and_host();
				best_value_nnet.sync_device_and_host();

				std::cout << "testing\n";
				test_result test_res = new_value_nnet.test_on_ds(*ds.get());
				std::cout << test_res.to_string() << "\n";

				auto start = std::chrono::high_resolution_clock::now();
				std::cout << "learning on " << ds->get_item_count() << " positions \n";
				new_value_nnet.learn_on_ds(*ds.get(), 1, 100, 0.0001, true);
				auto end = std::chrono::high_resolution_clock::now();
				std::cout << "done. took " << ms_to_str(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) << "\n";

				new_value_nnet.sync_device_and_host();
				best_value_nnet.sync_device_and_host();

				best_value_nnet.set_parameters(new_value_nnet);

				new_value_nnet.sync_device_and_host();
				best_value_nnet.sync_device_and_host();

				save_best_to_file(g, true, false);
			}

			ds_idx = 0;
			int batch_item_size = 0;
			for (int i = g; i < std::min((int)games.size(), g + batch_size); i++)
			{
				batch_item_size += split_string(games[i], ' ').size();
			}
			ds.reset();
			ds = std::make_unique<data_space>(
				batch_item_size,
				leonardo_util::get_input_format(),
				leonardo_util::get_value_nnet_output_format());

			std::cout << "start playing games. depth: " << depth << "\n";

		}
		std::vector<std::string> master_moves = split_string(games[g], ' ');
		if (master_moves.size() == 0)
		{
			std::cout << "mater moves are empty\n";
			continue;
		}
		std::vector<float> rewards;
		std::vector<float> ds_indices;
		ChessBoard board(STARTING_FEN);
		bool white_won = true;
		bool found_move = false;
		for (int m = 0; m < (int)master_moves.size(); m++)
		{
			if (m == 0)
			{
				if (master_moves[0] == "1")
				{
					white_won = true;
				}
				else if (master_moves[0] == "0")
				{
					white_won = false;
				}
				else
				{
					std::cout << "no outcome found\n";
				}
				continue;
			}

			std::vector<std::unique_ptr<Move>> moves = board.getAllLegalMoves();
			found_move = false;
			for (int i = 0; i < moves.size(); i++)
			{
				if (moves[i]->getString() == master_moves[m])
				{
					//add to ds
					leonardo_util::set_matrix_from_chessboard(board, input);
					//leonardo_util::set_move_value(*moves[i].get(), label, 1.0f, board.getCurrentTurnColor());
					//smart_assert(label.contains_non_zero_items());
					ds->set_data(input, ds_idx);
					//ds->set_label(label, ds_idx);
					//leonardo_util::set_move_value(*moves[i].get(), label, 0.0f, board.getCurrentTurnColor());
					//ds_indices.push_back(ds_idx);
					//rewards.push_back(get_curr_reward(board, moves[i], m == master_moves.size() - 1));
					/*
					if (rewards.size() != 1)
					{
						float curr_reward = rewards[rewards.size() - 1];
						rewards[rewards.size() - 2] -= curr_reward;
					}
					*/
					float stockfish_eval = stockfish_interface::eval(board.getFen(), depth);

					//std::cout << " - " << stockfish_eval << " - " << board.getFen() << " - " << moves[i].get()->getString()<<"\n";
					label.set_at_flat_host(0, stockfish_eval);
					ds->set_label(label, ds_idx);

					board.makeMove(*moves[i].get());
					found_move = true;
					ds_idx++;
					break;
				}
			}
			if (!found_move)
			{
				std::cout << "no move found: " + std::to_string(g) + " " + master_moves[m] + "\n";
				break;
			}
		}
		std::cout << ".";
		if ((g + 1) % 10 == 0)
		{
			std::cout << "\n";
		}
	}
}

static void create_dataset_thread(
	int depth,
	std::vector<std::string>& games,
	std::vector<std::string>& dataset,
	std::mutex& dataset_mutex,
	std::mutex& sf_mutex,
	int idx_from, //inclusive
	int idx_to, //exclusive
	int& progress_counter
)
{
	for (int g = idx_from; g < idx_to; g++)
	{
		std::vector<std::string> master_moves = split_string(games[g], ' ');
		if (master_moves.size() == 0)
		{
			std::cout << "mater moves are empty\n";
			continue;
		}
		ChessBoard board(STARTING_FEN);

		bool found_move = false;

		std::string move_str = "";
		std::string values_str = "";
		for (int m = 0; m < (int)master_moves.size(); m++)
		{
			if (m == 0)
			{
				continue;
			}

			std::vector<std::unique_ptr<Move>> moves = board.getAllLegalMoves();
			found_move = false;
			for (int i = 0; i < moves.size(); i++)
			{
				if (moves[i]->getString() == master_moves[m])
				{
					std::lock_guard<std::mutex> lock(sf_mutex);
					float stockfish_eval = stockfish_interface::eval(board.getFen(), depth);


					move_str += moves[i].get()->getString() + " ";
					values_str += std::to_string(stockfish_eval) + " ";

					board.makeMove(*moves[i].get());

					found_move = true;
					break;
				}
			}
			if (!found_move)
			{
				std::cout << "no move found: " + std::to_string(g) + " " + master_moves[m] + "\n";
				break;
			}
		}
		std::lock_guard<std::mutex> lock(dataset_mutex);
		std::string g_str = std::to_string(g);
		//std::cout << move_str << "\n";
		dataset.push_back("" + g_str + ";" + move_str);
		dataset.push_back("" + g_str + ";" + values_str);
		progress_counter++;
	}
}

void leonardo_overlord::create_dataset()
{
	std::vector<std::string> games = read_file_lines("games.txt");
	std::vector<std::string> dataset = read_file_lines("dataset.txt");

	std::vector<std::string> config = split_string(dataset[0], ' ');
	int depth = std::stoi(config[1]);
	int start_point = std::stoi(config[3]);
	int stop_point = std::stoi(config[5]);
	int threads = std::stoi(config[7]);

	std::cout << "depth: " << depth << "\n";
	std::cout << "start_point: " << start_point << "\n";
	std::cout << "stop_point: " << stop_point << "\n";
	std::cout << "threads: " << threads << "\n";


	if (depth < 4 || depth > 50)
	{
		std::cout << "depth should be between 4 and 50\n";
		return;
	}
	if (start_point < 0)
	{
		std::cout << "start_idx should be higher than 0\n";
		return;
	}
	if (threads < 1 || threads > 100)
	{
		std::cout << "threads should be between 1 and 100\n";
		return;
	}
	if (start_point >= stop_point)
	{
		std::cout << "start_idx should be lower than stop_idx\n";
		return;
	}
	if (stop_point > games.size())
	{
		std::cout << "stop_idx should be lower than the number of games (which is " << games.size() << ")\n";
		return;
	}


	int master_move_sum = 0;
	int played_games_sum = 0;

	int progress_counter = 0;
	std::mutex dataset_mutex;
	std::mutex sf_mutex;

	int total_games = stop_point - start_point;
	int games_per_thread = total_games / threads;

	std::vector<std::thread> thread_pool;

	for (int i = 0; i < threads; i++)
	{
		int start_idx = start_point + i * games_per_thread;
		int stop_idx = start_point + (i + 1) * games_per_thread;
		if (i == threads - 1)
		{
			stop_idx = stop_point;
		}

		std::thread t(
			create_dataset_thread,
			depth,
			std::ref(games),
			std::ref(dataset),
			std::ref(dataset_mutex),
			std::ref(sf_mutex),
			start_idx,
			stop_idx,
			std::ref(progress_counter));

		thread_pool.push_back(std::move(t));
	}

	std::cout << threads << " threads started. each working on " << games_per_thread << " games\n";

	auto start = std::chrono::high_resolution_clock::now();

	while (progress_counter < total_games)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		auto end = std::chrono::high_resolution_clock::now();
		long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		int remaining_ms = (total_games * elapsed_ms) / (std::max(1, progress_counter));

		std::cout << "progress: " << progress_counter << "/" << total_games <<
			" elapsed: " << ms_to_str(elapsed_ms) <<
			" remaining: " << ms_to_str(remaining_ms) <<
			"\n";

		std::lock_guard<std::mutex> lock(dataset_mutex);
		write_file_lines("dataset.txt", dataset);
	}
}

static void play_games(
	int game_idx,
	std::unique_ptr<data_space>& ds,
	int& ds_idx,
	std::vector<std::string>& master_moves,
	std::vector<float>& sf_eval)
{
	if (master_moves.size() != sf_eval.size())
	{
		std::cout << game_idx << " master_moves.size() " << master_moves.size() << " != " << sf_eval.size() << "sfeval.size()\n";
		return;
	}

	matrix input(leonardo_util::get_input_format());
	matrix label(leonardo_util::get_value_nnet_output_format());

	ChessBoard board(STARTING_FEN);
	bool found_move = false;
	for (int m = 0; m < (int)master_moves.size(); m++)
	{
		std::vector<std::unique_ptr<Move>> moves = board.getAllLegalMoves();
		found_move = false;
		for (int i = 0; i < moves.size(); i++)
		{
			if (moves[i]->getString() == master_moves[m])
			{
				leonardo_util::set_matrix_from_chessboard(board, input);
				ds->set_data(input, ds_idx);

				label.set_at_flat_host(0, sf_eval[m]);
				ds->set_label(label, ds_idx);

				board.makeMove(*moves[i].get());
				found_move = true;
				ds_idx++;
				break;
			}
		}
		if (!found_move)
		{
			std::cout << "no move found: " + std::to_string(game_idx) + " " + master_moves[m] + "\n";
			break;
		}
	}
}

void leonardo_overlord::train_on_dataset()
{
	std::vector<std::string> dataset = read_file_lines("dataset.txt");

	int games_per_training_step = 100;

	std::unique_ptr<data_space> ds;
	int ds_idx = 0;
	int start_idx = 0;
	int data_count = dataset.size() / 2;
	if (dataset.size() % 2 != 0)
	{
		std::cout << "dataset size is not even\n";
		return;
	}
	for (int i = start_idx; i < dataset.size(); i += 2)
	{
		/*std::cout << get_current_time_str() << "\n";
				if (gpu_mode)
					ds->copy_to_gpu();

				new_value_nnet.sync_device_and_host();
				best_value_nnet.sync_device_and_host();

				std::cout << "testing\n";
				test_result test_res = new_value_nnet.test_on_ds(*ds.get());
				std::cout << test_res.to_string() << "\n";

				auto start = std::chrono::high_resolution_clock::now();
				std::cout << "learning on " << ds->get_item_count() << " positions \n";
				new_value_nnet.learn_on_ds(*ds.get(), 1, 100, 0.0001, true);
				auto end = std::chrono::high_resolution_clock::now();
				std::cout << "done. took " << ms_to_str(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) << "\n";

				new_value_nnet.sync_device_and_host();
				best_value_nnet.sync_device_and_host();

				best_value_nnet.set_parameters(new_value_nnet);

				new_value_nnet.sync_device_and_host();
				best_value_nnet.sync_device_and_host();

				save_best_to_file(g, true, false);*/
		if ((i) % games_per_training_step == 0)
		{
			if (i != start_idx)
			{
				if(gpu_mode)
					ds->copy_to_gpu();

				new_value_nnet.sync_device_and_host();
				best_value_nnet.sync_device_and_host();

				std::cout << "testing\n";
				test_result test_res = new_value_nnet.test_on_ds(*ds.get());
				std::cout << test_res.to_string() << "\n";

				std::cout << "learning\n";
				auto start = std::chrono::high_resolution_clock::now();
				new_value_nnet.learn_on_ds(*ds, 1, 200, 0.0001, true);
				auto end = std::chrono::high_resolution_clock::now();
				long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
				std::cout << "learning took " << ms_to_str(elapsed_ms) << "\n";

				new_value_nnet.sync_device_and_host();
				best_value_nnet.sync_device_and_host();

				best_value_nnet.set_parameters(new_value_nnet);

				new_value_nnet.sync_device_and_host();
				best_value_nnet.sync_device_and_host();

				save_best_to_file(i, true, false);
			}

			ds_idx = 0;
			ds.reset();
			int move_sum = 0;
			for (int mv = i; mv < std::min((int)dataset.size() - 1, (i + games_per_training_step)); mv += 2)
			{//dont forget gpu
				move_sum += split_string(dataset[mv], ' ').size();
			}
			ds = std::make_unique<data_space>(
				move_sum,
				leonardo_util::get_input_format(),
				leonardo_util::get_value_nnet_output_format());
		}

		std::string first_string = dataset[i];
		std::string second_string = dataset[i + 1];
		std::vector<std::string> first_split = split_string(first_string, ';');
		std::vector<std::string> second_split = split_string(second_string, ';');
		if (first_split.size() != 2 || second_split.size() != 2)
		{
			std::cout << "first_split.size() != 2 || second_split.size() != 2\n";
			continue;
		}

		std::string dataset_number = first_split[0];
		std::string move_str = first_split[1];
		std::string values_str = second_split[1];
		std::string check_number = second_split[0];
		if (dataset_number != check_number)
		{
			std::cout << "dataset_number " + dataset_number + " != check_number " + check_number + "\n";
			continue;
		}
		std::vector<std::string> master_moves = split_string(move_str, ' ');
		std::vector<std::string> values = split_string(values_str, ' ');
		std::vector<float> master_values;

		for (int i = 0; i < values.size(); i++)
		{
			master_values.push_back(std::stof(values[i]));
		}

		play_games(i, ds, ds_idx, master_moves, master_values);
	}
}

void leonardo_overlord::train()
{
	chess_arena arena(
		"pit",
		std::make_unique<leonardo_bot>(best_policy_nnet, distributed_random),
		std::make_unique<leonardo_bot>(new_policy_nnet, distributed_random)
	);

	int iterations_per_file_save = 1;
	long long sum_elapsed_ms = 0;
	for (long long i = 0; ; i++)
	{
		std::chrono::steady_clock::time_point start =
			std::chrono::high_resolution_clock::now();

		std::cout
			<< "upgrade time. iteration: " << i
			<< "\n--------------------------------------\n";
		upgrade(i);

		//	# arena - with biased random instead of applying noise to input
		//	win_ratio = pit(policy_nn, new_policy_nn, n = number_of_games)

		std::cout << "putting new and best nnet into an arena" << std::endl;
		auto arena_start = std::chrono::high_resolution_clock::now();

		arena_result arena_result = arena.play(5);

		auto arena_stop = std::chrono::high_resolution_clock::now();
		auto arena_duration =
			std::chrono::duration_cast<std::chrono::milliseconds>(arena_stop - arena_start);
		std::cout << "arena done. took: " << ms_to_str(arena_duration.count()) << std::endl;

		int win_score = arena_result.player_2_won - arena_result.player_1_won;

		best_policy_nnet.sync_device_and_host();
		best_value_nnet.sync_device_and_host();
		new_value_nnet.sync_device_and_host();
		new_policy_nnet.sync_device_and_host();

		std::cout << "value nets are equal format: " << best_value_nnet.nn_equal_format(new_value_nnet) << std::endl;
		std::cout << "policy nets are equal format: " << best_policy_nnet.nn_equal_format(new_policy_nnet) << std::endl;
		std::cout << "value nets are equal parameters: " << best_value_nnet.equal_parameter(new_value_nnet) << std::endl;
		std::cout << "policy nets are equal parameters: " << best_policy_nnet.equal_parameter(new_policy_nnet) << std::endl;

		if (win_score > 0)
		{
			std::cout
				<< "---------------------\n"
				<< "new network is better (won " << win_score << " more)\n"
				<< "---------------------\n";
			best_policy_nnet.set_parameters(new_policy_nnet);
			best_value_nnet.set_parameters(new_value_nnet);

			if ((i + 1) % iterations_per_file_save == 0)
			{
				//start save_best_to_file in save_in_file_thread 
				save_best_to_file(i, true, true);
			}
		}
		else
		{
			std::cout
				<< "--------------------\n"
				<< "new network is worse (lost " << (-win_score) << " more)\n"
				<< "--------------------\n";
		}

		std::cout << "value nets are equal format: " << best_value_nnet.nn_equal_format(new_value_nnet) << std::endl;
		std::cout << "policy nets are equal format: " << best_policy_nnet.nn_equal_format(new_policy_nnet) << std::endl;
		std::cout << "value nets are equal parameters: " << best_value_nnet.equal_parameter(new_value_nnet) << std::endl;
		std::cout << "policy nets are equal parameters: " << best_policy_nnet.equal_parameter(new_policy_nnet) << std::endl;

		std::chrono::steady_clock::time_point stop =
			std::chrono::high_resolution_clock::now();

		long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		sum_elapsed_ms += elapsed_ms;
		std::cout << "cycle took: " << ms_to_str(elapsed_ms) << " ";
		std::cout << "average: " << ms_to_str(sum_elapsed_ms / (i + 1)) << "\n";
	}
}

void leonardo_overlord::train_value_nnet_thread_fn(
	size_t id,
	std::chrono::high_resolution_clock::time_point training_start,
	size_t& epoch,
	size_t& total_moves_made,
	size_t& total_games_played,
	size_t& total_black_won,
	size_t& total_white_won,
	size_t& total_draw,
	std::mutex& trainings_mutex
)
{
	size_t moves_per_game = 50;
	size_t number_of_games = 15;

	AlphaBetaPruningBot player1(4); //the argument is the depth
	AlphaBetaPruningBot player2(2);
	bool player1_plays_white = true;

	std::cout << "thread " + std::to_string(id) + " started\n";

	for (int i = 0;; i++)
	{
		data_space ds(
			number_of_games * moves_per_game,
			leonardo_util::get_input_format(),
			leonardo_util::get_value_nnet_output_format()
		);

		size_t move_sum = 0;
		size_t black_won_sum = 0;
		size_t white_won_sum = 0;
		size_t draw_sum = 0;

		for (size_t game_idx = 0; game_idx < number_of_games; game_idx++)
		{
			bool whites_turn = true;

			size_t start_idx = game_idx * moves_per_game;
			size_t end_idx = (game_idx + 1) * moves_per_game;
			ChessBoard game(STARTING_FEN);
			for (size_t move_idx = 0; ; move_idx++)
			{
				std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();

				size_t chosen_move_idx =
					whites_turn == player1_plays_white ?
					player1.getMove(game, legal_moves) :
					player2.getMove(game, legal_moves);
				whites_turn = !whites_turn;

				size_t ds_idx = start_idx + move_idx;

				if (ds_idx < end_idx)
				{
					matrix input_matrix(leonardo_util::get_input_format());
					leonardo_util::set_matrix_from_chessboard(game, input_matrix); //all on cpu

					ds.set_data(input_matrix, ds_idx);
				}

				game.makeMove(*legal_moves[chosen_move_idx].get());
				//std::cout << legal_moves[chosen_move_idx].get()->getString() << std::endl;

				GameState game_state = game.getGameState();
				if (game_state != Ongoing)
				{
					size_t game_end_idx = start_idx + std::min(move_idx, moves_per_game - 1);

					matrix final_game_state_w(leonardo_util::get_value_nnet_output_format());
					leonardo_util::set_value_nnet_output(final_game_state_w, game, White);
					matrix final_game_state_b(leonardo_util::get_value_nnet_output_format());
					leonardo_util::set_value_nnet_output(final_game_state_b, game, Black);
					bool back_track_white_turn = true;
					for (size_t back_track_idx = start_idx; back_track_idx <= game_end_idx; back_track_idx++)
					{
						ds.set_label(
							back_track_white_turn ? final_game_state_w : final_game_state_b,
							back_track_idx);
						back_track_white_turn = !back_track_white_turn;
					}

					/*
					std::cout << "\nt-id: " << id << " game " << game_idx << " done move idx: " << move_idx << "\n" << GAME_STATE_STRING[game.getGameState()] << "\n";
					std::cout << "p1 is white " << player1_plays_white << "\n";
					std::cout << game.getFen() << "\n";
					*/
					game_state == WhiteWon ? white_won_sum++ : game_state == BlackWon ? black_won_sum++ : draw_sum++;

					player1_plays_white = !player1_plays_white;
					move_sum += (move_idx + 1);
					break;
				}
			}
		}
		if (gpu_mode)
		{
			ds.copy_to_gpu();
		}

		auto start = std::chrono::high_resolution_clock::now();
		std::lock_guard<std::mutex> lock(trainings_mutex);
		auto stop = std::chrono::high_resolution_clock::now();
		long long mutex_waiting_time =
			std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

		best_value_nnet.learn_on_ds(
			ds,
			32,
			128,
			0.1f,
			true);

		total_games_played += number_of_games;
		total_moves_made += move_sum;
		total_black_won += black_won_sum;
		total_white_won += white_won_sum;
		total_draw += draw_sum;

		//update moves per game, that get saved to a bit more than the average
		moves_per_game = (size_t)((float)(total_moves_made / total_games_played) * 1.1f);
		smart_assert(moves_per_game > 0);

		if (epoch % 1 == 0)
		{
			save_best_to_file(epoch, true, false);
		}

		long long total_time_elapsed =
			std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::high_resolution_clock::now() - training_start).count();

		std::cout <<
			"# " << (epoch + 1) << " " <<
			get_current_time_str() <<
			" | " << ms_to_str(total_time_elapsed) <<
			" (total moves " << total_moves_made <<
			" | total games " << total_games_played <<
			" | m/g " << total_moves_made / total_games_played <<
			" | moves saved/g " << moves_per_game <<
			" | black won " << total_black_won <<
			" | white won " << total_white_won <<
			" | draw " << total_draw <<
			" | mutex wait " << ms_to_str(mutex_waiting_time) <<
			")\n";
		epoch++;
	}
}

void leonardo_overlord::train_value_nnet()
{
	size_t thread_count = 8;

	std::mutex trainings_mutex;

	std::vector<std::thread> threads;

	size_t epoch = 0;
	size_t total_moves_made = 0;
	size_t total_games_played = 0;
	size_t black_won = 0;
	size_t white_won = 0;
	size_t draw = 0;

	std::chrono::time_point start_time = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < thread_count; i++)
	{
		threads.emplace_back(
			&leonardo_overlord::train_value_nnet_thread_fn,
			this,
			i,
			start_time,
			std::ref(epoch),
			std::ref(total_moves_made),
			std::ref(total_games_played),
			std::ref(black_won),
			std::ref(white_won),
			std::ref(draw),
			std::ref(trainings_mutex)
		);
	}

	//willl never happen. just to block the thread
	for (auto& thread : threads)
	{
		thread.join();
	}
}

void leonardo_overlord::test_value_nnet()
{
	size_t moves_per_game = 50;
	size_t number_of_games = 10;

	AlphaBetaPruningBot player1(3); //the argument is the depth
	AlphaBetaPruningBot player2(3);
	bool player1_plays_white = true;

	for (int i = 0;; i++)
	{
		data_space ds(
			number_of_games * moves_per_game,
			leonardo_util::get_input_format(),
			leonardo_util::get_value_nnet_output_format()
		);

		size_t move_sum = 0;

		for (size_t game_idx = 0; game_idx < number_of_games; game_idx++)
		{
			bool whites_turn = true;

			size_t start_idx = game_idx * moves_per_game;
			size_t end_idx = (game_idx + 1) * moves_per_game;
			ChessBoard game(STARTING_FEN);
			for (size_t move_idx = 0; ; move_idx++)
			{
				std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();

				size_t chosen_move_idx =
					whites_turn == player1_plays_white ?
					player1.getMove(game, legal_moves) :
					player2.getMove(game, legal_moves);
				whites_turn = !whites_turn;

				size_t ds_idx = start_idx + move_idx;

				if (ds_idx < end_idx)
				{
					matrix input_matrix(leonardo_util::get_input_format());
					leonardo_util::set_matrix_from_chessboard(game, input_matrix); //all on cpu

					ds.set_data(input_matrix, ds_idx);
				}

				game.makeMove(*legal_moves[chosen_move_idx].get());

				if (game.getGameState() != Ongoing)
				{
					size_t game_end_idx = start_idx + std::min(move_idx, moves_per_game - 1);

					matrix final_game_state_w(leonardo_util::get_value_nnet_output_format());
					leonardo_util::set_value_nnet_output(final_game_state_w, game, White);
					matrix final_game_state_b(leonardo_util::get_value_nnet_output_format());
					leonardo_util::set_value_nnet_output(final_game_state_b, game, Black);
					bool back_track_white_turn = true;
					for (size_t back_track_idx = start_idx; back_track_idx <= game_end_idx; back_track_idx++)
					{
						ds.set_label(
							back_track_white_turn ? final_game_state_w : final_game_state_b,
							back_track_idx);
						back_track_white_turn = !back_track_white_turn;
					}


					std::cout << " game " << game_idx << " done move idx: " << move_idx << "\n" << GAME_STATE_STRING[game.getGameState()] << "\n";
					std::cout << "p1 is white " << player1_plays_white << "\n";
					std::cout << game.getFen() << "\n";

					player1_plays_white = !player1_plays_white;
					move_sum += (move_idx + 1);
					break;
				}
			}
		}
		if (gpu_mode)
		{
			ds.copy_to_gpu();
		}

		test_result result = best_value_nnet.test_on_ds(ds);
		std::cout << result.to_string();
	}
}

const neural_network& leonardo_overlord::get_best_network() const
{
	return best_policy_nnet;
}