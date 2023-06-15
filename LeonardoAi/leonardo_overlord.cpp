#include "leonardo_overlord.hpp"
#include "chess_arena.hpp"
#include "leonardo_bot.hpp"
#include <memory>
#include "NeuroFox/data_space.hpp"

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

float leonardo_overlord::search(const ChessBoard& game)
{
	/*def search(game, policy_network, prediction_nn) :
		if gameOver(input_matrix) : return outcome(input_matrix)

			if game.input_matrix() not in visited :
	visited.append(game.input_matrix())
		output_matrix = policy_network.forward_prop(game.input_matrix())
		for curr_move in legalMoves(game) :
			P[game.input_matrix()][curr_move] = output_matrix[curr_move]
			N[game.input_matrix()][curr_move] = 1 # number of times the current move was explored - maybe 0 ?

			evaluation = prediction_nn.forward_prop(game.input_matrix())
			return -evaluation

			max_u = -float("inf")
			best_move = random.choice(legalMoves(input_matrix))
			for curr_move in legalMoves(game) :
				# Q is the average evaluation of all moves that explored it
				# c is a hyperparameter that controls exploration vs exploitation
				# P indicates how promising a move is
				# N is the number of times the current move was explored
				# u is the best move - exploration - evaluation - score
				u = \
				#exploitation - which move has the highest average evaluation
				Q[game.input_matrix()][curr_move] + c * \
				#exploration - which move has the highest promise
# - was evaluated very high, but not explored much
				P[game.input_matrix()][curr_move] * \
				sqrt(sum(N[game.input_matrix()])) \
				/ (1 + N[game.input_matrix()][curr_move])

				if u > max_u:
	max_u = u
		best_move = curr_move

		new_game = makeMove(game, best_move)
		# we make the best move with e & e in mind and then search again
		# we do this recursively until we reach a leaf node
		evaluation = search(new_game, policy_network, prediction_nn)

		Q[game.input_matrix()][best_move] = \
		# current number of moves * current average evaluation + evaluation of current move
		# divided by current number of moves + 1
		(N[game.input_matrix()][curr_move] * Q[game.input_matrix()][curr_move] + evaluation) / \
		(N[input_matrix][curr_move] + 1)


		# add 1 to the number of times the current move was explored
		N[input_matrix][curr_move] += 1
		return -evaluation*/
	return 0.0f;
}

void leonardo_overlord::policy(matrix& output_matrix, const ChessBoard& game)
{
	/*def policy(game, policy_network, prediction_network) :
		# number_of_simulations = 1600 on alphazero
		for i in range(number_of_simulations) :
			search(game, policy_network, prediction_network)

			output_matrix;
	for curr_move in legal_moves(game)
		output_matrix[curr_move] = N[game.input_matrix()][curr_move]

		return output_matrix*/
}

void leonardo_overlord::get_training_data()
{
	size_t number_of_selfplay_games = 100;
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

	policy_training_ds.iterator_reset();
	prediction_training_ds.iterator_reset();

	for (int i = 0; i < number_of_selfplay_games; i++)
	{
		ChessBoard game(STARTING_FEN);

		while (true)
		{
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

			policy_training_ds.set_current_data(input_matrix);
			policy_training_ds.set_current_label(output_matrix);
			prediction_training_ds.set_current_data(input_matrix);

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
	//chess data space
	//input pos - output pos - win or lose

	//get data

	//train on data
	//policy nnet
	//prediction nnet
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
	for (int i = 0; i < 25000; i++)
	{
		//	upgrade(
		//		policy_nn,
		//		prediction_nn,
		//		new_policy_nn,
		//		new_prediction_nn)
		//todo
		void upgrade();

		//	# arena - with biased random instead of applying noise to input
		//	win_ratio = pit(policy_nn, new_policy_nn, n = number_of_games)
		int win_score = arena.play(1000);

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
