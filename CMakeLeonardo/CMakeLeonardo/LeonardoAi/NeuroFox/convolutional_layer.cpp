#include "convolutional_layer.hpp"

convolutional_layer::convolutional_layer(
	size_t number_of_kernels,
	size_t kernel_size,
	size_t stride,
	e_activation_t activation_function
)
	:layer(e_layer_type_t::convolutional),
	stride(stride),
	kernel_size(kernel_size),
	activation_fn(activation_function),
	kernel_count(number_of_kernels)
{
	if (number_of_kernels == 0)
		throw std::invalid_argument("number_of_kernels must be greater than 0");
	if (kernel_size == 0)
		throw std::invalid_argument("kernel_size must be greater than 0");
	if (stride == 0)
		throw std::invalid_argument("stride must be greater than 0");

	if (stride > kernel_size)
		throw std::invalid_argument("stride must be smaller or equal than the kernel_size");
}

convolutional_layer::convolutional_layer(std::ifstream& file)
	:layer(file, e_layer_type_t::convolutional)
{
	if (!file.is_open())
	{
		throw std::runtime_error("file is not open");
	}

	file.read((char*)&activation_fn, sizeof(activation_fn));
	file.read((char*)&kernel_size, sizeof(kernel_size));
	file.read((char*)&stride, sizeof(stride));
	file.read((char*)&kernel_count, sizeof(kernel_count));

	for (size_t i = 0; i < kernel_count; i++)
	{
		kernel_weights.push_back(matrix(file));
		kernel_weights_deltas.push_back(kernel_weights[0].get_format());
	}
	kernel_biases = matrix(file);
	kernel_bias_deltas = matrix(kernel_biases.get_format());
}

convolutional_layer::convolutional_layer(
	const convolutional_layer& other
) :
	layer(other),
	kernel_size(other.kernel_size),
	stride(other.stride),
	kernel_count(other.kernel_count),
	activation_fn(other.activation_fn),
	kernel_biases(other.kernel_biases),
	kernel_bias_deltas(other.kernel_bias_deltas, false) // do not copy the deltas
{
	for (const auto& kernel : other.kernel_weights)
		kernel_weights.push_back(matrix(kernel));
	for (const auto& kernel : other.kernel_weights_deltas)
		kernel_weights_deltas.push_back(matrix(kernel, false)); // do noty copy the deltas
}

std::unique_ptr<layer> convolutional_layer::clone() const
{
	return std::make_unique<convolutional_layer>(*this);
}

size_t convolutional_layer::get_parameter_count() const
{
	size_t result = 0;

	for (const auto& kernel : kernel_weights)
		result += kernel.item_count();

	result += kernel_biases.item_count();

	return result;
}

size_t convolutional_layer::get_kernel_size() const
{
	return kernel_size;
}

size_t convolutional_layer::get_stride() const
{
	return stride;
}

size_t convolutional_layer::get_kernel_count() const
{
	return kernel_count;
}

std::vector<matrix>& convolutional_layer::get_kernel_weights()
{
	return kernel_weights;
}

const std::vector<matrix>& convolutional_layer::get_kernel_weights_readonly() const
{
	return kernel_weights;
}

matrix& convolutional_layer::get_kernel_biases()
{
	return kernel_biases;
}

const matrix& convolutional_layer::get_kernel_biases_readonly() const
{
	return kernel_biases;
}

void convolutional_layer::set_input_format(vector3 input_format)
{
	layer::set_input_format(input_format);

	const size_t input_depth = input_format.z;

	const size_t output_width = convolution_output_size(input_format.x, kernel_size, stride);
	const size_t output_height = convolution_output_size(input_format.y, kernel_size, stride);

	activations = matrix(vector3(output_width, output_height, kernel_count));
	error = matrix(activations.get_format());

	for (int i = 0; i < kernel_count; i++)
	{
		kernel_weights.push_back(matrix(vector3(kernel_size, kernel_size, input_depth)));
		kernel_weights_deltas.push_back(matrix(vector3(kernel_size, kernel_size, input_depth)));

	}
	kernel_biases = matrix(
		vector3(
			output_width,
			output_height,
			kernel_count));
	kernel_bias_deltas = matrix(
		vector3(
			output_width,
			output_height,
			kernel_count));
}

void convolutional_layer::set_all_parameters(float value)
{
	for (matrix& weights : kernel_weights)
	{
		weights.set_all(value);
	}
	kernel_biases.set_all(value);
}

void convolutional_layer::apply_noise(float range)
{
	for (matrix& weights : kernel_weights)
	{
		weights.apply_noise(range);
	}
	kernel_biases.apply_noise(range);
}

void convolutional_layer::mutate(float range)
{
	//choose if a weight or a bias is mutated
	if (biased_coin_toss(
		(float)kernel_weights[0].item_count() * kernel_weights.size(),
		(float)kernel_biases.item_count()))
	{
		//choose a random weight to mutate
		int random_weight_idx =
			random_idx((int)kernel_weights[0].item_count());

		//mutate the weight
		kernel_weights[random_idx(kernel_weights.size())]
			.add_at_flat(random_weight_idx, random_float_excl(-range, range));
	}
	else
	{
		kernel_biases.mutate(range);
	}
}

std::string convolutional_layer::parameter_analysis() const
{
	std::string ret_val = layer::parameter_analysis();

	ret_val += std::string("\nkernel size: ") + std::to_string(kernel_size);
	ret_val += std::string("\nstride: ") + std::to_string(stride);
	ret_val += std::string("\nkernel count: ") + std::to_string(kernel_count);
	
	for(int i = 0; i < kernel_weights.size(); i++)
		ret_val += std::string("\nkernel ") + std::to_string(i) 
		+ std::string(" weights: \n") + kernel_weights[i].analyse_string() + "\n";

	ret_val += std::string("\nkernel biases: \n") + kernel_biases.analyse_string() + "\n";

	return ret_val;
}

void convolutional_layer::sync_device_and_host()
{
	layer::sync_device_and_host();

	for (matrix& weights : kernel_weights)
	{
		weights.sync_device_and_host();
	}
	kernel_biases.sync_device_and_host();

	for (matrix& weights : kernel_weights_deltas)
	{
		weights.sync_device_and_host();
	}
	kernel_bias_deltas.sync_device_and_host();
}

void convolutional_layer::forward_propagation(const matrix& input)
{
	layer::forward_propagation(input);

	const int output_width = activations.get_width();
	const int output_height = activations.get_height();
	const int input_depth = input.get_depth();

	if (activations.get_depth() != kernel_count)
		throw std::invalid_argument("activations depth must be equal to the number of kernels");

	activations.set_all(0);

	matrix::cross_correlation(
		input, kernel_weights, activations, stride);

	matrix::add(activations, kernel_biases, activations);

	activations.apply_activation_function(activation_fn);
}

void convolutional_layer::back_propagation(const matrix& input, matrix* passing_error)
{
	throw std::runtime_error("not implemented");
	layer::back_propagation(input, passing_error);
}

void convolutional_layer::apply_deltas(size_t training_data_count, float learning_rate)
{
	throw std::runtime_error("not implemented");
}

void convolutional_layer::enable_gpu_mode()
{
	layer::enable_gpu_mode();

	for (int i = 0; i < kernel_count; i++)
	{
		kernel_weights[i].enable_gpu_mode();
		kernel_weights_deltas[i].enable_gpu_mode();
	}
	kernel_biases.enable_gpu_mode();
	kernel_bias_deltas.enable_gpu_mode();
}

void convolutional_layer::disable_gpu()
{
	//gpu_kernel_weights.clear();
	//gpu_kernel_biases = nullptr;
}

bool convolutional_layer::equal_format(const layer& other)
{
	if (layer::equal_format(other))
	{
		const convolutional_layer& other_conv = dynamic_cast<const convolutional_layer&>(other);

		return
			//TODO convert kernelsize into kernel format
			activation_fn == other_conv.activation_fn &&
			kernel_size == other_conv.kernel_size &&
			stride == other_conv.stride &&
			kernel_count == other_conv.kernel_count;
	}
	return false;
}

bool convolutional_layer::equal_parameter(const layer& other)
{
	if (layer::equal_format(other))
	{
		const convolutional_layer& other_conv = dynamic_cast<const convolutional_layer&>(other);

		if (kernel_weights.size() == other_conv.kernel_weights.size())
		{
			for (int i = 0; i < kernel_weights.size(); i++)
			{
				if (!matrix::are_equal(kernel_weights[i], other_conv.kernel_weights[i]))
					return false;
			}
			return matrix::are_equal(kernel_biases, other_conv.kernel_biases);
		}
	}
	return false;
}

void convolutional_layer::set_parameters(const layer& other)
{
	if (layer::equal_format(other))
	{
		const convolutional_layer& other_conv = dynamic_cast<const convolutional_layer&>(other);

		if (kernel_weights.size() == other_conv.kernel_weights.size())
		{
			for (int i = 0; i < kernel_weights.size(); i++)
			{
				kernel_weights[i].set_data_from_src(other_conv.kernel_weights[i]);
			}
			kernel_biases.set_data_from_src(other_conv.kernel_biases);
		}
		else
		{
			throw std::invalid_argument("kernel weight count does not match");
		}
	}
	else
	{
		throw std::invalid_argument("layer format does not match");
	}
}

void convolutional_layer::write_to_ofstream(std::ofstream& file) const
{
	layer::write_to_ofstream(file);
	file.write((char*)&activation_fn, sizeof(activation_fn));
	file.write((char*)&kernel_size, sizeof(kernel_size));
	file.write((char*)&stride, sizeof(stride));
	file.write((char*)&kernel_count, sizeof(kernel_count));
	for (const matrix& weights : kernel_weights)
	{
		weights.write_to_ofstream(file);
	}
	kernel_biases.write_to_ofstream(file);
}