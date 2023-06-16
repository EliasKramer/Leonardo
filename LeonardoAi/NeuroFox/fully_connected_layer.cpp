#include "fully_connected_layer.hpp"

float fully_connected_layer::get_weight_at(int input_layer_idx, int current_activation_idx) const
{
	return weights.get_at_host(vector3(input_layer_idx, current_activation_idx));
}

void fully_connected_layer::set_weight_at(int input_layer_idx, int current_activation_idx, float value)
{
	weights.set_at(vector3(input_layer_idx, current_activation_idx), value);
}

float fully_connected_layer::get_weight_delta_at(int input_layer_idx, int current_activation_idx) const
{
	return weight_deltas.get_at_host(vector3(input_layer_idx, current_activation_idx));
}

void fully_connected_layer::set_weight_delta_at(int input_layer_idx, int current_activation_idx, float value)
{
	weight_deltas.set_at(vector3(input_layer_idx, current_activation_idx), value);
}

fully_connected_layer::fully_connected_layer(
	size_t number_of_neurons,
	e_activation_t activation_function
)
	:fully_connected_layer(
		vector3(1, number_of_neurons, 1),
		activation_function)
{}

fully_connected_layer::fully_connected_layer(
	vector3 activation_format,
	e_activation_t activation_function
) :
	layer(activation_format, e_layer_type_t::fully_connected),
	activation_fn(activation_function),
	biases(activation_format),
	bias_deltas(activation_format)
{}

fully_connected_layer::fully_connected_layer(std::ifstream & file)
	:layer(file, e_layer_type_t::fully_connected)
{
	file.read((char*)&activation_fn, sizeof(activation_fn));
	weights = matrix(file);
	biases = matrix(file);
	weight_deltas = matrix(weights.get_format());
	bias_deltas = matrix(biases.get_format());
}

fully_connected_layer::fully_connected_layer(
	const fully_connected_layer & other
) : 
	layer(other),
	weights(other.weights),
	biases(other.biases),
	weight_deltas(other.weight_deltas, false), //do not copy the deltas
	bias_deltas(other.bias_deltas, false), //do not copy the deltas
	activation_fn(other.activation_fn)
{}

std::unique_ptr<layer> fully_connected_layer::clone() const
{
	return std::make_unique<fully_connected_layer>(*this);
}

size_t fully_connected_layer::get_parameter_count() const
{
	return weights.item_count() + biases.item_count();
}

void fully_connected_layer::set_input_format(vector3 input_format)
{
	layer::set_input_format(input_format);

	weights = matrix(
		vector3(
			input_format.item_count(),
			activations.item_count(),
			(size_t)1));
	weight_deltas = matrix(weights.get_format());
}

const matrix& fully_connected_layer::get_weights() const
{
	return weights;
}

const matrix& fully_connected_layer::get_biases() const
{
	return biases;
}

matrix& fully_connected_layer::get_weights_ref()
{
	return weights;
}

matrix& fully_connected_layer::get_biases_ref()
{
	return biases;
}

void fully_connected_layer::set_all_parameters(float value)
{
	biases.set_all(value);
	weights.set_all(value);
}

void fully_connected_layer::apply_noise(float range)
{
	biases.apply_noise(range);
	weights.apply_noise(range);
}

void fully_connected_layer::mutate(float range)
{
	if (biased_coin_toss(
		(float)weights.item_count(),
		(float)biases.item_count()))
	{
		weights.mutate(range);
	}
	else
	{
		biases.mutate(range);
	}
}

void fully_connected_layer::sync_device_and_host()
{
	layer::sync_device_and_host();

	weights.sync_device_and_host();
	biases.sync_device_and_host();
	weight_deltas.sync_device_and_host();
	bias_deltas.sync_device_and_host();
}

void fully_connected_layer::forward_propagation(const matrix& input)
{
	layer::forward_propagation(input);

	matrix::dot_product_flat(weights, input, activations);
	matrix::add_flat(activations, biases, activations);
	activations.apply_activation_function(activation_fn);
}

void fully_connected_layer::back_propagation(const matrix& input, matrix* passing_error)
{
	layer::back_propagation(input, passing_error);

	if (!matrix::equal_format(activations, error))
	{
		throw std::invalid_argument("activations and error_right have different format");
	}

	for (int current_neuron_idx = 0; current_neuron_idx < activations.item_count(); current_neuron_idx++)
	{
		float error_value = error.get_at_flat_host(current_neuron_idx);
		//clear the error
		error.set_at_flat(current_neuron_idx, 0.0f);

		float current_activation_value = activations.get_at_flat_host(current_neuron_idx);
		float unactivated_activation = INVERSE[activation_fn](current_activation_value);
		float activation_derivative = DERIVATIVE[activation_fn](unactivated_activation);

		//bias change
		float bias_change = error_value * activation_derivative;
		bias_deltas.add_at_flat(current_neuron_idx, bias_change);

		//iterate input layer
		for (int current_input_idx = 0; current_input_idx < input.item_count(); current_input_idx++)
		{
			float current_previous_activation = input.get_at_flat_host(current_input_idx);

			//this weight connects the current input node to the current neuron
			float current_weight = get_weight_at(current_input_idx, current_neuron_idx);

			float weight_change = error_value * activation_derivative * current_previous_activation;

			float current_weight_change = get_weight_delta_at(current_input_idx, current_neuron_idx);
			set_weight_delta_at(current_input_idx, current_neuron_idx, current_weight_change + weight_change);

			//passing error is null when this is the first layer
			if (passing_error != nullptr)
			{
				float new_passing_error = error_value * activation_derivative * current_weight;
				passing_error->add_at_flat(current_input_idx, new_passing_error);
			}
		}
	}
}
/*
void fully_connected_layer::forward_propagation_gpu(const gpu_matrix& input)
{
	layer::forward_propagation_gpu(input);

	if (!gpu_weights || !gpu_biases || !gpu_activations)
	{
		throw std::invalid_argument("gpu_weights, gpu_biases or gpu_activations is null");
	}

	gpu_dot_product(*gpu_weights.get(), input, *gpu_activations.get());
	gpu_add(*gpu_activations.get(), *gpu_biases.get(), *gpu_activations.get());
	GPU_ACTIVATION[activation_fn](*gpu_activations.get());
}

void fully_connected_layer::back_propagation_gpu(const gpu_matrix& input, gpu_matrix* passing_error)
{
	layer::back_propagation_gpu(input, passing_error);
	throw std::exception("not implemented");
}*/

void fully_connected_layer::apply_deltas(size_t training_data_count, float learning_rate)
{
	for (int i = 0; i < biases.item_count(); i++)
	{
		float current_bias = biases.get_at_flat_host(i);
		float avg_bias_delta = bias_deltas.get_at_flat_host(i) / training_data_count;
		biases.set_at_flat(i, current_bias - (avg_bias_delta * learning_rate));
		bias_deltas.set_at_flat(i, 0.0f);
	}

	for (int i = 0; i < weights.item_count(); i++)
	{
		float current_weight = weights.get_at_flat_host(i);
		float avg_weight_delta = weight_deltas.get_at_flat_host(i) / training_data_count;
		weights.set_at_flat(i, current_weight - (avg_weight_delta * learning_rate));
		weight_deltas.set_at_flat(i, 0.0f);
	}
}

void fully_connected_layer::enable_gpu_mode()
{
	layer::enable_gpu_mode();

	weights.enable_gpu_mode();
	biases.enable_gpu_mode();
	weight_deltas.enable_gpu_mode();
	bias_deltas.enable_gpu_mode();
}

void fully_connected_layer::disable_gpu()
{
}

bool fully_connected_layer::equal_format(const layer& other)
{
	if (layer::equal_format(other))
	{
		const fully_connected_layer& other_casted = dynamic_cast<const fully_connected_layer&>(other);
		//we could check for the deltas as well, but they should always be the same format
		//weights should have the same format as weight deltas
		//biases should have the same format as bias deltas
		return
			activation_fn == other_casted.activation_fn &&
			matrix::equal_format(weights, other_casted.weights) &&
			matrix::equal_format(biases, other_casted.biases);
	}
	return false;
}

bool fully_connected_layer::equal_parameter(const layer& other)
{
	if (equal_format(other))
	{
		const fully_connected_layer& other_casted = dynamic_cast<const fully_connected_layer&>(other);
		return
			matrix::are_equal(weights, other_casted.weights) &&
			matrix::are_equal(biases, other_casted.biases);
	}

	return false;
}

void fully_connected_layer::set_parameters(const layer& other)
{
	if (equal_format(other))
	{
		const fully_connected_layer& other_casted = dynamic_cast<const fully_connected_layer&>(other);
		weights.set_data_from_src(other_casted.weights);
		biases.set_data_from_src(other_casted.biases);
	}
	else
	{
		throw std::invalid_argument("other does not have the same format");
	}
}

void fully_connected_layer::write_to_ofstream(std::ofstream& file) const
{
	layer::write_to_ofstream(file);
	file.write((char*)&activation_fn, sizeof(activation_fn));
	weights.write_to_ofstream(file);
	biases.write_to_ofstream(file);
}
