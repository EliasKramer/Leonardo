# https://towardsdatascience.com/alphazero-chess-how-it-works-what-sets-it-apart-and-what-it-can-tell-us-4ab3d2d08867

# data structure for visited states
# append and check if visited

# data structure for Q, N, P
# | input matrix | output_matrix
# | input matrix | output_matrix
# random access
# map(matrix, map(matrix, float))


def search(game, policy_network, prediction_nn):
    if gameOver(input_matrix): return outcome(input_matrix)

    if game.input_matrix() not in visited:
        visited.append(game.input_matrix())
        output_matrix = policy_network.forward_prop(game.input_matrix())
        for curr_move in legalMoves(game):
            P[game.input_matrix()][curr_move] = output_matrix[curr_move]
            N[game.input_matrix()][curr_move] = 1 # number of times the current move was explored - maybe 0?

        evaluation = prediction_nn.forward_prop(game.input_matrix())
        return -evaluation

    max_u = -float("inf")
    best_move = random.choice(legalMoves(input_matrix))
    for curr_move in legalMoves(game):
        # Q is the average evaluation of all moves that explored it
        # c is a hyperparameter that controls exploration vs exploitation
        # P indicates how promising a move is
        # N is the number of times the current move was explored
        # u is the best move-exploration-evaluation-score
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
    # we make the best move with e&e in mind and then search again
    # we do this recursively until we reach a leaf node
    evaluation = search(new_game, policy_network, prediction_nn)

    Q[game.input_matrix()][best_move] = \
        # current number of moves * current average evaluation + evaluation of current move
    # divided by current number of moves + 1
    (N[game.input_matrix()][curr_move] * Q[game.input_matrix()][curr_move] + evaluation) / \
    (N[input_matrix][curr_move] + 1)


# add 1 to the number of times the current move was explored
N[input_matrix][curr_move] += 1
return -evaluation


def policy(game,policy_network, prediction_network):
    # number_of_simulations = 1600 on alphazero
    for i in range(number_of_simulations):
        search(game, policy_network, prediction_network)

    output_matrix;
    for curr_move in legal_moves(game)
        output_matrix[curr_move] = N[game.input_matrix()][curr_move]

    return output_matrix


# ----------------------------------------------
# TODO implement chess data space
def data(policy_network, prediction_network):
    chess_data_space = (
        input = input_matrix_size,
    output = output_matrix_size,
    game_ending = 2,
    data_count = number_of_selfplay_games * number_of_moves_per_game)

    for i in range(number_of_selfplay_games):
        # star game - duh
        game = startingPosition()
        chess_data_space.new_game()  # remember index, to add outcome later
        while True:
            # get move of policy network
            output_matrix = policy(game, policy_network, prediction_network)
            # add input matrix and output matrix to training set
            chess_data_space.append(game, output_matrix)
            game.execute_move(matrix_to_move(output_matrix))

            chess_data_space.append
            data_space_for_win_prediction.append(input_matrix, inconclusive_ending)

            if gameOver(game):
                chess_data_space.set_last_outcomes(outcome(game))
                break
    return examples


def upgrade(
        policy_network,
        win_prediction_model,
        new_policy_network,
        new_win_prediction_model):
    chess_data_space = data(policy_network, win_prediction_model)

    new_policy_network.train(chess_data_space)
    new_win_prediction_model.train(chess_data_space)


# ----------------------------------------------
def finalnet():
    # neural network
    policy_nn;  # initialize
    prediction_nn;  # initialize
    new_policy_nn;  # initialize
    new_prediction_nn;  # initialize

    # training 25000 apparently
    for i in range(iterations):
        # learn on games
        upgrade(
            policy_nn,
            prediction_nn,
            new_policy_nn,
            new_prediction_nn)
        # arena - with biased random instead of applying noise to input
        win_ratio = pit(policy_nn, new_policy_nn, n=number_of_games)
        # best input for next iteration
        threshold = 0.55  # hyperparameter - how much better the new net has to be
        if win_ratio > threshold:
            policy_nn = new_policy_nn
    return policy_nn
