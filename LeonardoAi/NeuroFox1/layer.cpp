#include "layer.hpp"

void layer::valid_input_check(const matrix& input) const
{
	if (input_format.item_count() == 0 || // the input format is not set
		matrix::equal_format(input_format, input) == false)
	{
		throw std::runtime_error("input format is not set or does not match the input format");
	}
}

void layer::valid_passing_error_check_cpu(const matrix* passing_error) const
{
	if (passing_error == nullptr)
	{
		//a passing error is null when the layer is the very first in the network
		return;
	}
	//the passing error has the same format as the input format
	valid_input_check(*passing_error);
}

layer::layer(std::ifstream& file, e_layer_type_t given_type)
{
	if (!file.is_open())
	{
		throw std::runtime_error("file is not open");
	}
	type = given_type;
	input_format = vector3(file);
	vector3 activation_format(file);

	activations = matrix(activation_format);
	error = matrix(activation_format);
}

layer::layer(e_layer_type_t given_layer_type)
	: type(given_layer_type)
{}

layer::layer(
	vector3 activation_format,
	e_layer_type_t given_layer_type
) :
	type(given_layer_type),
	activations(activation_format),
	error(activation_format)
{}

layer::layer(
	const layer& other
) :
	type(other.type),
	activations(other.activations, false), // copy the format - not the values
	error(other.error, false), //copy the format - not the values
	input_format(other.input_format)
{}

const e_layer_type_t layer::get_layer_type() const
{
	return type;
}

size_t layer::get_param_byte_size() const
{
	return get_parameter_count() * sizeof(float);
}

void layer::set_input_format(vector3 given_input_format)
{
	if (given_input_format.item_count() == 0)
	{
		throw std::invalid_argument("input format must be set");
	}
	this->input_format = given_input_format;
}

const matrix& layer::get_activations_readonly() const
{
	return activations;
}

matrix& layer::get_activations()
{
	return activations;
}

matrix* layer::get_activations_p()
{
	return &activations;
}

const matrix& layer::get_error() const
{
	return error;
}

matrix* layer::get_error_p()
{
	return &error;
}

void layer::set_error_for_last_layer(const matrix& expected)
{
	if (!matrix::equal_format(activations, expected))
	{
		throw std::runtime_error("setting error for the last layer could not be done. wrong expected matrix format");
	}
	//this calculates the const derivative
	matrix::subtract(activations, expected, error);
	error.scalar_multiplication(2);
}

void layer::enable_gpu_mode()
{
	activations.enable_gpu_mode();
	error.enable_gpu_mode();

	//input_format.enable_gpu();
	//gpu_activations = std::make_unique<gpu_matrix>(activations, true);
}

void layer::disable_gpu()
{
	//gpu_activations = nullptr;
	//gpu_error = nullptr;
}

bool layer::equal_format(const layer& other)
{
	return
		input_format == other.input_format &&
		type == other.type &&
		matrix::equal_format(activations, other.activations) &&
		matrix::equal_format(error, other.error) &&
		matrix::equal_format(input_format, other.input_format);
}

void layer::write_to_ofstream(std::ofstream& file) const
{
	file.write((char*)&type, sizeof(e_layer_type_t));
	input_format.write_to_ofstream(file);
	activations.get_format().write_to_ofstream(file);
}

void layer::sync_device_and_host()
{
	activations.sync_device_and_host();
	error.sync_device_and_host();
}

void layer::forward_propagation(const matrix& input)
{
	valid_input_check(input);
}

void layer::back_propagation(const matrix& input, matrix* passing_error)
{
	valid_input_check(input);
	valid_passing_error_check_cpu(passing_error);
}