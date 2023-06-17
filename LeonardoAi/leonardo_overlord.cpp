#include "leonardo_overlord.hpp"

void leonardo_overlord::save_best_to_file(size_t epoch)
{
	//best_network.save_to_file(name);
	std::string policy_path = "models\\" + name + "_policy_epoch_" + std::to_string(epoch) + ".parameters";
	std::string prediction_path = "models\\" + name + "_prediction_epoch_" + std::to_string(epoch) + ".parameters";

	//start thread and save once
	std::thread file_save_thread([this, policy_path, prediction_path]() {
		best_policy_nnet.save_to_file(policy_path);
		best_prediction_nnet.save_to_file(prediction_path); });
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
		leonardo_util::set_matrix_from_chessboard(game, input_matrix);
		new_policy_nnet.forward_propagation(input_matrix);

		//see how "promising" the current position is for every move
		p[game] = new_policy_nnet.get_output();

		//predict the position value by the prediction nnet and return the value (between -1 and 1) 
		new_prediction_nnet.forward_propagation(input_matrix);
		return -1 * leonardo_util::get_prediction_output(new_prediction_nnet.get_output());
	}

	std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();

	float max_utility = FLT_MIN;
	Move& best_move = *legal_moves[random_idx(legal_moves.size())].get();
	for (int i = 0; i < legal_moves.size(); i++)
	{
		Move& move = *legal_moves[i].get();

		float c = 1;

		//TODO - loads of calculations
		float n_sum = 1;
		float n_at_move = 1;

		float utility =
			leonardo_util::matrix_map_get_float(q, game, move) +
			c * leonardo_util::matrix_map_get_float(p, game, move) *
			sqrt(n_sum) / (1 + n_at_move);

		if (utility > max_utility)
		{
			max_utility = utility;
			best_move = move;
		}
	}

	ChessBoard new_game = game.getCopyByValue();
	new_game.makeMove(best_move);

	float evaluation = search(new_game, n, p, q, visited);

	//calculate the new average evaulation for the current move
	leonardo_util::matrix_map_set_float(q, game, best_move,
		(leonardo_util::matrix_map_get_float(n, game, best_move) * leonardo_util::matrix_map_get_float(q, game, best_move) + evaluation) /
		(leonardo_util::matrix_map_get_float(n, game, best_move) + 1)
	);

	//add 1 to the number of times the current move was explored
	leonardo_util::matrix_map_set_float(n, game, best_move,
		leonardo_util::matrix_map_get_float(n, game, best_move) + 1);

	return -evaluation;
}
void leonardo_overlord::train_nn_on_ds(
	neural_network& nnet,
	data_space& ds,
	size_t epochs,
	size_t batch_size,
	float learning_rate)
{
	for (size_t epoch = 0; epoch < epochs; epoch++)
	{
		ds.iterator_reset();

		size_t batch_item_idx = 0;
		while (ds.iterator_has_next())
		{
			nnet.back_propagation(ds.get_current_data(), ds.get_current_label());

			if (batch_item_idx >= batch_size)
			{
				nnet.apply_deltas(batch_size, learning_rate);
				batch_item_idx = 0;
			}
			else
			{
				batch_item_idx++;
			}
			if (ds.iterator_has_next())
			{
				ds.iterator_next();
			}
		}
	}
}
void leonardo_overlord::policy(matrix& output_matrix, const ChessBoard& game)
{
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> n;
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> p;
	std::unordered_map<ChessBoard, matrix, chess_board_hasher> q;
	std::unordered_set<ChessBoard, chess_board_hasher> visited;

	//1600 in openai
	for (int i = 0; i < 16; i++)
	{
		search(game, n, p, q, visited);
	}

	std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();
	for (int i = 0; i < legal_moves.size(); i++)
	{
		int idx = leonardo_util::get_matrix_idx_for_move(*legal_moves[i].get());

		//N[game.input_matrix()][curr_move]
		matrix& n_matrix = n[game];
		n_matrix.sync_device_and_host(); // just in case
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
		ChessBoard game(STARTING_FEN);

		size_t move_count = 0;
		while (true)
		{
			move_count++;

			std::cout << "move count: " << move_count << std::endl;

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
			leonardo_util::set_matrix_from_chessboard(game, input_matrix);

			policy(output_matrix, game);

			if (move_count <= number_of_moves_per_game)
			{
				policy_training_ds.set_current_data(input_matrix);
				policy_training_ds.set_current_label(output_matrix);
				prediction_training_ds.set_current_data(input_matrix);
			}

			//make move
			std::vector<std::unique_ptr<Move>> legal_moves = game.getAllLegalMoves();
			int move_idx = leonardo_util::get_random_best_move(output_matrix, legal_moves);
			game.makeMove(*legal_moves[move_idx].get());

			if (game.getGameState() != GameState::Ongoing)
			{
				//get current index of data space
				size_t end_idx = prediction_training_ds.get_iterator_idx();

				//set the win matrix - 0 0 if draw - 1 0 for white winning and 0 1 for black winning
				matrix final_game_state(leonardo_util::get_prediction_output_format());
				leonardo_util::set_prediction_output(final_game_state, game);

				//the whole game was saved, but we do not know the outcome, so we add that now
				for (size_t prediction_idx = start_idx; prediction_idx <= end_idx; prediction_idx++)
				{
					prediction_training_ds.set_iterator_idx(prediction_idx);
					prediction_training_ds.set_current_label(final_game_state);
				}

				policy_training_ds.iterator_next();
				prediction_training_ds.iterator_next();
				break;
			}

			policy_training_ds.iterator_next();
			prediction_training_ds.iterator_next();
		}
	}
}

void leonardo_overlord::upgrade()
{
	size_t number_of_selfplay_games = 1;
	size_t number_of_moves_per_game = 100;

	data_space policy_training_ds(
		number_of_selfplay_games * number_of_moves_per_game,
		leonardo_util::get_input_format(),
		leonardo_util::get_policy_output_format()
	);
	data_space prediction_training_ds(
		number_of_selfplay_games * number_of_moves_per_game,
		leonardo_util::get_input_format(),
		leonardo_util::get_prediction_output_format()
	);

	get_training_data(
		number_of_selfplay_games, 
		number_of_moves_per_game, 
		policy_training_ds, 
		prediction_training_ds);

	train_nn_on_ds(
		new_policy_nnet,
		policy_training_ds,
		20,
		20,
		0.1f
	);

	train_nn_on_ds(
		new_prediction_nnet,
		prediction_training_ds,
		20,
		20,
		0.1f
	);
}

leonardo_overlord::leonardo_overlord(
	std::string name
) : name(name)
{
	//best_network = neural_network("models\\learner_epoch_500.parameters");

	best_policy_nnet.set_input_format(leonardo_util::get_input_format());
	best_policy_nnet.add_fully_connected_layer(16, sigmoid_fn);
	best_policy_nnet.add_fully_connected_layer(16, sigmoid_fn);
	best_policy_nnet.add_fully_connected_layer(leonardo_util::get_policy_output_format(), sigmoid_fn);
	best_policy_nnet.set_all_parameters(0.0f);
	best_policy_nnet.apply_noise(1);

	best_prediction_nnet.set_input_format(leonardo_util::get_input_format());
	best_prediction_nnet.add_fully_connected_layer(16, sigmoid_fn);
	best_prediction_nnet.add_fully_connected_layer(16, sigmoid_fn);
	best_prediction_nnet.add_fully_connected_layer(leonardo_util::get_prediction_output_format(), sigmoid_fn);
	best_prediction_nnet.set_all_parameters(0.0f);
	best_prediction_nnet.apply_noise(1);

	new_policy_nnet = neural_network(best_policy_nnet);
	new_prediction_nnet = neural_network(best_prediction_nnet);

	//best_policy_nnet.enable_gpu_mode();
	//best_prediction_nnet.enable_gpu_mode();
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
		std::make_unique<leonardo_bot>(best_policy_nnet),
		std::make_unique<leonardo_bot>(new_policy_nnet)
	);

	//# training 25000 apparently
	//for i in range(iterations) :
	//	# learn on games
	for (int i = 0; i < 1; i++)
	{
		upgrade();

		//	# arena - with biased random instead of applying noise to input
		//	win_ratio = pit(policy_nn, new_policy_nn, n = number_of_games)
		int win_score = arena.play(1);

		//	# best input for next iteration
		//	threshold = 0.55  # hyperparameter - how much better the new net has to be
		//	if win_ratio > threshold:
		if (win_score < 0) // because our new network is alway black - TODO CHANGE
		{
			//policy_nn = new_policy_nn
			//syncing host and device gets done inside ?
			best_policy_nnet.set_parameters(new_policy_nnet);
			best_prediction_nnet.set_parameters(new_prediction_nnet);
		}
	}
}

const neural_network& leonardo_overlord::get_best_network() const
{
	return best_policy_nnet;
}
