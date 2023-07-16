#pragma once
#include <vector>
#include <iostream>
#include <string>
#include "util.hpp"
#include "math_functions.hpp"
#include "vector3.hpp"
#include "gpu_math.cuh"
#include "enum_space.hpp"
#include "assert_throw.hpp"

class matrix {
private:
	vector3 format;

	bool owning_data;
	bool gpu_enabled = false;

	float* host_data;
	float* device_data;

	float* last_updated_data = nullptr;

	void set_host_as_last_updated();
	void set_device_as_last_updated();

	void if_not_initialized_throw() const;
	void if_not_owning_throw() const;

	void if_gpu_not_allocated_throw() const;
	void allocate_device_mem();
	void copy_host2device();
	void copy_device2host();

	void if_cuda_error_throw() const;

	bool format_is_valid() const;
	void allocate_host_mem();

	void copy_host2host_from(const matrix& src);
	void copy_device2device_from(const matrix& src);

	void delete_data_if_owning();

	//given_ptr must be either host or device pointer
	float* get_ptr_layer(float* given_ptr, size_t depth_idx);
	float* get_ptr_row(float* given_ptr, size_t height_idx, size_t depth_idx);
	float* get_ptr_item(float* given_ptr, size_t width_idx, size_t height_idx, size_t depth_idx);
public:
	matrix();
	matrix(vector3 given_format);
	matrix(
		vector3 given_format,
		const std::vector<float>& given_vector);
	matrix(const matrix& source, bool copy_values);
	matrix(const matrix& source);
	matrix(std::ifstream& file);

	matrix& operator=(const matrix& other);

	bool operator==(const matrix& other) const;
	bool operator!=(const matrix& other) const;

	~matrix();

	bool is_initialized() const;

	void sync_device_and_host();
	bool is_device_and_host_synced() const;
	bool device_data_is_updated() const;
	bool host_data_is_updated() const;
	void enable_gpu_mode();
	bool is_in_gpu_mode() const;
	bool is_owning_data() const;

	void set_data_from_src(const matrix& src);
	void set_all(float value);
	void apply_noise(float range);
	void apply_noise(float min, float max);
	void mutate(float range);

	void write_to_ofstream(std::ofstream& file) const;

	vector3 get_format() const;
	size_t get_width() const;
	size_t get_height() const;
	size_t get_depth() const;
	size_t item_count() const;

	float avg_values() const;
	float std_dev() const;
	float max_value() const;
	float min_value() const;
	float percentile(float percentage) const;
	std::string analyse_string() const;

	float get_at_flat_host(size_t idx) const;
	void set_at_flat_host(size_t idx, float value);
	void add_at_flat(size_t idx, float value);

	float* get_device_ptr();
	const float* get_device_ptr_readonly() const;
	float* get_device_ptr_layer(size_t depth_idx);

	//the current matrix gets the data from a different matrix row
	//the current matrix must have the same amount of elements as this row
	//the current matrix will not own the data of the other matrix
	void observe_row(matrix& m, size_t row_idx);
	//the current matrix gets the data from a different matrix row at a certain element in this row
	//the current matrix must have the same amount of elements as this row from the given item index on
	//the current matrix will not own the data of the other matrix
	void observe_row(matrix& m, size_t row_idx, size_t item_idx);

	void set_row_from_matrix(const matrix& m, size_t row_idx);
	void set_row_from_matrix(const matrix& m, size_t row_idx, size_t item_idx);

	//setter
	void set_at_host(vector3 position, float value);
	void add_at_host(vector3 position, float value);

	//getter
	float get_at_host(vector3 pos) const;

	//matrix contains at least one item that is not zero
	bool contains_non_zero_items();

	static void dot_product_flat(const matrix& a, const matrix& flat, matrix& result_flat);

	static void add(const matrix& a, const matrix& b, matrix& result);
	static void add_flat(const matrix& a, const matrix& b, matrix& result);

	static void subtract(const matrix& a, const matrix& b, matrix& result);

	static void pooling(
		const matrix& input,
		matrix& output,
		size_t stride,
		size_t kernel_size,
		e_pooling_type_t pooling_type);

	static void fully_connected_backprop(
		const matrix& activations,
		const matrix& weights,
		const matrix& input,
		const matrix& error,
		matrix* passing_error,
		matrix& weight_deltas,
		matrix& bias_deltas,
		e_activation_t activation_fn
	);

	static bool are_equal(const matrix& a, const matrix& b);
	static bool are_equal(const matrix& a, const matrix& b, float tolerance);
	static bool equal_format(const matrix& a, const matrix& b);


	static void cross_correlation(
		const matrix& input,
		const std::vector<matrix>& kernels,
		matrix& output,
		size_t stride);
	//static void valid_convolution(const matrix& input, const matrix& kernel, matrix& output);
	//static void full_cross_correlation(const matrix& input, const matrix& kernel, matrix& output, int stride);
	
	void apply_deltas(
		matrix& delta, 
		matrix& momentum,
		size_t training_data_count, 
		float learning_rate);

	void scalar_multiplication(float a);
	void apply_activation_function(e_activation_t activation_fn);

	std::string get_string() const;
};

//GPU SECTION

//OUTDATED COMMENTS - NEEDS TO BE UPDATED (data types have changed)

/// <summary>
/// performs a dot product on the gpu
/// the input is the vector A
/// the weights are the matrix B
/// the activations are the vector C
/// B has to be have the width of A
/// B has to have the height of C
/// 
/// tho it is only checked if B's size is A's size * C's size
/// since the gpu_memory class stores the data in one dimension
/// </summary>
void gpu_dot_product(
	const matrix& gpu_weights,
	const matrix& gpu_input,
	matrix& gpu_activations);

/// <summary>
/// adds the values of two gpu memory objects
/// these will be stored in the result object
/// //all have to be the same size
/// </summary>
void gpu_add(
	const matrix& gpu_memory_a,
	const matrix& gpu_memory_b,
	matrix& gpu_memory_result);

void gpu_subtract(
	const matrix& gpu_memory_a,
	const matrix& gpu_memory_b,
	matrix& gpu_memory_result);

void gpu_scalar_mult(
	const matrix& gpu_memory_a,
	float scalar,
	matrix& gpu_memory_result);

/// <summary>
/// performs a valid cross correlation
/// this is done by laying the kernels over the input one by one
/// and multiply overlaying values and summing them up
/// then the kernel will be moved by the stride and the process will be repeated
/// 
/// the kernels have to have the same depth as the input
/// the output will have the depth of the amount of kernels that exist
/// </summary>
void gpu_valid_cross_correlation(
	const matrix& gpu_input,
	const std::vector<matrix>& gpu_kernel_weights,
	matrix& gpu_activations,
	size_t input_width,
	size_t input_depth,
	size_t kernel_width,
	size_t kernel_count,
	size_t stride,
	size_t output_width);

void gpu_pooling(
	const matrix& input,
	matrix& output,
	size_t stride,
	size_t kernel_size,
	e_pooling_type_t pooling_type);

void gpu_fc_backprop(
	const matrix& activations,
	const matrix& weights,
	const matrix& input,
	const matrix& error,
	matrix* passing_error,
	matrix& weight_deltas,
	matrix& bias_deltas,
	e_activation_t activation_fn);

void gpu_apply_deltas(
	matrix& a,
	matrix& delta,
	matrix& momentum,
	size_t training_data_count,
	float learning_rate);

/*
	activation functions
	performs a function that has one input and one output
	for example relu where x = max(0, x)
*/
void gpu_activation_fn(matrix& gpu_memory, e_activation_t activation_idx);