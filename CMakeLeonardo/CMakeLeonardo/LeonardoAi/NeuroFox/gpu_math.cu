#include "gpu_math.cuh"

#define THREADS_PER_BLOCK 1024

static unsigned int get_block_count(unsigned int size)
{
	//if we have 1024 elements, we need 1 block
	//if we have 1025 elements, we need 2 blocks
	//if we have 2048 elements, we need 2 blocks
	//and as long as it is under 1024 - 1 thread will still work
	return ((size - 1) / THREADS_PER_BLOCK) + 1;
}

static void cuda_error_check()
{
	cudaError_t cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess)
	{
		std::string cuda_status = cudaGetErrorString(cudaStatus);
		throw std::runtime_error("error while executing cuda kernel cuda status:" + cuda_status);
	}
}
static void cuda_sync()
{
	cudaError_t cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess)
	{
		std::string cuda_status = cudaGetErrorString(cudaStatus);
		throw std::runtime_error("could not sync cuda device cuda status:" + cuda_status);
	}
}

static void check_for_error_and_synchronize()
{
	cuda_error_check();
	cuda_sync();
}

__device__ int get_idx(int x, int y, int z, int height, int width)
{
	return x + y * width + z * width * height;
}

__device__ int get_z(int idx, int height, int width)
{
	return idx / (width * height);
}

__device__ int get_y(int idx, int height, int width)
{
	return (idx - get_z(idx, height, width) * width * height) / width;
}

__device__ int get_x(int idx, int height, int width)
{
	return idx - get_z(idx, height, width) * width * height - get_y(idx, height, width) * width;
}

__device__ float gpu_single_sigmoid(float x)
{
	return 1 / (1 + exp(-x));
}

__device__ float gpu_single_relu(float x)
{
	return x > 0 ? x : 0;
}

__device__ float gpu_single_leaky_relu(float x)
{
	return x > 0 ? x : LEAKY_RELU_FACTOR * x;
}

//not clean, but it has to do for now
__device__ float gpu_single_activation(float x, int function_idx)
{
	if (function_idx == 0)
	{
		return gpu_single_sigmoid(x);
	}
	else if (function_idx == 1)
	{
		return gpu_single_relu(x);
	}
	else if (function_idx == 2)
	{
		return gpu_single_leaky_relu(x);
	}
	else
	{
		printf("single_activation not implemented");
		return 0;
	}
}

__device__ float gpu_single_sigmoid_derivative(float x)
{
	float sigmoid = gpu_single_sigmoid(x);
	return sigmoid * (1 - sigmoid);
}

__device__ float gpu_single_relu_derivative(float x)
{
	return x > 0 ? 1 : 0;
}

__device__ float gpu_single_leaky_relu_derivative(float x)
{
	return x > 0 ? 1 : LEAKY_RELU_FACTOR;
}

//not clean, but it has to do for now
__device__ float gpu_single_derivative(float x, int function_idx)
{
	if (function_idx == 0)
	{
		return gpu_single_sigmoid_derivative(x);
	}
	else if (function_idx == 1)
	{
		return gpu_single_relu_derivative(x);
	}
	else if (function_idx == 2)
	{
		return gpu_single_leaky_relu_derivative(x);
	}
	else
	{
		printf("single_derivative not implemented");
		return 0;
	}
}

__device__ float gpu_single_sigmoid_inverse(float x)
{
	return log(x / (1 - x));
}

__device__ float gpu_single_relu_inverse(float x)
{
	printf("gpu_single_relu_inverse not implemented");
	return x;
}

__device__ float gpu_single_leaky_relu_inverse(float x)
{
	return x > 0 ? x : x / LEAKY_RELU_FACTOR;
}

//not clean, but it has to do for now
__device__ float gpu_single_inverse(float x, int function_idx)
{
	if (function_idx == 0)
	{
		return gpu_single_sigmoid_inverse(x);
	}
	else if (function_idx == 1)
	{
		return gpu_single_relu_inverse(x);
	}
	else if (function_idx == 2)
	{
		return gpu_single_leaky_relu_inverse(x);
	}
	else
	{
		printf("single_inverse not implemented");
		return 0;
	}
}

__global__ void gpu_dot_product_kernel(
	const float* weights,
	const float* input,
	const int input_size,
	float* activations,
	const int activations_size)
{
	unsigned int activation_idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (activation_idx < activations_size)
	{
		float sum = 0;
		for (int i = 0; i < input_size; i++)
		{
			int weight_idx = get_idx(i, activation_idx, 0,
				activations_size, input_size);
			sum += weights[weight_idx] * input[i];
		}
		activations[activation_idx] = sum;
	}
}

void gpu_dot_product(
	const matrix& gpu_weights,
	const matrix& gpu_input,
	matrix& gpu_activations)
{
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

	check_for_error_and_synchronize();
}

__global__ void gpu_add_matrices_kernel(const float* a, const float* b, float* result, unsigned int size)
{
	unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < size)
	{
		result[index] = a[index] + b[index];
	}
}
void gpu_add(
	const matrix& gpu_memory_a,
	const matrix& gpu_memory_b,
	matrix& gpu_memory_result)
{
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

	check_for_error_and_synchronize();
}

__global__ void gpu_subtract_matrices_kernel(const float* a, const float* b, float* result, unsigned int size)
{
	unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < size)
	{
		result[index] = a[index] - b[index];
	}
}

void gpu_subtract(
	const matrix& gpu_memory_a,
	const matrix& gpu_memory_b,
	matrix& gpu_memory_result)
{
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

	check_for_error_and_synchronize();
}


__global__ void gpu_scalar_mult_kernel(const float* a, float scalar, float* result, unsigned int size)
{
	unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < size)
	{
		result[index] = a[index] * scalar;
	}
}

void gpu_scalar_mult(
	const matrix& gpu_memory_a,
	float scalar,
	matrix& gpu_memory_result)
{
	smart_assert((gpu_memory_a.get_device_ptr_readonly() != nullptr));
	smart_assert((gpu_memory_result.get_device_ptr() != nullptr));

	unsigned int size = gpu_memory_a.item_count();

	cuda_sync();
	gpu_scalar_mult_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		gpu_memory_a.get_device_ptr_readonly(),
		scalar,
		gpu_memory_result.get_device_ptr(),
		size);

	check_for_error_and_synchronize();
}

__global__ void gpu_valid_cross_correlation_kernel(
	const float* input,
	const float* weights,
	float* result,
	const int input_depth,
	const int input_width,
	const int kernel_width,
	const int output_width,
	const int stride)
{
	unsigned int result_idx = blockIdx.x * blockDim.x + threadIdx.x;

	if (result_idx < output_width * output_width)
	{
		int input_x = (result_idx % output_width) * stride;
		int input_y = (result_idx / output_width) * stride;

		float sum = 0;
		for (int kernel_x = 0; kernel_x < kernel_width; kernel_x++)
		{
			for (int kernel_y = 0; kernel_y < kernel_width; kernel_y++)
			{
				for (int kernel_z = 0; kernel_z < input_depth; kernel_z++)
				{
					int input_idx = get_idx(input_x + kernel_x, input_y + kernel_y, kernel_z, input_width, input_width);
					int weight_idx = get_idx(kernel_x, kernel_y, kernel_z, kernel_width, kernel_width);
					sum += input[input_idx] * weights[weight_idx];
				}
			}
		}
		result[result_idx] = sum;
	}
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
{
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
	}
}

__global__ void pooling_kernel(
	const float* input,
	float* output,
	const int input_width,
	const int output_width,
	const int depth,
	const int kernel_size,
	const int stride,
	const int pooling_type)
{
	unsigned int result_idx = blockIdx.x * blockDim.x + threadIdx.x;

	if (result_idx < output_width * output_width * depth)
	{
		int x = get_x(result_idx, output_width, output_width);
		int y = get_y(result_idx, output_width, output_width);
		int z = get_z(result_idx, output_width, output_width);

		int input_x = x * stride;
		int input_y = y * stride;

		float min = FLT_MAX;
		float max = FLT_MIN;
		float sum = 0;

		for (int kernel_x = 0; kernel_x < kernel_size; kernel_x++)
		{
			for (int kernel_y = 0; kernel_y < kernel_size; kernel_y++)
			{
				int input_idx = get_idx(input_x + kernel_x, input_y + kernel_y, z, input_width, input_width);
				float value = input[input_idx];

				if (value < min)
				{
					min = value;
				}
				if (value > max)
				{
					max = value;
				}
				sum += value;
			}
		}

		float result = 0;
		/*
		copied from the enum - TODO find a way to use the enum directly
		max_pooling = 0,
		min_pooling = 1,
		average_pooling = 2
		*/
		switch (pooling_type)
		{
		case 0:
			result = max;
			break;
		case 1:
			result = min;
			break;
		case 2:
			result = sum / (kernel_size * kernel_size);
			break;
		}

		output[result_idx] = result;
	}
}

void gpu_pooling(
	const matrix& input,
	matrix& output,
	size_t stride,
	size_t kernel_size,
	e_pooling_type_t pooling_type)
{
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

	check_for_error_and_synchronize();
}

__global__ void gpu_fc_backprop_kernel(
	const float* activations,
	const float* weights,
	const float* input,
	const float* error,
	float* passing_error,
	float* weight_deltas,
	float* bias_deltas,
	e_activation_t activation_fn,
	const unsigned int activation_count,
	const unsigned int input_count
)
{
	unsigned int neuron_idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (neuron_idx < activation_count)
	{
		float error_value = error[neuron_idx];

		float unactivated_activation = gpu_single_inverse(activations[neuron_idx], activation_fn);
		float activation_derivative = gpu_single_derivative(unactivated_activation, activation_fn);

		//bias change
		float bias_change = error_value * activation_derivative;
		bias_deltas[neuron_idx] += bias_change;

		//iterate input layer
		for (int input_idx = 0; input_idx < input_count; input_idx++)
		{
			float input_value = input[input_idx];

			int weight_idx = get_idx(input_idx, neuron_idx, 0, activation_count, input_count);

			float weight = weights[weight_idx]; // could be moved in if statement

			weight_deltas[weight_idx] += (error_value * activation_derivative * input_value);

			if (passing_error != nullptr)
			{
				passing_error[input_idx] = (error_value * activation_derivative * weight);
			}
		}
	}
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
{
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

	check_for_error_and_synchronize();
}

__global__ void gpu_apply_deltas_kernel(
	float* a,
	float* delta,
	float* momentum,
	int training_data_count,
	float learning_rate,
	unsigned int size
)
{
	unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < size)
	{
		float beta = 0.9f;
		float curr_delta = delta[index] / (float)training_data_count;
		momentum[index] = beta * momentum[index] + (1 - beta) * curr_delta;

		a[index] -= (momentum[index] * learning_rate);
		delta[index] = 0;
	}
}

void gpu_apply_deltas(
	matrix& a,
	matrix& delta,
	matrix& momentum,
	size_t training_data_count,
	float learning_rate)
{
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
	check_for_error_and_synchronize();
}

__global__ void gpu_activation_kernel(float* data, unsigned int size, int activation_idx)
{
	unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < size)
	{
		data[index] = gpu_single_activation(data[index], activation_idx);
	}
}

void gpu_activation_fn(
	matrix& gpu_memory,
	e_activation_t activation_idx)
{
	smart_assert((gpu_memory.get_device_ptr() != nullptr));
	smart_assert(gpu_memory.item_count() > 0);

	unsigned int size = gpu_memory.item_count();
	cuda_sync();
	gpu_activation_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		gpu_memory.get_device_ptr(),
		size,
		(int)activation_idx);

	check_for_error_and_synchronize();
}