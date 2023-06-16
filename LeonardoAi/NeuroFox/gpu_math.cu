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

static void set_device()
{
	cudaError_t cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess)
	{
		throw std::runtime_error("cudaSetDevice failed " + cudaStatus);
	}
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
	if (gpu_weights.get_device_ptr_readonly() == nullptr ||
		gpu_input.get_device_ptr_readonly() == nullptr ||
		gpu_activations.get_device_ptr() == nullptr)
	{
		throw std::invalid_argument("argument is nullptr");
	}

	if (gpu_weights.item_count() == 0 ||
		gpu_input.item_count() == 0 ||
		gpu_activations.item_count() == 0)
	{
		throw std::invalid_argument("gpu_dot_product failed. size must be greater than 0");
	}
	if (gpu_activations.item_count() * gpu_input.item_count() != gpu_weights.item_count())
	{
		throw std::invalid_argument("gpu_dot_product failed. false format");
	}

	set_device();

	unsigned int size = gpu_activations.item_count();
	unsigned int block_count = get_block_count(size);
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
	if (gpu_memory_a.get_device_ptr_readonly() == nullptr ||
		gpu_memory_b.get_device_ptr_readonly() == nullptr ||
		gpu_memory_result.get_device_ptr() == nullptr)
	{
		throw std::invalid_argument("argument is nullptr");
	}

	if (gpu_memory_a.item_count() == 0 ||
		gpu_memory_a.item_count() != gpu_memory_b.item_count() ||
		gpu_memory_a.item_count() != gpu_memory_result.item_count())
	{
		throw std::invalid_argument("gpu_add_matrices failed. size must be greater than 0");
	}

	set_device();

	unsigned int size = gpu_memory_a.item_count();

	gpu_add_matrices_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		gpu_memory_a.get_device_ptr_readonly(),
		gpu_memory_b.get_device_ptr_readonly(),
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
		//print all arguments
		//printf("input_depth: %d, input_width: %d, kernel_width: %d, output_width: %d, stride: %d\n",
		//	input_depth, input_width, kernel_width, output_width, stride);

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
		//printf("result: %f\n", sum);
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
	cuda_error_check();
	//error check has been done before

	set_device();

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
	check_for_error_and_synchronize();
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
	cuda_error_check();
	set_device();

	unsigned int size = output.item_count();
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

__global__ void gpu_sigmoid_kernel(float* data, int size)
{
	unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < size)
	{
		data[index] = 1 / (1 + exp(-data[index]));
	}
}

void gpu_sigmoid(matrix& gpu_memory)
{
	if (gpu_memory.item_count() == 0)
	{
		throw std::invalid_argument("gpu_sigmoid failed. size must be greater than 0");
	}

	set_device();

	unsigned int size = gpu_memory.item_count();
	gpu_sigmoid_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		gpu_memory.get_device_ptr(),
		size);

	check_for_error_and_synchronize();
}

__global__ void gpu_relu_kernel(float* data, int size)
{
	unsigned int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < size)
	{
		data[index] = data[index] > 0 ? data[index] : 0;
	}
}

void gpu_relu(matrix& gpu_memory)
{
	if (gpu_memory.item_count() == 0)
	{
		throw std::invalid_argument("gpu_relu failed. size must be greater than 0");
	}

	set_device();

	unsigned int size = gpu_memory.item_count();
	gpu_relu_kernel << < get_block_count(size), THREADS_PER_BLOCK >> > (
		gpu_memory.get_device_ptr(),
		size);

	check_for_error_and_synchronize();
}