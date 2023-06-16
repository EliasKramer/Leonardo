#pragma once
#include "matrix.hpp"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <fstream>

class layer {

private:
	void valid_input_check(const matrix& input) const;
	void valid_passing_error_check_cpu(const matrix* passing_error) const;
protected:
	e_layer_type_t type;

	//the current matrix of neurons
	matrix activations;
	//the current error - the same format as the activations
	matrix error;

	vector3 input_format;

	layer(std::ifstream& file, e_layer_type_t given_type);

public:
	layer(e_layer_type_t given_layer_type);
	layer(vector3 activation_format, e_layer_type_t given_layer_type);

	//copy
	layer(const layer& other);
	//clone
	virtual std::unique_ptr<layer> clone() const = 0;
	
	const e_layer_type_t get_layer_type() const;

	virtual size_t get_parameter_count() const = 0;
	size_t get_param_byte_size() const;

	virtual void set_input_format(vector3 given_input_format);
	
	const matrix& get_activations_readonly() const;
	matrix& get_activations();
	matrix* get_activations_p();

	const matrix& get_error() const;
	matrix* get_error_p();

	//TODO - make this separate
	virtual void set_error_for_last_layer(const matrix& expected);

	//set all weights and biases to that value
	virtual void set_all_parameters(float value) = 0;
	//a random value to the current weights and biases between -value and value
	virtual void apply_noise(float range) = 0;
	//add a random value between range and -range to one weight or bias 
	virtual void mutate(float range) = 0;

	virtual void sync_device_and_host();

	virtual void forward_propagation(const matrix& input);
	virtual	void back_propagation(const matrix& input, matrix* passing_error);

	//the deltas got calculated in the backprop function
	//all the deltas got summed up. now we need to apply the
	//average. this is done by dividing the deltas by the number of inputs
	virtual void apply_deltas(size_t training_data_count, float learning_rate) = 0;

	virtual void enable_gpu_mode();
	virtual void disable_gpu();

	virtual bool equal_format(const layer& other);
	virtual bool equal_parameter(const layer& other) = 0;
	virtual void set_parameters(const layer& other) = 0;

	virtual void write_to_ofstream(std::ofstream& file) const;
};