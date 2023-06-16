#include "pooling_layer.hpp"

pooling_layer::pooling_layer(
	size_t filter_size,
	size_t stride,
	e_pooling_type_t pooling_fn
)
	:layer(e_layer_type_t::pooling),
	filter_size(filter_size),
	stride(stride),
	pooling_fn(pooling_fn)
{
	if (filter_size == 0)
		throw std::invalid_argument("filter size must be greater than 0");
	if (stride == 0)
		throw std::invalid_argument("stride must be greater than 0");
	if (stride > filter_size)
		throw std::invalid_argument("stride must be smaller or equal than the filter size");
}

pooling_layer::pooling_layer(std::ifstream& file)
	:layer(file, e_layer_type_t::pooling)
{
	if (!file.is_open())
	{
		throw std::runtime_error("file is not open");
	}
	file.read((char*)&filter_size, sizeof(filter_size));
	file.read((char*)&stride, sizeof(stride));
	file.read((char*)&pooling_fn, sizeof(pooling_fn));
}

std::unique_ptr<layer> pooling_layer::clone() const
{
	return std::make_unique<pooling_layer>(*this);
}

size_t pooling_layer::get_parameter_count() const
{
	throw std::invalid_argument("pooling layer does not have any parameters");
}

pooling_layer::pooling_layer(const pooling_layer& other)
	:layer(other),
	filter_size(other.filter_size),
	stride(other.stride),
	pooling_fn(other.pooling_fn)
{}

void pooling_layer::set_input_format(vector3 input_format)
{
	//does check if input_format has more than 0 on every dimension
	layer::set_input_format(input_format);
	
	if (input_format.x != input_format.y)
	{
		throw std::invalid_argument("pooling layer only supports square input");
	}

	const size_t output_width = convolution_output_size(input_format.x, filter_size, stride);
	const size_t output_height = convolution_output_size(input_format.y, filter_size, stride);

	activations = matrix(
		vector3(
			output_width,
			output_height,
			input_format.z));

	error = matrix(
		vector3(
			output_width,
			output_height,
			input_format.z));
}

size_t pooling_layer::get_filter_size() const
{
	return filter_size;
}

size_t pooling_layer::get_stride() const
{
	return stride;
}

e_pooling_type_t pooling_layer::get_pooling_fn() const
{
	return pooling_fn;
}

void pooling_layer::set_all_parameters(float value)
{
	throw std::invalid_argument("pooling layer does not have any parameters");
}

void pooling_layer::apply_noise(float range)
{
	throw std::invalid_argument("pooling layer does not have any parameters");
}

void pooling_layer::mutate(float range)
{
	throw std::invalid_argument("pooling layer does not have any parameters");
}

void pooling_layer::sync_device_and_host()
{
	layer::sync_device_and_host();
}

void pooling_layer::forward_propagation(const matrix& input)
{
	layer::forward_propagation(input);
	matrix::pooling(input, activations, filter_size, stride, pooling_fn);
}

void pooling_layer::back_propagation(const matrix& input, matrix* passing_error)
{
	layer::back_propagation(input, passing_error);
	throw std::exception("not implemented");
}

void pooling_layer::apply_deltas(size_t training_data_count, float learning_rate)
{
	throw std::invalid_argument("pooling layer does not have any parameters");
}

void pooling_layer::enable_gpu_mode()
{
	layer::enable_gpu_mode();
	gpu_enabled = true;
}

void pooling_layer::disable_gpu()
{
	layer::disable_gpu();
	gpu_enabled = false;
}

bool pooling_layer::equal_format(const layer& other)
{
	if (layer::equal_format(other))
	{
		const pooling_layer& other_cast = dynamic_cast<const pooling_layer&>(other);

		return filter_size == other_cast.filter_size &&
			stride == other_cast.stride &&
			pooling_fn == other_cast.pooling_fn;
	}

	return false;
}

bool pooling_layer::equal_parameter(const layer& other)
{
	return equal_format(other);
}

void pooling_layer::set_parameters(const layer& other)
{
	throw std::invalid_argument("pooling layer does not have any parameters");
}

void pooling_layer::write_to_ofstream(std::ofstream& file) const
{
	layer::write_to_ofstream(file);
	file.write((char*)&filter_size, sizeof(filter_size));
	file.write((char*)&stride, sizeof(stride));
	file.write((char*)&pooling_fn, sizeof(pooling_fn));
}
