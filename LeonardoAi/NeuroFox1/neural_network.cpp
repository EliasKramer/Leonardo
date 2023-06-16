#include "neural_network.hpp"
#include "util.hpp"
#include <fstream>

const float FILE_MAGIC_NUMBER = (float)0xfacade;

layer* neural_network::get_last_layer()
{
	//the last layer is the layer that was added last or nullptr 
	//if there are no layers yet
	return layers.empty() ? nullptr : layers.back().get();
}

neural_network::neural_network()
{}
neural_network::neural_network(const std::string& file)
{
	std::ifstream input(file, std::ios::binary | std::ios::in);
	try
	{
		if (!input.is_open())
			throw std::runtime_error("Could not open file " + file);

		//read the magic number
		float magic_number;
		input.read((char*)&magic_number, sizeof(float));

		if (magic_number != FILE_MAGIC_NUMBER)
			throw std::runtime_error("file is invalid");

		input_format = vector3(input);

		size_t layer_count;
		input.read((char*)&layer_count, sizeof(size_t));

		for (size_t i = 0; i < layer_count; i++)
		{
			e_layer_type_t layer_type;
			input.read((char*)&layer_type, sizeof(e_layer_type_t));
			switch (layer_type)
			{
			case e_layer_type_t::convolutional:
				parameter_layer_indices.push_back(layers.size());
				layers.push_back(std::move(std::make_unique<convolutional_layer>(input)));
				break;
			case e_layer_type_t::fully_connected:
				parameter_layer_indices.push_back(layers.size());
				layers.push_back(std::move(std::make_unique<fully_connected_layer>(input)));
				break;
			case e_layer_type_t::pooling:
				layers.push_back(std::move(std::make_unique<pooling_layer>(input)));
				break;

			default:
				throw std::runtime_error("Unknown layer type");
			}
		}

		gpu_enabled = false;
	}
	catch (const std::exception& e)
	{
		input.close();
		throw e;
	}
	input.close();
}

neural_network::neural_network(const neural_network& source)
{
	layers = std::vector<std::unique_ptr<layer>>();
	//copy all layers
	for (const auto& curr : source.layers)
	{
		layers.push_back(std::move(curr->clone()));
	}

	//copy the input format
	input_format = source.input_format;

	//copy the parameter layer indices
	parameter_layer_indices = source.parameter_layer_indices;

	//copy the gpu_enabled flag
	gpu_enabled = source.gpu_enabled;
}
neural_network& neural_network::operator=(const neural_network& source)
{
	if (this != &source)
	{
		layers = std::vector<std::unique_ptr<layer>>();
		for (const auto& curr : source.layers)
		{
			layers.push_back(std::move(curr->clone()));
		}

		//copy the input format
		input_format = source.input_format;

		//copy the parameter layer indices
		parameter_layer_indices = source.parameter_layer_indices;

		//copy the gpu_enabled flag
		gpu_enabled = source.gpu_enabled;

	}
	return *this;
}

size_t neural_network::get_param_count() const
{
	size_t result = 0;

	for (int i = 0; i < parameter_layer_indices.size(); i++)
	{
		result += layers[parameter_layer_indices[i]].get()->get_parameter_count();
	}

	return result;
}

size_t neural_network::get_param_byte_size() const
{
	return get_param_count() * sizeof(float);
}

void neural_network::set_input_format(vector3 given_input_format)
{
	if (input_format.item_count() != 0)
		throw std::runtime_error("Cannot set input format twice.");

	this->input_format = given_input_format;
}

const matrix& neural_network::get_output_readonly() const
{
	if (layers.empty())
		throw std::runtime_error("Cannot get output of neural network with no layers.");
	return layers.back().get()->get_activations_readonly();
}

matrix& neural_network::get_output()
{
	if (layers.empty())
		throw std::runtime_error("Cannot get output of neural network with no layers.");
	return layers.back().get()->get_activations();
}

void neural_network::add_layer(std::unique_ptr<layer>&& given_layer)
{
	//add the index of the layer to the vector of parameter layers
	//if the layer is not a pooling layer
	//because pooling layers do not have parameters
	if (given_layer->get_layer_type() != e_layer_type_t::pooling)
	{
		parameter_layer_indices.push_back((int)layers.size());
	}

	if (layers.empty())
	{
		//if there are no layers yet, the input format of the first layer
		//is the input format of the neural network
		given_layer->set_input_format(input_format);
	}
	else
	{
		//if there are already layers,
		//set the previous layer of the new layer to the last layer
		given_layer->set_input_format(get_last_layer()->get_activations_readonly().get_format());
	}

	//putting the new layer into the vector of layers
	layers.push_back(std::move(given_layer));
}

float neural_network::calculate_cost(const matrix& expected_output)
{
	if (matrix::equal_format(get_output_readonly(), expected_output) == false)
	{
		throw std::runtime_error("Output format does not match expected output format.");
	}

	float cost = 0.0f;
	for (int i = 0; i < expected_output.item_count(); i++)
	{
		float expected = expected_output.get_at_flat_host(i);
		float actual = get_output_readonly().get_at_flat_host(i);
		cost += ((actual - expected) * (actual - expected));
	}
	return cost;
}

void neural_network::sync_device_and_host()
{
	if (gpu_enabled)
	{
		for (auto& l : parameter_layer_indices)
		{
			layers[l]->sync_device_and_host();
		}
	}
}

void neural_network::add_fully_connected_layer(size_t num_neurons, e_activation_t activation_fn)
{
	std::unique_ptr<fully_connected_layer> new_layer =
		std::make_unique<fully_connected_layer>(num_neurons, activation_fn);

	add_layer(std::move(new_layer));
}

void neural_network::add_fully_connected_layer(vector3 neuron_format, e_activation_t activation_fn)
{
	std::unique_ptr<fully_connected_layer> new_layer =
		std::make_unique<fully_connected_layer>(neuron_format, activation_fn);
	add_layer(std::move(new_layer));
}

void neural_network::add_convolutional_layer(
	size_t number_of_kernels,
	size_t kernel_size,
	size_t stride,
	e_activation_t activation_fn)
{
	std::unique_ptr<convolutional_layer> new_layer =
		std::make_unique<convolutional_layer>(
			number_of_kernels,
			kernel_size,
			stride,
			activation_fn);

	add_layer(std::move(new_layer));
}

void neural_network::add_pooling_layer(size_t kernel_size, size_t stride, e_pooling_type_t pooling_type)
{
	//TODO
}

void neural_network::set_all_parameters(float value)
{
	//for parameter layers
	for (auto& l : parameter_layer_indices)
	{
		layers[l]->set_all_parameters(value);
	}
}

void neural_network::apply_noise(float range)
{
	//for parameter layers
	for (auto& l : parameter_layer_indices)
	{
		layers[l]->apply_noise(range);
	}
	sync_device_and_host();
}

void neural_network::mutate(float range)
{
	if (parameter_layer_indices.empty())
	{
		throw std::runtime_error("Cannot mutate. No parameter layers have been added yet.");
	}
	int layer_idx = parameter_layer_indices[random_idx((int)parameter_layer_indices.size())];
	
	if (is_in_gpu_mode())
	{
		layers[layer_idx]->sync_device_and_host();
	}
	
	layers[layer_idx]->mutate(range);
	
	if (is_in_gpu_mode())
	{
		layers[layer_idx]->sync_device_and_host();
	}
}
/*
test_result neural_network::test(const std::vector<std::unique_ptr<nn_data>>& test_data)
{
	test_result result;
	result.data_count = test_data.size();
	int correct_predictions = 0;
	float cost_sum = 0.0f;
	auto start = std::chrono::high_resolution_clock::now();

	for (auto& curr_data : test_data)
	{
		forward_propagation_cpu(curr_data.get()->get_data());
		if (get_interpreter<interpreter>()->same_result(*get_output(), curr_data.get()->get_label()))
		{
			correct_predictions++;
		}
		cost_sum += calculate_cost(curr_data.get()->get_label());
	}

	auto end = std::chrono::high_resolution_clock::now();

	result.time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	result.accuracy = (float)correct_predictions / (float)result.data_count;
	result.avg_cost = cost_sum / (float)result.data_count;

	return result;
}*/

void neural_network::forward_propagation(const matrix& input)
{
	if (input.is_in_gpu_mode() != is_in_gpu_mode())
	{
		throw std::runtime_error("Input data is not in the same mode as the neural network.");
	}
	matrix* last_layer = nullptr;
	//std::vector<std::unique_ptr<layer>>::iterator::value_type
	for (auto& l : layers)
	{
		l->forward_propagation(
			last_layer == nullptr ?
			input :
			*last_layer
		);
		last_layer = l.get()->get_activations_p();
	}
}

void neural_network::back_propagation(const matrix& given_data, const matrix& given_label)
{
	//feeding the data through
	forward_propagation(given_data);

	//calculating the cost derivative
	//calculate_cost_derivative(training_data->get_label_p());
	get_last_layer()->set_error_for_last_layer(given_label);

	//we start from the last layer
	for (int i = layers.size() - 1; i >= 0; i--)
	{
		const matrix& input =
			i == 0 ?
			given_data :
			layers[i - 1].get()->get_activations_readonly();

		matrix* passing_error =
			i == 0 ?
			nullptr :
			layers[i - 1].get()->get_error_p();

		layers[i].get()->back_propagation(input, passing_error);
	}
}

void neural_network::apply_deltas(size_t training_data_count, float learning_rate)
{
	//iterate over all parameter layers
	for (auto& l : parameter_layer_indices)
	{
		//update the parameters
		layers[l]->apply_deltas(training_data_count, learning_rate);
	}
}
void neural_network::enable_gpu_mode()
{
	int device_count = 0;
	cudaError_t error = cudaGetDeviceCount(&device_count);

	if (error != cudaSuccess)
	{
		throw std::runtime_error("CUDA error: " + std::string(cudaGetErrorString(error)));
	}

	if (device_count == 0)
	{
		throw std::runtime_error("No CUDA capable devices (GPUs) found.");
	}

	for (auto& l : layers)
	{
		l->enable_gpu_mode();
	}

	gpu_enabled = true;

	sync_device_and_host();
}

bool neural_network::is_in_gpu_mode()
{
	return gpu_enabled;
}

bool neural_network::equal_format(const neural_network& other)
{
	if (layers.size() != other.layers.size())
	{
		return false;
	}
	for (int i = 0; i < layers.size(); i++)
	{
		if (!layers[i]->equal_format(*other.layers[i]))
		{
			return false;
		}
	}
	return true;
}

bool neural_network::equal_parameter(const neural_network& other)
{
	if (layers.size() != other.layers.size())
	{
		return false;
	}
	for (int i = 0; i < layers.size(); i++)
	{
		if (!layers[i]->equal_parameter(*other.layers[i]))
		{
			return false;
		}
	}
	return true;
}

void neural_network::set_parameters(const neural_network& other)
{
	if (!equal_format(other))
	{
		throw std::runtime_error("Cannot set parameter. Format of the networks is not equal.");
	}
	for (auto& l : parameter_layer_indices)
	{
		layers[l]->set_parameters(*other.layers[l]);
	}
}

void neural_network::save_to_file(const std::string& file_path)
{
	std::ofstream out(file_path, std::ios::out | std::ios::binary);
	try
	{
		if (!out.is_open())
			throw std::runtime_error("Cannot open file " + file_path);

		out.write((char*)&FILE_MAGIC_NUMBER, sizeof(float));

		input_format.write_to_ofstream(out);

		size_t layer_count = layers.size();
		out.write((char*)&layer_count, sizeof(size_t));

		sync_device_and_host();

		for (auto& l : layers)
		{
			l->write_to_ofstream(out);
		}
	}
	catch (const std::exception& e)
	{
		out.close();
		throw e;
	}
	out.close();
}