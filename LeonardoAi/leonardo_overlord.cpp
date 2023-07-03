#include "leonardo_overlord.hpp"

void leonardo_overlord::save_best_to_file(size_t epoch)
{
	//best_network.save_to_file(name);
	std::cout << "\nsaving best network to file\n";
	std::string policy_path = "models\\" + name + "_policy_epoch_" + std::to_string(epoch) + ".parameters";
	std::string prediction_path = "models\\" + name + "_prediction_epoch_" + std::to_string(epoch) + ".parameters";
	best_policy_nnet.save_to_file(policy_path);
	best_prediction_nnet.save_to_file(prediction_path);
	std::cout << "\nsaved best network to file\n";
}
float leonardo_overlord::search(
	const ChessBoard& game,
	neural_network& given_policy_nnet,
	neural_network& given_prediction_nnet,
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


		//predict the position value by the prediction nnet and return the value (between -1 and 1) 
		given_prediction_nnet.forward_propagation(input_matrix);
		given_prediction_nnet.get_output().sync_device_and_host();
		return -1 * leonardo_util::get_prediction_output(given_prediction_nnet.get_output()); //cpu
	}

	std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();

	float max_utility = FLT_MIN;
	int best_move_idx = 0; // replace with random idx
	for (int i = 0; i < legal_moves.size(); i++)
	{
		const Move& move = *legal_moves[i].get();

		//if c is high - lots of exploration
		//if c is low - lots of exploitation
		float c = 1;

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

	float evaluation = search(new_game, given_policy_nnet, given_prediction_nnet, n, p, q, visited);

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
	neural_network& given_prediction_nnet,
	const ChessBoard& game
)
{
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> n;
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> p;
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> q;
	std::unordered_set<ChessBoard, chess_board_hasher> visited;

	//1600 in openai
	for (int i = 0; i < (3 + (epoch * 0.5)); i++)
	{
		search(game, given_policy_nnet, given_prediction_nnet, n, p, q, visited);
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
	data_space& prediction_training_ds)
{
	//we make a copy of the networks, because this makes it absolutely threadsafe
	neural_network policy_nnet_copy = neural_network(new_policy_nnet);
	neural_network prediction_nnet_copy = neural_network(new_prediction_nnet);

	for (int game_idx = first_game_idx; game_idx < last_game_idx; game_idx++)
	{
		ChessBoard game(STARTING_FEN);

		size_t move_idx = 0;
		size_t data_space_game_start_idx = game_idx * number_of_moves_per_game;

		while (true)
		{
			//std::cout << "(" + std::to_string(game_idx) + ")";

			matrix output_matrix(leonardo_util::get_policy_output_format());
			matrix input_matrix(leonardo_util::get_input_format());
			leonardo_util::set_matrix_from_chessboard(game, input_matrix); //all on cpu

			policy(epoch, output_matrix, policy_nnet_copy, prediction_nnet_copy, game);

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
				prediction_training_ds.set_data(input_matrix, ds_idx);
			}

			//make move
			std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();
			int chosen_move_idx = leonardo_util::get_random_best_move(output_matrix, legal_moves, game.getCurrentTurnColor());
			game.makeMove(*legal_moves[chosen_move_idx].get());

			if (game.getGameState() != GameState::Ongoing)
			{
				//get current index of data space
				size_t end_idx = data_space_game_start_idx + std::min(move_idx, number_of_moves_per_game - 1);

				//set the win matrix - 0 0 if draw - 1 0 for white winning and 0 1 for black winning
				matrix final_game_state_w(leonardo_util::get_prediction_output_format());
				leonardo_util::set_prediction_output(final_game_state_w, game, White);
				matrix final_game_state_b(leonardo_util::get_prediction_output_format());
				leonardo_util::set_prediction_output(final_game_state_b, game, Black);
				if (gpu_mode)
				{
					final_game_state_w.enable_gpu_mode();
					final_game_state_b.enable_gpu_mode();
				}

				bool white_turn = true;
				//the whole game was saved, but we do not know the outcome, so we add that now
				for (size_t prediction_idx = data_space_game_start_idx; prediction_idx <= end_idx; prediction_idx++)
				{
					//invalid argument throw in cuda here - TODO FIX
					prediction_training_ds.set_label(
						white_turn ? final_game_state_w : final_game_state_b,
						prediction_idx);
					white_turn = !white_turn;
				}

				/*
				std::cout
					<<
					"\ngame " + std::to_string(game_idx) +
					" finished (" + std::to_string(move_idx + 1) + ")" +
					"\n";
				*/
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
	data_space& prediction_training_ds)
{
	std::cout << "starting " << std::to_string(games_per_thread * thread_count) << " games\n";

	std::vector<std::thread> threads;

	size_t progression = 0;
	std::mutex progression_mutex;

	std::thread timer_thread = std::thread(
		&leonardo_util::update_thread,
		std::ref(progression),
		thread_count * games_per_thread,
		5000);

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
			std::ref(prediction_training_ds));

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
	size_t selfplay_thread_count = 16;
	size_t number_of_games_per_thread = 20;
	size_t number_of_selfplay_games = number_of_games_per_thread * selfplay_thread_count;
	size_t number_of_moves_per_game = 200;

	std::cout << "initalizing data space\n";
	data_space policy_training_ds(
		number_of_selfplay_games * number_of_moves_per_game,
		leonardo_util::get_input_format(),
		leonardo_util::get_policy_output_format()
	);
	std::cout << "policy training ds is " << byte_size_to_str(policy_training_ds.byte_size()) << "\n";

	data_space prediction_training_ds(
		number_of_selfplay_games * number_of_moves_per_game,
		leonardo_util::get_input_format(),
		leonardo_util::get_prediction_output_format()
	);
	std::cout << "prediction training ds is " << byte_size_to_str(prediction_training_ds.byte_size()) << "\n";

	if (gpu_mode)
	{
		policy_training_ds.copy_to_gpu();
		prediction_training_ds.copy_to_gpu();
	}

	std::cout
		<< "policy network is " << byte_size_to_str(new_policy_nnet.get_param_byte_size())
		<< " (" << selfplay_thread_count << " threads -> " << byte_size_to_str(new_policy_nnet.get_param_byte_size() * selfplay_thread_count) << ")"
		<< "\n";
	std::cout << "prediction network is " << byte_size_to_str(new_prediction_nnet.get_param_byte_size())
		<< " (" << selfplay_thread_count << " threads -> " << byte_size_to_str(new_prediction_nnet.get_param_byte_size() * selfplay_thread_count) << ")"
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
		prediction_training_ds);

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "selfplay done. took: " << ms_to_str(duration.count()) << std::endl;

	std::cout << "start training\n";
	start = std::chrono::high_resolution_clock::now();

	//train policy and prediction in parallel
	//if a game has less than number_of_moves_per_game moves, the rest of the data is not used - it will train on 0 data
	std::thread policy_thread = std::thread(
		&neural_network::learn_on_ds,
		&new_policy_nnet,
		std::ref(policy_training_ds),
		10,
		20,
		0.1f,
		true
	);
	std::thread prediction_thread = std::thread(
		&neural_network::learn_on_ds,
		&new_prediction_nnet,
		std::ref(prediction_training_ds),
		10,
		20,
		0.1f,
		true
	);

	if (policy_thread.joinable())
		policy_thread.join();
	if (prediction_thread.joinable())
		prediction_thread.join();

	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "training done. took: " << ms_to_str(duration.count()) << std::endl;
}

leonardo_overlord::leonardo_overlord(
	std::string name
) : name(name)
{
	//best_network = neural_network("models\\learner_epoch_500.parameters");

	best_policy_nnet.set_input_format(leonardo_util::get_input_format());
	best_policy_nnet.add_fully_connected_layer(1024, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(512, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(leonardo_util::get_policy_output_format(), leaky_relu_fn);
	best_policy_nnet.set_all_parameters(0.0f);
	best_policy_nnet.apply_noise(.1f);

	best_prediction_nnet.set_input_format(leonardo_util::get_input_format());
	best_prediction_nnet.add_fully_connected_layer(1024, leaky_relu_fn);
	best_prediction_nnet.add_fully_connected_layer(512, leaky_relu_fn);
	best_prediction_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	best_prediction_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	best_prediction_nnet.add_fully_connected_layer(leonardo_util::get_prediction_output_format(), leaky_relu_fn);
	best_prediction_nnet.set_all_parameters(0.0f);
	best_prediction_nnet.apply_noise(.1f);

	new_policy_nnet = neural_network(best_policy_nnet);
	new_prediction_nnet = neural_network(best_prediction_nnet);

	if (gpu_mode)
	{
		best_policy_nnet.enable_gpu_mode();
		best_prediction_nnet.enable_gpu_mode();
		new_policy_nnet.enable_gpu_mode();
		new_prediction_nnet.enable_gpu_mode();
	}
}

leonardo_overlord::~leonardo_overlord()
{
	if (file_save_thread.joinable())
	{
		file_save_thread.join();
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

		if (file_save_thread.joinable())
		{
			file_save_thread.join();
		}

		std::cout << "putting new and best nnet into an arena" << std::endl;
		auto arena_start = std::chrono::high_resolution_clock::now();

		arena_result arena_result = arena.play(200);

		auto arena_stop = std::chrono::high_resolution_clock::now();
		auto arena_duration =
			std::chrono::duration_cast<std::chrono::milliseconds>(arena_stop - arena_start);
		std::cout << "arena done. took: " << ms_to_str(arena_duration.count()) << std::endl;

		int win_score = arena_result.player_2_won - arena_result.player_1_won;

		if (win_score > 0)
		{
			std::cout
				<< "---------------------\n"
				<< "new network is better (won " << win_score << " more)\n"
				<< "---------------------\n";
			best_policy_nnet.set_parameters(new_policy_nnet);
			best_prediction_nnet.set_parameters(new_prediction_nnet);
		}
		else
		{
			std::cout
				<< "--------------------\n"
				<< "new network is worse (lost " -win_score << " more)\n"
				<< "--------------------\n";
		}

		if ((i + 1) % iterations_per_file_save == 0)
		{
			//start save_best_to_file in save_in_file_thread 
			file_save_thread = std::thread(&leonardo_overlord::save_best_to_file, this, i);
		}

		std::chrono::steady_clock::time_point stop =
			std::chrono::high_resolution_clock::now();

		long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		sum_elapsed_ms += elapsed_ms;
		std::cout << "cycle took: " << ms_to_str(elapsed_ms) << " ";
		std::cout << "average: " << ms_to_str(sum_elapsed_ms / (i + 1)) << "\n";
	}
}

const neural_network& leonardo_overlord::get_best_network() const
{
	return best_policy_nnet;
}