#include "fully_connected_layer.hpp"

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
	bias_deltas(activation_format),
	bias_momentum(activation_format)
{}

fully_connected_layer::fully_connected_layer(std::ifstream& file)
	:layer(file, e_layer_type_t::fully_connected)
{
	file.read((char*)&activation_fn, sizeof(activation_fn));
	weights = matrix(file);
	biases = matrix(file);

	weight_deltas = matrix(weights.get_format());
	bias_deltas = matrix(biases.get_format());

	weight_momentum = matrix(weights.get_format());
	bias_momentum = matrix(biases.get_format());
}

fully_connected_layer::fully_connected_layer(
	const fully_connected_layer& other
) :
	layer(other),
	weights(other.weights),
	biases(other.biases),
	activation_fn(other.activation_fn),
	weight_deltas(other.weight_deltas, false), //do not copy the deltas
	bias_deltas(other.bias_deltas, false), //do not copy the deltas
	weight_momentum(other.weight_momentum, false), //do not copy the momentum
	bias_momentum(other.bias_momentum, false) //du not copy the momentum
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
	weight_momentum = matrix(weights.get_format());
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

std::string fully_connected_layer::parameter_analysis() const
{
	std::string ret_val = layer::parameter_analysis();
	ret_val += "Weights: \n" + weights.analyse_string();
	ret_val += "Biases: \n" + biases.analyse_string() + "\n";
	return ret_val;
}

void fully_connected_layer::sync_device_and_host()
{
	layer::sync_device_and_host();

	weights.sync_device_and_host();
	biases.sync_device_and_host();
	weight_deltas.sync_device_and_host();
	bias_deltas.sync_device_and_host();
	weight_momentum.sync_device_and_host();
	bias_momentum.sync_device_and_host();
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

	matrix::fully_connected_backprop(
		activations,
		weights,
		input,
		error,
		passing_error,
		weight_deltas,
		bias_deltas,
		activation_fn
	);
}

void fully_connected_layer::apply_deltas(size_t training_data_count, float learning_rate)
{
	biases.apply_deltas(bias_deltas, bias_momentum, training_data_count, learning_rate);
	weights.apply_deltas(weight_deltas, weight_momentum, training_data_count, learning_rate);
}

void fully_connected_layer::enable_gpu_mode()
{
	layer::enable_gpu_mode();

	weights.enable_gpu_mode();
	biases.enable_gpu_mode();
	weight_deltas.enable_gpu_mode();
	bias_deltas.enable_gpu_mode();
	weight_momentum.enable_gpu_mode();
	bias_momentum.enable_gpu_mode();
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
