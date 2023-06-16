#include "matrix.hpp"
#include <fstream>
#include <numeric>

void matrix::set_host_as_last_updated()
{
	if_not_initialized_throw();
	if (!gpu_enabled)
	{
		last_updated_data = nullptr;
		return;
	}
	if (last_updated_data == device_data)
	{
		throw std::exception("cannot set host as last updated, you have to sync first");
	}
	last_updated_data = host_data;
}

void matrix::set_device_as_last_updated()
{
	if_not_initialized_throw();
	if (!gpu_enabled)
	{
		throw std::exception("no device can set, if gpu is disabled");
	}
	if (last_updated_data == host_data)
	{
		throw std::exception("cannot set device as last updated, you have to sync first");
	}
	last_updated_data = device_data;
}

bool matrix::is_initialized() const
{
	return host_data != nullptr && format.item_count() != 0;
}

void matrix::if_not_initialized_throw() const
{
	if (!is_initialized())
	{
		throw std::runtime_error("matrix not initialized");
	}
}

void matrix::if_not_owning_throw() const
{
	if (!owning_data)
	{
		throw std::runtime_error("matrix not owning data");
	}
}

void matrix::if_gpu_not_allocated_throw() const
{
	if (!gpu_enabled)
	{
		throw std::runtime_error("matrix not using gpu");
	}
}

void matrix::allocate_device_mem()
{
	if_not_initialized_throw();
	if (gpu_enabled)
		throw std::runtime_error("gpu memory already allocated");

	cudaMalloc(&device_data, item_count() * sizeof(float));
	if_cuda_error_throw();
	gpu_enabled = true;
}

void matrix::copy_host_to_device()
{
	if_not_initialized_throw();
	if_gpu_not_allocated_throw();

	cudaMemcpy(
		device_data,
		host_data,
		item_count() * sizeof(float),
		cudaMemcpyHostToDevice);
	if_cuda_error_throw();
}
void matrix::copy_device_to_host()
{
	if_not_initialized_throw();
	if_gpu_not_allocated_throw();

	cudaMemcpy(
		host_data,
		device_data,
		item_count() * sizeof(float),
		cudaMemcpyDeviceToHost);
}

void matrix::if_cuda_error_throw() const
{
	cudaError_t cuda_error = cudaGetLastError();
	if (cuda_error != cudaSuccess)
	{
		std::string err_str = std::string(cudaGetErrorString(cuda_error));
		throw std::runtime_error(
			std::string("cuda error: ") +
			err_str);
	}
}

bool matrix::format_is_valid() const
{
	return  format.item_count() != 0;
}

void matrix::allocate_host_mem()
{
	if (host_data != nullptr)
	{
		throw std::runtime_error("cannot allocate if data will be overwritten");
	}
	if (item_count() == 0)
	{
		throw std::runtime_error("cannot allocate if item count is 0");
	}
	host_data = new float[item_count()];
	owning_data = true;
	set_all(0);
}

void matrix::set_own_host_data_from(const std::vector<float> src)
{
	if (src.empty())
	{
		throw std::runtime_error("cannot set data from empty vector");
	}
	if (src.size() != item_count())
	{
		throw std::runtime_error("cannot set data from vector of different size");
	}
	if (!owning_data)
	{
		throw std::runtime_error("cannot set data if not owning data");
	}
	delete_data_if_owning();
	allocate_host_mem();

	std::copy(src.data(), src.data() + item_count(), this->host_data);

	//not tested 
	set_host_as_last_updated();
}

void matrix::set_own_host_data_from(const matrix& src)
{
	src.if_not_initialized_throw();

	if (host_data == nullptr)
	{
		throw std::runtime_error("cannot set data if no memory is allocated");
	}
	if (!matrix::equal_format(*this, src))
	{
		throw std::runtime_error("cannot copy data from one matrix to another if they are not in the same format");
	}

	delete_data_if_owning();
	allocate_host_mem();

	std::copy(src.host_data, src.host_data + item_count(), this->host_data);

	//not tested 
	set_host_as_last_updated();

}

void matrix::copy_host_data_from(const matrix& src)
{
	if_not_initialized_throw();
	src.if_not_owning_throw();

	if (!matrix::equal_format(*this, src))
	{
		throw std::runtime_error("cannot copy data from one matrix to another if they are not in the same format");
	}

	std::copy(src.host_data, src.host_data + item_count(), this->host_data);
	set_host_as_last_updated();
}

void matrix::copy_device_data_from(const matrix& src)
{
	if_not_initialized_throw();
	src.if_not_owning_throw();
	if_gpu_not_allocated_throw();
	src.if_gpu_not_allocated_throw();
	//we have to own our data - the source does not have to
	if_not_owning_throw();

	if (!matrix::equal_format(*this, src))
	{
		throw std::runtime_error("cannot copy data from one matrix to another if they are not in the same format");
	}

	cudaMemcpy(
		device_data,
		src.device_data,
		item_count() * sizeof(float),
		cudaMemcpyDeviceToDevice);
	if_cuda_error_throw();
	set_device_as_last_updated();
}

void matrix::delete_data_if_owning()
{
	if (owning_data)
	{
		if (host_data != nullptr)
		{
			delete[] host_data;
			host_data = nullptr;
		}
		if (device_data != nullptr)
		{
			cudaFree(device_data);
			if_cuda_error_throw();
			device_data = nullptr;
		}
		owning_data = false;
	}
}

float* matrix::get_ptr_layer(float* given_ptr, size_t depth_idx)
{
	if_not_initialized_throw();
	if (given_ptr == nullptr)
		throw std::invalid_argument("given_ptr is null");
	if (given_ptr != host_data && given_ptr != device_data)
		throw std::invalid_argument("given_ptr is not pointing to this matrix");

	return sub_ptr<float>(given_ptr, get_width() * get_height(), depth_idx);
}

float* matrix::get_ptr_row(float* given_ptr, size_t height_idx, size_t depth_idx)
{
	return get_ptr_layer(given_ptr, depth_idx) + height_idx * get_width();
}

float* matrix::get_ptr_item(float* given_ptr, size_t width_idx, size_t height_idx, size_t depth_idx)
{
	return get_ptr_row(given_ptr, height_idx, depth_idx) + width_idx;
}

matrix::matrix(
) :
	owning_data(false),
	gpu_enabled(false),
	host_data(nullptr),
	device_data(nullptr)
{}

matrix::matrix(
	vector3 given_format
) :
	format(given_format),
	owning_data(true),
	host_data(nullptr),
	device_data(nullptr)
{
	if (format_is_valid())
	{
		allocate_host_mem();
	}
	else
	{
		format = vector3(0, 0, 0);
		owning_data = false;
	}
}

matrix::matrix(
	vector3 given_format,
	const std::vector<float>& given_vector
) :
	matrix(given_format)
{
	set_own_host_data_from(given_vector);
}


matrix::matrix(const matrix& source, bool copy_values)
	:matrix()
{
	if (source.is_initialized())
	{
		this->format = source.format;
		allocate_host_mem();
		if (copy_values)
		{
			set_own_host_data_from(source);
		}
		if (source.gpu_enabled)
		{
			enable_gpu_mode();
		}
	}
}

matrix::matrix(const matrix& source)
	:matrix(source, true)
{}

matrix::matrix(std::ifstream& file)
	:matrix()
{
	if (!file.is_open())
	{
		throw std::runtime_error("cannot read from file, file is not open");
	}
	format = vector3(file);
	if (format_is_valid())
	{
		allocate_host_mem();
		file.read((char*)host_data, sizeof(float) * item_count());
	}
	else
	{
		throw std::runtime_error("invalid format");
	}
}


matrix::~matrix()
{
	delete_data_if_owning();
}

void matrix::sync_device_and_host()
{
	if (last_updated_data == nullptr || !gpu_enabled)
	{
		last_updated_data = nullptr;
		return;
	}
	else if (last_updated_data == host_data)
	{
		copy_host_to_device();
	}
	else if (last_updated_data == device_data)
	{
		copy_device_to_host();
	}
	else
	{
		throw std::runtime_error("last_updated_data is not pointing to host or device");
	}
	last_updated_data = nullptr;
}

matrix& matrix::operator=(const matrix& other)
{
	other.if_not_initialized_throw();
	//sets this matrix to the value of the other
	//by copying

	if (this != &other) {
		delete_data_if_owning();
		this->format = other.format;

		if (other.is_initialized() &&
			other.format.item_count() != 0)
		{
			allocate_host_mem();
			set_own_host_data_from(other);
			if (other.gpu_enabled)
			{
				enable_gpu_mode();
			}
		}
	}
	return *this;
}

bool matrix::operator==(const matrix& other) const
{
	return matrix::are_equal(*this, other);
}


//impl with gpu TODO
void matrix::set_data_from_src(const matrix& src)
{
	if_not_initialized_throw();
	src.if_not_owning_throw();

	if (format != src.format)
	{
		throw std::invalid_argument("src format is not the same as this format");
	}

	if (src.gpu_enabled != gpu_enabled)
	{
		throw std::invalid_argument("src gpu_enabled is not the same as this gpu_enabled");
	}

	if (gpu_enabled)
	{
		copy_device_data_from(src);
	}
	else
	{
		copy_host_data_from(src);
	}
}

void matrix::set_all(float value)
{
	if_not_initialized_throw();

	for (int i = 0; i < item_count(); i++)
	{
		host_data[i] = value;
	}

	if (gpu_enabled)
	{
		copy_host_to_device();
	}
}

void matrix::apply_noise(float range)
{
	if_not_initialized_throw();

	for (int i = 0; i < item_count(); i++)
	{
		host_data[i] += random_float_excl(-range, range);
	}

	if (gpu_enabled)
	{
		copy_host_to_device();
	}
}

void matrix::mutate(float range)
{
	if_not_initialized_throw();

	add_at_flat(
		random_idx((int)item_count()),
		random_float_excl(-range, range));

	set_host_as_last_updated();
}

void matrix::write_to_ofstream(std::ofstream& file) const
{
	if_not_initialized_throw();
	format.write_to_ofstream(file);
	file.write((char*)host_data, sizeof(float) * item_count());
}

vector3 matrix::get_format() const
{
	return format;
}

size_t matrix::get_width() const
{
	return format.x;
}
size_t matrix::get_height() const
{
	return format.y;
}
size_t matrix::get_depth() const
{
	return format.z;
}

size_t matrix::item_count() const
{
	return format.item_count();
}

float matrix::get_at_flat_host(size_t idx) const
{
	if_not_initialized_throw();

	if (idx >= item_count())
	{
		throw std::invalid_argument("idx must be in range");
	}

	return host_data[idx];
}

void matrix::set_at_flat(size_t idx, float value)
{
	if_not_initialized_throw();

	if (idx >= item_count())
	{
		throw std::invalid_argument("idx must be in range");
	}

	host_data[idx] = value;

	set_host_as_last_updated();
}

void matrix::add_at_flat(size_t idx, float value)
{
	set_at_flat(idx, get_at_flat_host(idx) + value);
}

float* matrix::get_device_ptr()
{
	return device_data;
}
const float* matrix::get_device_ptr_readonly() const
{
	return device_data;
}

float* matrix::get_device_ptr_layer(size_t depth_idx)
{
	if_not_initialized_throw();
	if_gpu_not_allocated_throw();

	return get_ptr_layer(device_data, depth_idx);
}

void matrix::enable_gpu_mode()
{
	if_not_initialized_throw();

	if (device_data == nullptr)
	{
		if (owning_data)
		{
			allocate_device_mem();
			copy_host_to_device();
		}
	}
}
bool matrix::is_in_gpu_mode() const
{
	return gpu_enabled;
}
/*
float* matrix::get_data()
{
	return host_data;
}

const float* matrix::get_data_readonly() const
{
	return host_data;
}
*/

void matrix::observe_row(matrix& m, size_t row_idx)
{
	observe_row(m, row_idx, 0);
}
void matrix::observe_row(matrix& m, size_t row_idx, size_t item_idx)
{
	m.if_not_initialized_throw();

	if (row_idx >= m.get_height())
	{
		throw std::invalid_argument("row_idx must be in range");
	}
	if (item_idx >= m.get_width())
	{
		throw std::invalid_argument("item_idx must be in range");
	}
	//if we have a matrix that is currently 3x3 it has an item count of 9
	//lets say the given matrix is 12x12 (the given matrix is named m)
	//we select the current row of the given matrix
	//this row has 12 items
	//in order to set our 3x3 matrix, we need 9 items
	//in a row of 12 items, you have to start at either the 1st, 2nd, 3rd or 4th item
	//otherwise you would use more than 9 items
	// 
	//so the item_count() is 9
	//m.get_width() is 12
	//and now our item index has to be either 0, 1, 2 or 3
	//lets test it. 
	//12 - 9 - 0 = 3 >= 0
	//12 - 9 - 1 = 2 >= 0
	//12 - 9 - 2 = 1 >= 0
	//12 - 9 - 3 = 0 >= 0
	//12 - 9 - 4 = -1 >= 0 (false)
	// 
	//so as soon as
	//12 - 9 - item_idx < 0 we throw an error
	if ((m.get_width() - item_count() - item_idx) < 0)
	{
		throw std::invalid_argument("the item count on this matrix must match the amount of items left in the given row");
	}

	delete_data_if_owning();

	host_data = m.get_ptr_item(m.host_data, item_idx, row_idx, 0);

	if (gpu_enabled)
	{
		device_data = m.get_ptr_item(m.device_data, item_idx, row_idx, 0);
	}
	owning_data = false;
}

void matrix::set_row_from_matrix(const matrix& m, size_t row_idx)
{
	set_row_from_matrix(m, row_idx, 0);
}
void matrix::set_row_from_matrix(const matrix& m, size_t row_idx, size_t item_idx)
{
	m.if_not_initialized_throw();
	if_not_initialized_throw();
	if (!owning_data)
	{
		throw std::invalid_argument("this matrix must own its data");
	}
	if (row_idx >= get_height())
	{
		throw std::invalid_argument("row_idx must be in range");
	}
	if (item_idx >= get_width())
	{
		throw std::invalid_argument("item_idx must be in range");
	}
	//lets say our matrix is 5x5, the item count is 25
	//if our given matrix (named m) is 2x2, the item count is 4
	//we select the current row of our matrix (row idx)
	//this row has 5 items
	//in order to set our 2x2 matrix in this row, we need 4 items
	//in a row of 5 items, you have to start at either the 1st or 2nd item
	//otherwise you would use more than 5 items
	//
	//our 5x5 matrix (the first two rows). (the values in the matrix are the item indices)
	//+--+--+--+--+--+
	//|00|01|02|03|04|
	//+--+--+--+--+--+
	//|05|06|07|08|09|
	//+--+--+--+--+--+
	// 
	//the given matrix looks like this
	//+--+--+
	//|00|01|
	//+--+--+
	//|02|03|
	//+--+--+
	//
	//we want to write the given matrix into the second row of our matrix
	// 
	//+--+--+--+--+
	//|00|01|02|03| (flat representation of the give matrix)
	//+--+--+--+--+
	// |  |  |  |
	// V  V  V  V
	//+--+--+--+--+--+
	//|05|06|07|08|09| (second row of our matrix)
	//+--+--+--+--+--+
	// 
	// as you can see, it works if we start at the 0th item in the row, 
	// but this can also be done starting at the 1st item in the row
	// 
	//	 +--+--+--+--+
	//	 |00|01|02|03| (flat representation of the give matrix)
	//	 +--+--+--+--+
	//	  |  |  |  |
	//	  V  V  V  V
	//+--+--+--+--+--+
	//|05|06|07|08|09| (second row of our matrix)
	//+--+--+--+--+--+
	// 
	//
	//get_width() is 5
	//so the m.item_count() is 4
	//and now our item index has to be either 0 or 1
	//lets test it. 
	// 
	// 5 - 4 - 0 = 1 >= 0
	// 5 - 4 - 1 = 0 >= 0
	// 5 - 4 - 2 = -1 >= 0 (false)
	// 
	//so as soon as
	//12 - 9 - item_idx < 0 we throw an error
	//or in more general terms
	//get_width() - m.item_count() - item_idx < 0

	if ((get_width() - m.item_count() - item_idx) < 0)
	{
		throw std::invalid_argument("the item count on this matrix must match the amount of items left in the given row");
	}

	float* new_ptr = get_ptr_item(host_data, item_idx, row_idx, 0);
	memcpy(new_ptr, m.host_data, m.item_count() * sizeof(float));

	set_host_as_last_updated();
}

void matrix::set_at(vector3 pos, float value)
{
	if_not_initialized_throw();
	if_not_owning_throw();

	if (!pos.is_in_bounds(format))
		throw std::invalid_argument("pos must be in bounds");

	host_data[pos.get_index(format)] = value;

	set_host_as_last_updated();
}

void matrix::add_at_host(vector3 pos, float value)
{
	set_at(pos, get_at_host(pos) + value);
}

float matrix::get_at_host(vector3 pos) const
{
	if_not_initialized_throw();

	if (!pos.is_in_bounds(format))
		throw std::invalid_argument("pos must be in bounds");

	return host_data[pos.get_index(format)];
}

void matrix::dot_product(const matrix& a, const matrix& b, matrix& result)
{
	a.if_not_initialized_throw();
	b.if_not_initialized_throw();
	result.if_not_initialized_throw();

	if (a.get_width() != b.get_height() || a.get_depth() != b.get_depth())
	{
		throw std::invalid_argument("dot product could not be performed. input matrices are in the wrong format");
	}
	if (result.get_width() != b.get_width() || result.get_height() != a.get_height() || result.get_depth() != a.get_depth())
	{
		throw std::invalid_argument("dot product could not be performed. result matrix is not the correct size");
	}

	if (a.gpu_enabled || b.gpu_enabled || result.gpu_enabled)
	{
		throw std::exception("no proper dot product implemented on the gpu. use dot_product_flat");
		result.set_device_as_last_updated();
		return;
	}

	for (int z = 0; z < result.get_depth(); z++)
	{
		for (int y = 0; y < result.get_height(); y++)
		{
			for (int x = 0; x < result.get_width(); x++)
			{
				float sum = 0;
				for (int i = 0; i < a.get_width(); i++)
				{
					sum += a.get_at_host(vector3(i, y, z)) * b.get_at_host(vector3(x, i, z));
				}
				result.set_at(vector3(x, y, z), sum);
			}
		}
	}
	result.set_host_as_last_updated();
}

void matrix::dot_product_flat(const matrix& a, const matrix& flat, matrix& result_flat)
{
	a.if_not_initialized_throw();
	flat.if_not_initialized_throw();
	result_flat.if_not_initialized_throw();

	if (a.get_width() != flat.item_count() ||
		a.get_height() != result_flat.item_count() ||
		a.get_depth() != 1 ||
		result_flat.get_depth() != 1)
	{
		throw std::invalid_argument("dot product could not be performed. input matrices are in the wrong format");
	}

	if (a.gpu_enabled &&
		flat.gpu_enabled &&
		result_flat.gpu_enabled)
	{
		gpu_dot_product(a, flat, result_flat);
		result_flat.set_device_as_last_updated();
		return;
	}

	for (int y = 0; y < a.get_height(); y++)
	{
		result_flat.set_at_flat(y, 0);
		for (int x = 0; x < a.get_width(); x++)
		{
			result_flat.add_at_flat(y, a.get_at_host(vector3(x, y)) * flat.get_at_flat_host(x));
		}
	}
	result_flat.set_host_as_last_updated();
}

void matrix::add(const matrix& a, const matrix& b, matrix& result)
{
	a.if_not_initialized_throw();
	b.if_not_initialized_throw();
	result.if_not_initialized_throw();

	if (a.get_width() != b.get_width() || a.get_height() != b.get_height() || a.get_depth() != b.get_depth())
	{
		throw std::invalid_argument("addition could not be performed. input matrices are in the wrong format");
	}
	if (result.get_width() != a.get_width() || result.get_height() != a.get_height() || result.get_depth() != a.get_depth())
	{
		throw std::invalid_argument("addition could not be performed. result matrix is not the correct size");
	}

	if (a.gpu_enabled &&
		b.gpu_enabled &&
		result.gpu_enabled)
	{
		gpu_add(a, b, result);
		result.set_device_as_last_updated();
		return;
	}

	for (int i = 0; i < a.item_count(); i++)
	{
		result.host_data[i] = a.host_data[i] + b.host_data[i];
	}
	result.set_host_as_last_updated();
}

void matrix::add_flat(const matrix& a, const matrix& b, matrix& result)
{
	a.if_not_initialized_throw();
	b.if_not_initialized_throw();
	result.if_not_initialized_throw();

	if (a.item_count() != b.item_count())
	{
		throw std::invalid_argument("addition could not be performed. input matrices are not the same size");
	}

	if (a.gpu_enabled &&
		b.gpu_enabled &&
		result.gpu_enabled)
	{
		gpu_add(a, b, result);
		result.set_device_as_last_updated();
		return;
	}

	for (int i = 0; i < a.item_count(); i++)
	{
		result.host_data[i] = a.host_data[i] + b.host_data[i];
	}
	result.set_host_as_last_updated();
}

void matrix::subtract(const matrix& a, const matrix& b, matrix& result)
{
	a.if_not_initialized_throw();
	b.if_not_initialized_throw();
	result.if_not_initialized_throw();
	result.if_not_owning_throw();

	if (!equal_format(a, b) ||
		!equal_format(b, result) ||
		!equal_format(result, a))
	{
		throw std::invalid_argument("subtraction could not be performed. input matrices are in the wrong format");
	}

	if (a.gpu_enabled &&
		b.gpu_enabled &&
		result.gpu_enabled)
	{
		//gpu_subtract(a, b, result);
		throw std::exception("gpu subtract not implemented");
		result.set_device_as_last_updated();
		return;
	}

	for (int i = 0; i < a.item_count(); i++)
	{
		result.host_data[i] = a.host_data[i] - b.host_data[i];
	}
	result.set_host_as_last_updated();
}

void matrix::pooling(
	const matrix& input,
	matrix& output,
	size_t stride,
	size_t kernel_size,
	e_pooling_type_t pooling_type)
{
	input.if_not_initialized_throw();
	output.if_not_initialized_throw();
	output.if_not_owning_throw();

	if (input.get_depth() != output.get_depth() ||
		convolution_output_size(input.get_width(), kernel_size, stride) != output.get_width())
	{
		throw std::invalid_argument("pooling could not be performed. input and output matrices are in the wrong format");
	}

	if (input.gpu_enabled &&
		output.gpu_enabled)
	{
		gpu_pooling(input, output, stride, kernel_size, pooling_type);
		output.set_device_as_last_updated();
		return;
	}

	//iterate over each depth
	for (size_t d = 0; d < output.get_depth(); d++)
	{
		//iterate over each row of the output
		for (size_t y = 0; y < output.get_height(); y++)
		{
			//calculate the start and end index of the filter on the y axis
			const size_t start_idx_y = y * stride;
			const size_t end_idx_y = start_idx_y + kernel_size;

			for (size_t x = 0; x < output.get_width(); x++)
			{
				//calculate the start and end index of the filter on the x axis
				const size_t start_idx_x = x * stride;
				const size_t end_idx_x = start_idx_x + kernel_size;

				//calculating the max, min, and average values
				//this could be improved by only calculating one of these values
				float max = FLT_MIN;
				float min = FLT_MAX;
				float sum = 0;

				//iterate over the filter
				for (size_t i = start_idx_y; i <= end_idx_y; i++)
				{
					if (i >= input.get_height())
						break;

					for (size_t j = start_idx_x; j <= end_idx_x; j++)
					{
						if (j >= input.get_width())
							break;

						//get the value of the input at the current index
						const float curr_val = input.get_at_host(vector3(j, i, d));

						//if the current value is greater than the max value
						//set the max value to the current value
						if (curr_val > max)
						{
							max = curr_val;
						}
						if (curr_val < min)
						{
							min = curr_val;
						}
						sum += curr_val;
					}
				}

				switch (pooling_type)
				{
				case max_pooling:
					output.set_at(vector3(x, y, d), max);
					break;
				case min_pooling:
					output.set_at(vector3(x, y, d), min);
					break;
				case average_pooling:
					output.set_at(vector3(x, y, d), sum / (kernel_size * kernel_size));
					break;
				default:
					throw std::runtime_error("Invalid pooling type");
					break;
				}
			}
		}
	}
	output.set_host_as_last_updated();
}

bool matrix::are_equal(const matrix& a, const matrix& b)
{
	return are_equal(a, b, FLOAT_TOLERANCE);
}

bool matrix::are_equal(const matrix& a, const matrix& b, float tolerance)
{
	a.if_not_initialized_throw();
	b.if_not_initialized_throw();

	if (!equal_format(a, b))
	{
		return false;
	}

	for (int i = 0; i < a.item_count(); i++)
	{
		if (std::abs(a.host_data[i] - b.host_data[i]) > tolerance)
		{
			return false;
		}
	}
	return true;
}

bool matrix::equal_format(const matrix& a, const matrix& b)
{
	return vector3::are_equal(a.format, b.format);
}

void matrix::cross_correlation(
	const matrix& input,
	const std::vector<matrix>& kernels,
	matrix& output,
	int stride)
{
	input.if_not_initialized_throw();
	output.if_not_initialized_throw();
	output.if_not_owning_throw();

	bool use_gpu = true;
	use_gpu = use_gpu && input.gpu_enabled;
	use_gpu = use_gpu && output.gpu_enabled;

	for (const auto& curr_kernel : kernels)
	{
		curr_kernel.if_not_initialized_throw();
		use_gpu = use_gpu && curr_kernel.gpu_enabled;
	}

	if (kernels.size() == 0 &&
		!convolution_format_valid(
			input.get_format(),
			kernels[0].get_format(),
			stride,
			output.get_format()))
	{
		throw std::invalid_argument("cross correlation could not be performed. input matrices are in the wrong format");
	}

	output.set_all(0);

	if (use_gpu)
	{
		gpu_valid_cross_correlation(
			input,
			kernels,
			output,
			input.get_width(),
			input.get_depth(),
			kernels[0].get_width(),
			kernels.size(),
			stride,
			output.get_width());

		output.set_device_as_last_updated();
		return;
	}

	size_t kernel_size = kernels[0].get_width();

	//iterate over each
	for (int z = 0; z < output.get_depth(); z++)
	{
		for (int y = 0; y < output.get_height(); y++)
		{
			for (int x = 0; x < output.get_width(); x++)
			{
				//iterate over the kernel and input.
				//the overlaying values are multiplied and added to the output
				//input		kernel
				//+--+--+	+--+--+	  +--+--+
				//|1 |2 |	|2 |3 |	  |2 |6 |
				//+--+--+ * +--+--+ = +--+--+ = 2 + 6 + 12 + 20 = 40 
				//|3 |4 |	|4 |5 |	  |12|20|
				//+--+--+	+--+--+	  +--+--+
				//
				//40 is the sum

				float sum = 0;
				for (int curr_depth = 0; curr_depth < kernels[0].get_depth(); curr_depth++)
				{
					for (int i = 0; i < kernel_size; i++)
					{
						for (int j = 0; j < kernel_size; j++)
						{
							sum +=
								input.get_at_host(
									vector3(
										x * stride + i,
										y * stride + j,
										curr_depth)) *
								kernels[z].get_at_host(
									vector3(
										i,
										j,
										curr_depth));
						}
					}
				}
				//if we do this, the output of all depths will be added together
				output.add_at_host(vector3(x, y, z), sum);
			}
		}
	}

	output.set_host_as_last_updated();
}

void matrix::scalar_multiplication(float a)
{
	if_not_initialized_throw();
	if_not_owning_throw();

	if (gpu_enabled)
	{
		//TODO
		throw std::exception("not implemented");
		set_device_as_last_updated();
		return;
	}


	for (int i = 0; i < item_count(); i++)
	{
		host_data[i] *= a;
	}
	set_host_as_last_updated();
}

void matrix::apply_activation_function(e_activation_t activation_fn)
{
	if_not_initialized_throw();
	if_not_owning_throw();

	if (gpu_enabled)
	{
		//NOT TESTED
		GPU_ACTIVATION[activation_fn](*this);
		set_device_as_last_updated();
		return;
	}

	for (int i = 0; i < item_count(); i++)
	{
		host_data[i] = ACTIVATION[activation_fn](host_data[i]);
	}
	set_host_as_last_updated();
}

std::string matrix::get_string() const
{
	if_not_initialized_throw();

	std::string ret_val = "";

	for (int z = 0; z < get_depth(); z++)
	{
		for (int y = 0; y < get_height(); y++)
		{
			for (int x = 0; x < get_width(); x++)
			{
				ret_val += std::to_string(get_at_host(vector3(x, y, z))) + " ";
			}
			ret_val += "\n";
		}
		ret_val += "\n";
	}

	return ret_val;
}
