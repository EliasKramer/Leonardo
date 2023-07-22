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
	smart_assert(input_format.item_count() == 0); //Cannot set input format twice

	this->input_format = given_input_format;
}

const matrix& neural_network::get_output_readonly() const
{
	smart_assert(layers.empty() == false);
	return layers.back().get()->get_activations_readonly();
}

matrix& neural_network::get_output()
{
	smart_assert(layers.empty() == false);
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
	smart_assert(matrix::equal_format(get_output_readonly(), expected_output));

	float cost = 0.0f;
	for (int i = 0; i < expected_output.item_count(); i++)
	{
		float expected = expected_output.get_at_flat_host(i);
		float actual = get_output_readonly().get_at_flat_host(i);
		cost += ((actual - expected) * (actual - expected));
	}
	return cost;
}

size_t neural_network::idx_of_max(const matrix& m) const
{
	size_t max_idx = 0;
	float max = m.get_at_host(vector3(0, 0));
	for (size_t idx = 1; idx < m.item_count(); idx++)
	{
		float curr = m.get_at_flat_host(idx);
		if (curr > max)
		{
			max = curr;
			max_idx = idx;
		}
	}
	return max_idx;
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
	throw std::runtime_error("not implemented");
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
	smart_assert(parameter_layer_indices.empty() == false);

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

void neural_network::forward_propagation(const matrix& input)
{
	smart_assert(input.is_in_gpu_mode() == is_in_gpu_mode());

	matrix* last_layer = nullptr;
	//std::vector<std::unique_ptr<layer>>::iterator::value_type
	std::lock_guard<std::mutex> lock(forward_mutex);
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

	std::lock_guard<std::mutex> lock(back_mutex);
	//calculating the cost derivative
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

void neural_network::learn_on_ds(
	data_space& ds,
	size_t epochs,
	size_t batch_size,
	float learning_rate,
	bool input_zero_check)
{
	smart_assert(ds.is_in_gpu_mode() == is_in_gpu_mode());
	smart_assert(vector3::are_equal(ds.get_data_format(), input_format));
	smart_assert(vector3::are_equal(ds.get_label_format(), get_output_readonly().get_format()));
	smart_assert(ds.get_item_count() > 0);

	matrix input(ds.get_data_format());
	matrix label(ds.get_label_format());
	if (is_in_gpu_mode())
	{
		input.enable_gpu_mode();
		label.enable_gpu_mode();
	}

	for (size_t curr_epoch = 0; curr_epoch < epochs; curr_epoch++)
	{
		size_t batch_item = 0;
		for (int i = 0; i < ds.get_item_count(); i++)
		{
			ds.observe_data_at_idx(input, i);
			ds.observe_label_at_idx(label, i);

			if (!input_zero_check || input.contains_non_zero_items())
			{
				batch_item++;
				back_propagation(input, label);

				if (batch_item >= batch_size)
				{
					apply_deltas(batch_size, learning_rate);
					batch_item = 0;
				}
			}
		}
		//apply the remaining deltas, that were not applied in the loop
		if (batch_item > 0)
		{
			apply_deltas(batch_item, learning_rate);
			batch_item = 0;
		}
		ds.shuffle();
	}
}
/*size_t mnist_digit_overlord::idx_of_max(const matrix& m) const
{
	size_t max_idx = 0;
	float max = m.get_at_host(vector3(0, 0));
	for (size_t idx = 1; idx < m.item_count(); idx++)
	{
		float curr = m.get_at_flat_host(idx);
		if (curr > max)
		{
			max = curr;
			max_idx = idx;
		}
	}
	return max_idx;
}


float mnist_digit_overlord::get_digit_cost(const matrix& output, const matrix& label) const
{
	float cost = 0;
	for (size_t i = 0; i < output.item_count(); i++)
	{
		cost +=
			(output.get_at_flat_host(i) - label.get_at_flat_host(i)) *
			(output.get_at_flat_host(i) - label.get_at_flat_host(i));
	}
	return cost;
}
*/

test_result neural_network::test_on_ds(data_space& ds)
{
	smart_assert(ds.is_in_gpu_mode() == is_in_gpu_mode());
	test_result result;

	size_t correct = 0;
	float cost_sum = 0;
	size_t total = 0;

	auto start = std::chrono::high_resolution_clock::now();

	matrix input(input_format);
	matrix label(get_output().get_format());
	if (ds.is_in_gpu_mode())
	{
		input.enable_gpu_mode();
		label.enable_gpu_mode();
	}

	for (int i = 0; i < ds.get_item_count(); i++)
	{
		ds.observe_data_at_idx(input, i);
		ds.observe_label_at_idx(label, i);
		forward_propagation(input);
		get_output().sync_device_and_host();

		cost_sum += calculate_cost(label);
		  
		size_t idx = idx_of_max(get_output_readonly());
		size_t label_idx = idx_of_max(label);
		if (idx == label_idx)
		{
			correct++;
		}
		total++;
	}

	auto end = std::chrono::high_resolution_clock::now();

	result.accuracy = (float)correct / (float)total;
	result.data_count = total;
	result.time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	result.avg_cost = (float)cost_sum / (float)total;

	return result;
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
void neural_network::xavier_initialization()
{
	for (int i = 0; i < layers.size(); i++)
	{
		layers[i]->set_all_parameters(0.0f);

		size_t input_size = layers[i]->get_input_format().item_count();
		size_t outputsize = layers[i]->get_activations_readonly().item_count();

		float range = sqrtf(6.0f / ((float)input_size + (float)outputsize));

		layers[i]->apply_noise(range);
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

	cudaSetDevice(0);

	for (auto& l : layers)
	{
		l->enable_gpu_mode();
	}

	gpu_enabled = true;

	sync_device_and_host();
}

bool neural_network::is_in_gpu_mode() const
{
	return gpu_enabled;
}

bool neural_network::nn_equal_format(const neural_network& other)
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
	smart_assert(nn_equal_format(other));
	smart_assert(is_in_gpu_mode() == other.is_in_gpu_mode());

	for (auto& l : parameter_layer_indices)
	{
		layers[l]->set_parameters(*other.layers[l]);
	}
}

std::string neural_network::parameter_analysis() const
{
	//all layers

	std::string result = "Parameter analysis:\n";
	for (auto& l : layers)
	{
		result += l->parameter_analysis();
	}

	return result;
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