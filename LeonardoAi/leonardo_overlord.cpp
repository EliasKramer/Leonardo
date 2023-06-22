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
		//now we visted the game
		visited.insert(game);

		//feed the input matrix into the policy network
		matrix input_matrix(leonardo_util::get_input_format());
		leonardo_util::set_matrix_from_chessboard(game, input_matrix); //cpu
		if (gpu_mode)
		{
			input_matrix.enable_gpu_mode();
		}
		new_policy_nnet.forward_propagation(input_matrix);

		//see how "promising" the current position is for every move
		new_policy_nnet.get_output().sync_device_and_host();
		p[game] = new_policy_nnet.get_output(); //cpu


		//predict the position value by the prediction nnet and return the value (between -1 and 1) 
		new_prediction_nnet.forward_propagation(input_matrix);
		new_prediction_nnet.get_output().sync_device_and_host();
		return -1 * leonardo_util::get_prediction_output(new_prediction_nnet.get_output()); //cpu
	}

	std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();

	float max_utility = FLT_MIN;
	int best_move_idx = 0; // replace with random idx
	for (int i = 0; i < legal_moves.size(); i++)
	{
		const Move& move = *legal_moves[i].get();
		std::string s = move.getString();

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


	std::string fen = game.getFen();
	if (fen == "r1bqk2r/ppp1pp2/n2p1npb/6Bp/P7/N2P1N2/1PP1PPPP/R2QKB1R b Qkq - 1 7")
	{

		std::vector<std::string> moves_s;
		int i = 0;
		for (auto& move : legal_moves)
		{
			moves_s.push_back(move->getString());
			if (move->getString() == "O-O-O")
			{
				//std::cout << SQUARE_STRING[(int)move->getStart()] << "|" << SQUARE_STRING[(int)move->getDestination()] << "\n";
				move->getString();
				//std::cout << "found move\n" << move->getString() << " idx " << i << "\n";

				for (auto& s : moves_s)
				{
					std::cout << s << " " << "\n";
				}
				auto lm = game.getAllLegalMoves();
				for (auto& s : lm)
				{
					std::cout << s.get()->getString() << " " << "\n";
				}

			}
			i++;
		}
	}


	ChessBoard new_game = game.getCopyByValue();
	//DEBUG
	std::string board_s = game.getString();
	//std::string move = best_move.getString();
	//DEBUG
	new_game.makeMove(best_move);

	float evaluation = search(new_game, n, p, q, visited);

	//calculate the new average evaulation for the current move
	leonardo_util::matrix_map_set_float(q, game, best_move,
		(leonardo_util::matrix_map_get_float(n, game, best_move) * leonardo_util::matrix_map_get_float(q, game, best_move) + evaluation) /
		(leonardo_util::matrix_map_get_float(n, game, best_move) + 1)
	);

	//add 1 to the number of times the current move was explored
	leonardo_util::matrix_map_set_float(n, game, best_move,
		leonardo_util::matrix_map_get_float(n, game, best_move) + 1); //all on cpu

	return -evaluation;
}

void leonardo_overlord::policy(matrix& output_matrix, const ChessBoard& game)
{
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> n;
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> p;
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> q;
	std::unordered_set<ChessBoard, chess_board_hasher> visited;

	//1600 in openai
	for (int i = 0; i < 5; i++)
	{
		search(game, n, p, q, visited);
	}

	std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();
	for (int i = 0; i < legal_moves.size(); i++)
	{
		int idx = leonardo_util::get_matrix_idx_for_move(*legal_moves[i].get());

		matrix& n_matrix = n[game];
		n_matrix.sync_device_and_host(); // just in case, but it should be on cpu
		float value = n_matrix.get_at_flat_host(idx);

		output_matrix.set_at_flat(idx, value);
	}
}

void leonardo_overlord::get_training_data(
	size_t number_of_selfplay_games,
	size_t number_of_moves_per_game,
	data_space& policy_training_ds,
	data_space& prediction_training_ds)
{

	policy_training_ds.iterator_reset();
	prediction_training_ds.iterator_reset();

	for (int i = 0; i < number_of_selfplay_games; i++)
	{
		ChessBoard game(STARTING_FEN); // replace with starting pos TODO

		size_t move_count = 0;
		while (true)
		{
			move_count++;


			//quick check if the iterators are valid
			if (prediction_training_ds.get_iterator_idx() != policy_training_ds.get_iterator_idx())
			{
				throw std::exception("something went wrong");
			}

			//chess_data_space.new_game()  # remember index, to add outcome later
			size_t start_idx = prediction_training_ds.get_iterator_idx();

			//# get move of policy network
			//output_matrix = policy(game, policy_network, prediction_network)
			matrix output_matrix(leonardo_util::get_policy_output_format());
			matrix input_matrix(leonardo_util::get_input_format());
			leonardo_util::set_matrix_from_chessboard(game, input_matrix); //all on cpu

			policy(output_matrix, game);

			if (move_count <= number_of_moves_per_game)
			{
				/*
				if (gpu_mode)
				{
					input_matrix.enable_gpu_mode();
					output_matrix.enable_gpu_mode();
				}
				//TODO - promotion is not working rn
				policy_training_ds.set_current_data(input_matrix);
				policy_training_ds.set_current_label(output_matrix);
				prediction_training_ds.set_current_data(input_matrix);
				*/
			}

			//make move
			std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();
			int move_idx = leonardo_util::get_random_best_move(output_matrix, legal_moves);
			std::string s = "game: " + std::to_string(i) + " move: " + std::to_string(move_count) + " " + legal_moves[move_idx].get()->getString();
			std::cout << s << "\n";
			game.makeMove(*legal_moves[move_idx].get());

			if (move_count > 40)
			{
				break;
			}

			if (game.getGameState() != GameState::Ongoing)
			{
				//get current index of data space
				size_t end_idx = prediction_training_ds.get_iterator_idx();

				//set the win matrix - 0 0 if draw - 1 0 for white winning and 0 1 for black winning
				matrix final_game_state(leonardo_util::get_prediction_output_format());
				leonardo_util::set_prediction_output(final_game_state, game);
				if (gpu_mode)
				{
					final_game_state.enable_gpu_mode();
				}

				//the whole game was saved, but we do not know the outcome, so we add that now
				for (size_t prediction_idx = start_idx; prediction_idx <= end_idx; prediction_idx++)
				{
					prediction_training_ds.set_iterator_idx(prediction_idx);
					prediction_training_ds.set_current_label(final_game_state);
				}

				policy_training_ds.iterator_next();
				prediction_training_ds.iterator_next();
				std::cout
					<< (i + 1) << "/" << number_of_selfplay_games
					<< " game ended. move count: " << move_count
					<< " game_state: " << GAME_STATE_STRING[game.getGameState()]
					<< std::endl;
				break;
			}

			policy_training_ds.iterator_next();
			prediction_training_ds.iterator_next();
		}
	}
}

void leonardo_overlord::upgrade()
{
	size_t number_of_selfplay_games = 1000;
	size_t number_of_moves_per_game = 10;

	data_space policy_training_ds(
		number_of_selfplay_games * number_of_moves_per_game,
		leonardo_util::get_input_format(),
		leonardo_util::get_policy_output_format()
	);
	policy_training_ds.copy_to_gpu();

	data_space prediction_training_ds(
		number_of_selfplay_games * number_of_moves_per_game,
		leonardo_util::get_input_format(),
		leonardo_util::get_prediction_output_format()
	);
	prediction_training_ds.copy_to_gpu();

	get_training_data(
		number_of_selfplay_games,
		number_of_moves_per_game,
		policy_training_ds,
		prediction_training_ds);

	policy_training_ds.iterator_reset();
	prediction_training_ds.iterator_reset();

	std::cout << "start training\n";

	//train policy and prediction in parallel
	//if a game has less than number_of_moves_per_game moves, the rest of the data is not used - it will train on 0 data
	std::thread policy_thread = std::thread(
		&neural_network::learn_on_ds,
		&new_policy_nnet,
		std::ref(policy_training_ds),
		2,
		20,
		0.1f
	);
	std::thread prediction_thread = std::thread(
		&neural_network::learn_on_ds,
		&new_prediction_nnet,
		std::ref(prediction_training_ds),
		2,
		20,
		0.1f
	);

	if (policy_thread.joinable())
		policy_thread.join();
	if (prediction_thread.joinable())
		prediction_thread.join();

	std::cout << "training done" << std::endl;
}

leonardo_overlord::leonardo_overlord(
	std::string name
) : name(name)
{
	//best_network = neural_network("models\\learner_epoch_500.parameters");

	best_policy_nnet.set_input_format(leonardo_util::get_input_format());
	best_policy_nnet.add_fully_connected_layer(512, sigmoid_fn);
	best_policy_nnet.add_fully_connected_layer(512, sigmoid_fn);
	best_policy_nnet.add_fully_connected_layer(leonardo_util::get_policy_output_format(), sigmoid_fn);
	best_policy_nnet.set_all_parameters(0.0f);
	best_policy_nnet.apply_noise(1);

	best_prediction_nnet.set_input_format(leonardo_util::get_input_format());
	best_prediction_nnet.add_fully_connected_layer(512, sigmoid_fn);
	best_prediction_nnet.add_fully_connected_layer(512, sigmoid_fn);
	best_prediction_nnet.add_fully_connected_layer(leonardo_util::get_prediction_output_format(), sigmoid_fn);
	best_prediction_nnet.set_all_parameters(0.0f);
	best_prediction_nnet.apply_noise(1);

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

	//start timer
	std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();

	//training 25000 apparently
	int iterations = 21;
	for (int i = 0; i < iterations; i++)
	{
		std::cout << "upgrade time. iteration: " << i << std::endl;
		upgrade();

		//	# arena - with biased random instead of applying noise to input
		//	win_ratio = pit(policy_nn, new_policy_nn, n = number_of_games)

		if (file_save_thread.joinable())
		{
			file_save_thread.join();
		}

		std::cout << "throwing new and best nnet into a arena" << std::endl;
		arena_result arena_result = arena.play(50);
		int win_score = arena_result.player_2_won - arena_result.player_1_won;

		//	# best input for next iteration
		//	threshold = 0.55  # hyperparameter - how much better the new net has to be
		//	if win_ratio > threshold:
		if (win_score > 0)
		{
			//policy_nn = new_policy_nn
			//syncing host and device gets done inside ?
			std::cout << "new network is better" << std::endl;
			best_policy_nnet.set_parameters(new_policy_nnet);
			best_prediction_nnet.set_parameters(new_prediction_nnet);
		}
		else
		{
			std::cout << "new network is worse" << std::endl;
			//new_policy_nnet.set_parameters(best_policy_nnet);
			//new_prediction_nnet.set_parameters(new_prediction_nnet);
		}

		if (i % 20 == 0)
		{
			//start save_best_to_file in save_in_file_thread 

			file_save_thread = std::thread(&leonardo_overlord::save_best_to_file, this, i);
		}

		std::chrono::steady_clock::time_point stop = std::chrono::high_resolution_clock::now();

		//elapsed seconds since start
		long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

		std::cout << "elapsed: " << ms_to_str(elapsed_ms) << std::endl;
		//remaining - TODO check if this is correct
		long long remaining_seconds = (elapsed_ms / (i + 1)) * (iterations - i - 1);
		std::cout << "remaining: " << ms_to_str(remaining_seconds) << std::endl;
	}
}

void leonardo_overlord::debug()
{
	ChessBoard board(STARTING_FEN);

	std::vector<std::string> inputs = {
		"d2d3",
		"h7h5",
		"c1g5",
		"g7g6",
		"g1f3",
		"b8a6",
		"h1g1",
		"g8f6",
		"a2a4",
		"f8h6",
		"b1a3",
		"d7d6",
		"g1h1"
	};

	for (int i = 0; i < inputs.size(); i++)
	{
		auto legal_moves = board.getAllLegalMoves();
		bool found_move = false;
		for (int j = 0; j < legal_moves.size(); j++)
		{
			if (legal_moves[j].get()->getString() == inputs[i])
			{
				std::cout << "move: " << inputs[i] << std::endl;
				board.makeMove(*legal_moves[j].get());
				found_move = true;
				break;
			}
		}
		if (!found_move)
		{
			std::cout << "move not found: " << inputs[i] << std::endl;
			break;
		}
	}

	for (int x = 0; x < 100; x++)
	{
		std::cout << "# " << x << "\n";
		new_policy_nnet.set_all_parameters(0);
		new_policy_nnet.apply_noise(1);
		new_prediction_nnet.set_all_parameters(0);
		new_prediction_nnet.apply_noise(1);

		std::unordered_map<ChessBoard, matrix, chess_board_hasher> n;
		std::unordered_map<ChessBoard, matrix, chess_board_hasher> p;
		std::unordered_map<ChessBoard, matrix, chess_board_hasher> q;
		std::unordered_set<ChessBoard, chess_board_hasher> visited;
		for (int i = 0; i < 50; i++)
		{
			search(board.getCopyByValue(), n, p, q, visited);
		}
	}
}

const neural_network& leonardo_overlord::get_best_network() const
{
	return best_policy_nnet;
}