#include "leonardo_overlord.hpp"

void leonardo_overlord::save_best_to_file(size_t epoch, bool value_nnet, bool policy_nnet)
{
	//best_network.save_to_file(name);
	std::cout << "\nsaving best network to file";
	//check if folder exists
	if (!std::filesystem::exists("models"))
	{
		std::filesystem::create_directory("models");
	}
	std::string folder_name = "models\\" + name + "_epoch_" + std::to_string(epoch);
	if (!std::filesystem::exists(folder_name))
	{
		std::filesystem::create_directory(folder_name);
	}
	if (policy_nnet)
	{
		std::string policy_path = folder_name + "\\policy.parameters";
		best_policy_nnet.save_to_file(policy_path);
	}
	if (value_nnet)
	{
		std::string value_nnet_path = folder_name + "\\value.parameters";
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
		return -1 * leonardo_util::get_value_nnet_output(given_value_nnet.get_output()); //cpu
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
	for (int i = 0; i < (3 + (epoch * 0.5)); i++)
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

			if (game.getGameState() != GameState::Ongoing)
			{
				//get current index of data space
				size_t end_idx = data_space_game_start_idx + std::min(move_idx, number_of_moves_per_game - 1);

				//set the win matrix - 0 0 if draw - 1 0 for white winning and 0 1 for black winning
				matrix final_game_state_w(leonardo_util::get_value_nnet_output());
				leonardo_util::set_value_nnet_output(final_game_state_w, game, White);
				matrix final_game_state_b(leonardo_util::get_value_nnet_output());
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

	data_space value_nnet_training_ds(
		number_of_selfplay_games * number_of_moves_per_game,
		leonardo_util::get_input_format(),
		leonardo_util::get_value_nnet_output()
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

	//train policy and value nnet in parallel
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
	std::thread value_nnet_thread = std::thread(
		&neural_network::learn_on_ds,
		&new_value_nnet,
		std::ref(value_nnet_training_ds),
		10,
		20,
		0.1f,
		true
	);

	if (policy_thread.joinable())
		policy_thread.join();
	if (value_nnet_thread.joinable())
		value_nnet_thread.join();

	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "training done. took: " << ms_to_str(duration.count()) << std::endl;
}

leonardo_overlord::leonardo_overlord(
	std::string name
) : name(name)
{
	
	best_policy_nnet.set_input_format(leonardo_util::get_input_format());
	//best_policy_nnet.add_fully_connected_layer(1024, leaky_relu_fn);
	//best_policy_nnet.add_fully_connected_layer(512, leaky_relu_fn);
	//best_policy_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	//best_policy_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	best_policy_nnet.add_fully_connected_layer(leonardo_util::get_policy_output_format(), leaky_relu_fn);
	best_policy_nnet.xavier_initialization();
	
	best_value_nnet = neural_network("C:\\Users\\krame\\Desktop\\all\\_coding\\Leonardo\\LeonardoAi\\models\\better_player_700\\value.parameters");
	/*
	best_value_nnet.set_input_format(leonardo_util::get_input_format());
	//best_value_nnet.add_fully_connected_layer(1024, leaky_relu_fn);
	//best_value_nnet.add_fully_connected_layer(512, leaky_relu_fn);
	//best_value_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	//best_value_nnet.add_fully_connected_layer(256, leaky_relu_fn);
	best_value_nnet.add_fully_connected_layer(leonardo_util::get_value_nnet_output(), leaky_relu_fn);
	best_value_nnet.xavier_initialization();
	*/
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

leonardo_overlord::~leonardo_overlord()
{
	if (file_save_thread.joinable())
	{
		file_save_thread.join();
	}
}

void leonardo_overlord::train_policy()
{
	//train on grandmaster games
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
			best_value_nnet.set_parameters(new_value_nnet);
		}
		else
		{
			std::cout
				<< "--------------------\n"
				<< "new network is worse (lost " - win_score << " more)\n"
				<< "--------------------\n";
		}

		if ((i + 1) % iterations_per_file_save == 0)
		{
			//start save_best_to_file in save_in_file_thread 
			file_save_thread = std::thread(&leonardo_overlord::save_best_to_file, this, i, true, true);
		}

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
	size_t number_of_games = 20;

	AlphaBetaPruningBot player1(4); //the argument is the depth
	AlphaBetaPruningBot player2(2);
	bool player1_plays_white = true;

	std::cout << "thread " + std::to_string(id) + " started\n";

	for (int i = 0;; i++)
	{
		data_space ds(
			number_of_games * moves_per_game,
			leonardo_util::get_input_format(),
			leonardo_util::get_value_nnet_output()
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
				std::cout << legal_moves[chosen_move_idx].get()->getString() << std::endl;

				GameState game_state = game.getGameState();
				if (game_state != Ongoing)
				{
					size_t game_end_idx = start_idx + std::min(move_idx, moves_per_game - 1);

					matrix final_game_state_w(leonardo_util::get_value_nnet_output());
					leonardo_util::set_value_nnet_output(final_game_state_w, game, White);
					matrix final_game_state_b(leonardo_util::get_value_nnet_output());
					leonardo_util::set_value_nnet_output(final_game_state_b, game, Black);
					bool back_track_white_turn = true;
					for (size_t back_track_idx = start_idx; back_track_idx <= game_end_idx; back_track_idx++)
					{
						ds.set_label(
							back_track_white_turn ? final_game_state_w : final_game_state_b,
							back_track_idx);
						back_track_white_turn = !back_track_white_turn;
					}

					
					std::cout << "\nt-id: " << id << " game " << game_idx << " done move idx: " << move_idx << "\n" << GAME_STATE_STRING[game.getGameState()] << "\n";
					std::cout << "p1 is white " << player1_plays_white << "\n";
					std::cout << game.getFen() << "\n";
					
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

		if (epoch % 100 == 0)
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
	size_t thread_count = 1;

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
	RandomPlayer player2;
	bool player1_plays_white = true;

	for (int i = 0;; i++)
	{
		data_space ds(
			number_of_games * moves_per_game,
			leonardo_util::get_input_format(),
			leonardo_util::get_value_nnet_output()
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

					matrix final_game_state_w(leonardo_util::get_value_nnet_output());
					leonardo_util::set_value_nnet_output(final_game_state_w, game, White);
					matrix final_game_state_b(leonardo_util::get_value_nnet_output());
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