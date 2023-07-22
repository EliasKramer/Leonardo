#include "gpu_math.cuh"

void gpu_dot_product(
	const matrix& gpu_weights,
	const matrix& gpu_input,
	matrix& gpu_activations)
{/*
	smart_assert((gpu_weights.get_device_ptr_readonly() != nullptr));
	smart_assert((gpu_input.get_device_ptr_readonly() != nullptr));
	smart_assert((gpu_activations.get_device_ptr() != nullptr));

	smart_assert(gpu_weights.item_count() != 0);
	smart_assert(gpu_input.item_count() != 0);
	smart_assert(gpu_activations.item_count() != 0);

	smart_assert(gpu_activations.item_count() * gpu_input.item_count() == gpu_weights.item_count());


	unsigned int size = gpu_activations.item_count();
	unsigned int block_count = get_block_count(size);
	cuda_sync();
	gpu_dot_product_kernel << < block_count, THREADS_PER_BLOCK >> > (
		gpu_weights.get_device_ptr_readonly(),
		gpu_input.get_device_ptr_readonly(),
		gpu_input.item_count(),
		gpu_activations.get_device_ptr(),
		gpu_activations.item_count());

	check_for_error_and_synchronize();*/
}

void gpu_add(
	const matrix& gpu_memory_a,
	const matrix& gpu_memory_b,
	matrix& gpu_memory_result)
{/*
	smart_assert((gpu_memory_a.get_device_ptr_readonly() != nullptr));
	smart_assert((gpu_memory_b.get_device_ptr_readonly() != nullptr));
	smart_assert((gpu_memory_result.get_device_ptr() != nullptr));

	smart_assert((gpu_memory_a.item_count() != 0));
	smart_assert((gpu_memory_a.item_count() == gpu_memory_b.item_count()));
	smart_assert((gpu_memory_a.item_count() == gpu_memory_result.item_count()));

	unsigned int size = gpu_memory_a.item_count();

	cuda_sync();
	gpu_add_matrices_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		gpu_memory_a.get_device_ptr_readonly(),
		gpu_memory_b.get_device_ptr_readonly(),
		gpu_memory_result.get_device_ptr(),
		size);

	check_for_error_and_synchronize();*/
}

void gpu_subtract(
	const matrix& gpu_memory_a,
	const matrix& gpu_memory_b,
	matrix& gpu_memory_result)
{/*
	smart_assert((gpu_memory_a.get_device_ptr_readonly() != nullptr));
	smart_assert((gpu_memory_b.get_device_ptr_readonly() != nullptr));
	smart_assert((gpu_memory_result.get_device_ptr() != nullptr));

	unsigned int size = gpu_memory_a.item_count();

	cuda_sync();
	gpu_subtract_matrices_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		gpu_memory_a.get_device_ptr_readonly(),
		gpu_memory_b.get_device_ptr_readonly(),
		gpu_memory_result.get_device_ptr(),
		size);

	check_for_error_and_synchronize();*/
}


void gpu_scalar_mult(
	const matrix& gpu_memory_a,
	float scalar,
	matrix& gpu_memory_result)
{/*
	smart_assert((gpu_memory_a.get_device_ptr_readonly() != nullptr));
	smart_assert((gpu_memory_result.get_device_ptr() != nullptr));

	unsigned int size = gpu_memory_a.item_count();

	cuda_sync();
	gpu_scalar_mult_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		gpu_memory_a.get_device_ptr_readonly(),
		scalar,
		gpu_memory_result.get_device_ptr(),
		size);

	check_for_error_and_synchronize();*/
}

void gpu_valid_cross_correlation(
	const matrix& gpu_input,
	const std::vector<matrix>& gpu_kernel_weights,
	matrix& gpu_activations,
	size_t input_width,
	size_t input_depth,
	size_t kernel_width,
	size_t kernel_count,
	size_t stride,
	size_t output_width)
{/*
	smart_assert((gpu_input.get_device_ptr_readonly() != nullptr));
	smart_assert((gpu_activations.get_device_ptr() != nullptr));

	cuda_sync();
	for (int activation_depth = 0; activation_depth < kernel_count; activation_depth++)
	{
		//splits the gpu_activations into each depth layer
		//if the activations have a depth of 3 this loop will iterate 3 times
		//float* activation_ptr = gpu_sub_ptr(gpu_activations.get_device_ptr(), output_width * output_width, activation_depth);

		size_t block_count = get_block_count(output_width * output_width);

		gpu_valid_cross_correlation_kernel << <(int)block_count, THREADS_PER_BLOCK >> > (
			gpu_input.get_device_ptr_readonly(),
			gpu_kernel_weights[activation_depth].get_device_ptr_readonly(),
			gpu_activations.get_device_ptr_layer(activation_depth),
			(int)input_depth,
			(int)input_width,
			(int)kernel_width,
			(int)output_width,
			(int)stride);
		check_for_error_and_synchronize();
	}*/
}

void gpu_pooling(
	const matrix& input,
	matrix& output,
	size_t stride,
	size_t kernel_size,
	e_pooling_type_t pooling_type)
{/*
	smart_assert((input.get_device_ptr_readonly() != nullptr));
	smart_assert((output.get_device_ptr() != nullptr));


	unsigned int size = output.item_count();
	cuda_sync();
	pooling_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		input.get_device_ptr_readonly(),
		output.get_device_ptr(),
		(int)input.get_width(),
		(int)output.get_width(),
		(int)input.get_depth(), //must be same as output
		(int)kernel_size,
		(int)stride,
		(int)pooling_type);

	check_for_error_and_synchronize();*/
}

void gpu_fc_backprop(
	const matrix& activations,
	const matrix& weights,
	const matrix& input,
	const matrix& error,
	matrix* passing_error,
	matrix& weight_deltas,
	matrix& bias_deltas,
	e_activation_t activation_fn)
{/*
	smart_assert((activations.get_device_ptr_readonly() != nullptr));
	smart_assert((weights.get_device_ptr_readonly() != nullptr));
	smart_assert((input.get_device_ptr_readonly() != nullptr));
	smart_assert((error.get_device_ptr_readonly() != nullptr));
	smart_assert((weight_deltas.get_device_ptr() != nullptr));
	smart_assert((bias_deltas.get_device_ptr() != nullptr));

	unsigned int size = activations.item_count();

	cuda_sync();
	gpu_fc_backprop_kernel << <get_block_count(size), THREADS_PER_BLOCK >> > (
		activations.get_device_ptr_readonly(),
		weights.get_device_ptr_readonly(),
		input.get_device_ptr_readonly(),
		error.get_device_ptr_readonly(),
		passing_error == nullptr ? nullptr : passing_error->get_device_ptr(),
		weight_deltas.get_device_ptr(),
		bias_deltas.get_device_ptr(),
		activation_fn,
		size,
		input.item_count());

	check_for_error_and_synchronize();*/
}


void gpu_apply_deltas(
	matrix& a,
	matrix& delta,
	matrix& momentum,
	size_t training_data_count,
	float learning_rate)
{/*
	smart_assert((a.get_device_ptr() != nullptr));
	smart_assert((delta.get_device_ptr() != nullptr));
	smart_assert((momentum.get_device_ptr() != nullptr));

	unsigned int size = a.item_count();
	cuda_sync();
	gpu_apply_deltas_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		a.get_device_ptr(),
		delta.get_device_ptr(),
		momentum.get_device_ptr(),
		training_data_count,
		learning_rate,
		a.item_count()
		);
	check_for_error_and_synchronize();*/
}


void gpu_activation_fn(
	matrix& gpu_memory,
	e_activation_t activation_idx)
{
	/*
	smart_assert((gpu_memory.get_device_ptr() != nullptr));
	smart_assert(gpu_memory.item_count() > 0);

	unsigned int size = gpu_memory.item_count();
	cuda_sync();
	gpu_activation_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		gpu_memory.get_device_ptr(),
		size,
		(int)activation_idx);

	check_for_error_and_synchronize();
	*/
}