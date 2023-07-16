#pragma once
#include "matrix.hpp"
#include "math_functions.hpp"
#include "layer.hpp"
#include <memory>
#include <vector>

class convolutional_layer : public layer {
private:
	std::vector<matrix> kernel_weights;
	matrix kernel_biases;
	std::vector<matrix> kernel_weights_deltas;
	matrix kernel_bias_deltas;

	size_t kernel_size;
	size_t stride;
	size_t kernel_count;

	e_activation_t activation_fn;
public:
	convolutional_layer(
		size_t number_of_kernels,
		size_t kernel_size,
		size_t stride,
		e_activation_t activation_function
	);

	convolutional_layer(std::ifstream& file);

	convolutional_layer(const convolutional_layer& other);

	std::unique_ptr<layer> clone() const override;

	size_t get_parameter_count() const override;

	//getters
	size_t get_kernel_size() const;
	size_t get_stride() const;
	size_t get_kernel_count() const;

	std::vector<matrix>& get_kernel_weights();
	const std::vector<matrix>& get_kernel_weights_readonly() const;
	matrix& get_kernel_biases();
	const matrix& get_kernel_biases_readonly() const;

	void set_input_format(vector3 input_format) override;

	//set all weights and biases to that value
	void set_all_parameters(float value) override;
	//a random value to the current weights and biases between -value and value
	void apply_noise(float range) override;
	//add a random value between range and -range to one weight or bias 
	void mutate(float range) override;

	std::string parameter_analysis() const override;

	void sync_device_and_host() override;

	void forward_propagation(const matrix& input) override;
	void back_propagation(const matrix& input, matrix* passing_error) override;

	void apply_deltas(size_t training_data_count, float learning_rate) override;

	void enable_gpu_mode() override;
	void disable_gpu() override;

	bool equal_format(const layer& other) override;
	bool equal_parameter(const layer& other) override;
	void set_parameters(const layer& other) override;

	void write_to_ofstream(std::ofstream& file) const override;
};