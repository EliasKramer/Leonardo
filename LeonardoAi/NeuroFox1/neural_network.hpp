#pragma once
#include <chrono>
#include "matrix.hpp"
#include "math_functions.hpp"
#include "pooling_layer.hpp"
#include "fully_connected_layer.hpp"
#include "convolutional_layer.hpp"
#include "test_result.hpp"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

class neural_network {
private:
	vector3 input_format;

	std::vector<std::unique_ptr<layer>> layers;
	//saves the indices of all layers tha have parameter
	//convolutional and fully connected 
	std::vector<int> parameter_layer_indices;

	bool gpu_enabled = false;

	layer* get_last_layer();

	void add_layer(std::unique_ptr<layer>&& given_layer);

	float calculate_cost(const matrix& expected_output);

	void sync_device_and_host();
public:

	neural_network();
	neural_network(const std::string& file);
	neural_network(const neural_network& source);

	neural_network& operator=(const neural_network& source);

	//returns the number of parameters (weights and biases) of the nn
	size_t get_param_count() const;
	//returns the number of bytes the nn needs to store all parameters (weights and biases)
	size_t get_param_byte_size() const;

	//sets the input matrix to a certain format
	void set_input_format(vector3 input_format);

	const matrix& get_output_readonly() const;
	matrix& get_output();

	void add_fully_connected_layer(size_t num_neurons, e_activation_t activation_fn);
	void add_fully_connected_layer(vector3 neuron_format, e_activation_t activation_fn);
	
	void add_convolutional_layer(
		size_t number_of_kernels, 
		size_t kernel_size,
		size_t stride,
		e_activation_t activation_fn);
	void add_pooling_layer(size_t kernel_size, size_t stride, e_pooling_type_t pooling_type);

	//set all weights and biases to that value
	void set_all_parameters(float value);
	//a random value to the current weights and biases between -value and value
	void apply_noise(float range);
	//add a random value between range and -range to one weight or bias 
	void mutate(float range);

	//test_result test(const std::vector<std::unique_ptr<nn_data>>& training_data);
	void forward_propagation(const matrix& input);
	
	void back_propagation(const matrix& given_data, const matrix& given_label);

	//we need the training_data_count for 
	//calculating the average of the deltas
	void apply_deltas(size_t training_data_count, float learning_rate);

	void enable_gpu_mode();
	bool is_in_gpu_mode();

	bool equal_format(const neural_network& other);
	bool equal_parameter(const neural_network& other);
	void set_parameters(const neural_network& other);

	//this is not const, because we need to sync the device and host memory before saving
	void save_to_file(const std::string& file_path);
};