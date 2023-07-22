#include "matrix.hpp"
#include <fstream>
#include <numeric>

void matrix::set_host_as_last_updated()
{
	smart_assert(is_initialized());
	smart_assert(!is_in_gpu_mode() || last_updated_data != device_data);

	last_updated_data = host_data;
}

void matrix::set_device_as_last_updated()
{
	smart_assert(is_initialized());
	smart_assert(gpu_enabled);
	smart_assert(last_updated_data != host_data);
	
	last_updated_data = device_data;
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
	if (!is_owning_data())
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
	smart_assert(is_initialized());
	smart_assert(!is_in_gpu_mode());

	cudaMalloc(&device_data, item_count() * sizeof(float));
	if_cuda_error_throw();
	gpu_enabled = true;
}

void matrix::copy_host2device()
{
	smart_assert(is_initialized());
	//smart_assert(is_owning_data()); //copying to a non-owning matrix is allowed - but it is not tested
	smart_assert(is_in_gpu_mode());

	cudaMemcpy(
		device_data,
		host_data,
		item_count() * sizeof(float),
		cudaMemcpyHostToDevice);
	if_cuda_error_throw();

	last_updated_data = nullptr;
}
void matrix::copy_device2host()
{
	smart_assert(is_initialized());
	//smart_assert(is_owning_data());//copying to a non-owning matrix is allowed - but it is not tested
	smart_assert(is_in_gpu_mode());

	cudaMemcpy(
		host_data,
		device_data,
		item_count() * sizeof(float),
		cudaMemcpyDeviceToHost);
	if_cuda_error_throw();

	last_updated_data = nullptr;
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
	smart_assert(host_data == nullptr);
	smart_assert(format_is_valid());
	smart_assert(item_count() > 0);

	host_data = new float[item_count()];
	owning_data = true;
	set_all(0);
}

void matrix::copy_host2host_from(const matrix& src)
{
	smart_assert(is_initialized());
	smart_assert(src.is_initialized());

	smart_assert(is_owning_data());
	smart_assert(equal_format(*this, src));

	std::copy(src.host_data, src.host_data + item_count(), this->host_data);
	set_host_as_last_updated();
}

void matrix::copy_device2device_from(const matrix& src)
{
	smart_assert(is_initialized());
	smart_assert(src.is_initialized());
	
	smart_assert(is_owning_data());
	smart_assert(src.is_owning_data());
	
	smart_assert(is_in_gpu_mode());
	smart_assert(src.is_in_gpu_mode());
	
	smart_assert(equal_format(*this, src));

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
	if (is_owning_data())
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
	smart_assert(is_initialized());
	smart_assert(given_ptr != nullptr);
	smart_assert(depth_idx < get_depth());
	smart_assert(given_ptr == host_data || given_ptr == device_data);

	return sub_ptr<float>(given_ptr, get_width() * get_height(), depth_idx);
}

float* matrix::get_ptr_row(float* given_ptr, size_t height_idx, size_t depth_idx)
{
	smart_assert(height_idx < get_height());
	return get_ptr_layer(given_ptr, depth_idx) + height_idx * get_width();
}

float* matrix::get_ptr_item(float* given_ptr, size_t width_idx, size_t height_idx, size_t depth_idx)
{
	smart_assert(width_idx < get_width());
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
	smart_assert(!given_vector.empty());
	smart_assert(given_vector.size() == item_count());
	smart_assert(is_owning_data()); //if this occurs, the format was not valid
	
	std::copy(given_vector.data(), given_vector.data() + item_count(), this->host_data);

	set_host_as_last_updated();
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
			copy_host2host_from(source);
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

bool matrix::is_initialized() const
{
	return host_data != nullptr && format.item_count() != 0;
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
		copy_host2device();
	}
	else if (last_updated_data == device_data)
	{
		copy_device2host();
	}
	else
	{
		throw std::runtime_error("last_updated_data is not pointing to host or device");
	}
 }

bool matrix::is_device_and_host_synced() const
{
	return last_updated_data == nullptr;
}

bool matrix::device_data_is_updated() const
{
	return last_updated_data == device_data || is_device_and_host_synced();
}

bool matrix::host_data_is_updated() const
{
	return  last_updated_data == host_data || is_device_and_host_synced();
}

matrix& matrix::operator=(const matrix& other)
{
	smart_assert(other.is_initialized());
	//sets this matrix to the value of the other
	//by copying

	if (this != &other) {
		delete_data_if_owning();
		this->format = other.format;

		if (other.is_initialized() &&
			other.format.item_count() != 0)
		{
			allocate_host_mem();
			copy_host2host_from(other);
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

bool matrix::operator!=(const matrix& other) const
{
	return !(*this == other);
}

void matrix::set_data_from_src(const matrix& src)
{
	smart_assert(is_initialized());
	smart_assert(src.is_initialized());
	smart_assert(src.is_owning_data());
	smart_assert(format == src.format);
	smart_assert(is_in_gpu_mode() == src.is_in_gpu_mode());

	if (gpu_enabled)
	{
		copy_device2device_from(src);
	}
	else
	{
		copy_host2host_from(src);
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
		copy_host2device();
	}
}

void matrix::apply_noise(float range)
{
	apply_noise(-range, range);
}

void matrix::apply_noise(float min, float max)
{
	if_not_initialized_throw();

	for (int i = 0; i < item_count(); i++)
	{
		host_data[i] += random_float_excl(min, max);
	}

	if (gpu_enabled)
	{
		copy_host2device();
	}
}

void matrix::mutate(float range)
{
	smart_assert(is_initialized());

	add_at_flat(
		random_idx((int)item_count()),
		random_float_excl(-range, range));

	set_host_as_last_updated();
}

void matrix::write_to_ofstream(std::ofstream& file) const
{
	smart_assert(is_initialized());

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

float matrix::avg_values() const
{
	smart_assert(is_initialized());
	smart_assert(host_data_is_updated());

	float sum = 0.0f;
	for (int i = 0; i < item_count(); i++)
	{
		sum += host_data[i];
	}
	return sum / item_count();
}

float matrix::std_dev() const
{
	smart_assert(is_initialized());
	smart_assert(host_data_is_updated());

	float avg = avg_values();
	float sum = 0.0f;
	for (int i = 0; i < item_count(); i++)
	{
		sum += powf(host_data[i] - avg, 2.0f);
	}
	return sqrtf(sum / item_count());
}

float matrix::max_value() const
{
	smart_assert(is_initialized());
	smart_assert(host_data_is_updated());

	float max = host_data[0];
	for (int i = 1; i < item_count(); i++)
	{
		if (host_data[i] > max)
		{
			max = host_data[i];
		}
	}
	return max;
}

float matrix::min_value() const
{
	smart_assert(is_initialized());
	smart_assert(host_data_is_updated());

	float min = host_data[0];
	for (int i = 1; i < item_count(); i++)
	{
		if (host_data[i] < min)
		{
			min = host_data[i];
		}
	}
	return min;
}

float matrix::percentile(float percentage) const
{
	smart_assert(is_initialized());
	smart_assert(host_data_is_updated());
	smart_assert(percentage >= 0.0f && percentage <= 1.0f);
	
	float* sorted = new float[item_count()];
	std::memcpy(sorted, host_data, sizeof(float) * item_count());
	std::sort(sorted, sorted + item_count());

	float idx_f = percentage * item_count();
	size_t idx = (size_t)idx_f;

	float value = sorted[idx];

	delete[] sorted;

	return value;
}

std::string matrix::analyse_string() const
{
	std::string str = "";
	str += format.to_string() + "\n";
	str += "avg: " + std::to_string(avg_values()) + "\n";
	str += "std_dev: " + std::to_string(std_dev()) + "\n";
	str += "max: " + std::to_string(max_value()) + "\n";
	str += "min: " + std::to_string(min_value()) + "\n";
	str += "percentile(0.25): " + std::to_string(percentile(0.25f)) + "\n";
	str += "percentile(0.5): " + std::to_string(percentile(0.5f)) + "\n";
	str += "percentile(0.75): " + std::to_string(percentile(0.75f)) + "\n";
	return str;
}

float matrix::get_at_flat_host(size_t idx) const
{
	smart_assert(is_initialized());
	smart_assert(idx < item_count());

	return host_data[idx];
}

void matrix::set_at_flat_host(size_t idx, float value)
{
	smart_assert(is_initialized());
	smart_assert(idx < item_count());

	host_data[idx] = value;

	set_host_as_last_updated();
}

void matrix::add_at_flat(size_t idx, float value)
{
	set_at_flat_host(idx, get_at_flat_host(idx) + value);
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
		if (is_owning_data())
		{
			allocate_device_mem();
			copy_host2device();
		}
	}
}
bool matrix::is_in_gpu_mode() const
{
	return gpu_enabled;
}

bool matrix::is_owning_data() const
{
	return owning_data;
}

void matrix::observe_row(matrix& m, size_t row_idx)
{
	observe_row(m, row_idx, 0);
}
void matrix::observe_row(matrix& m, size_t row_idx, size_t item_idx)
{
	smart_assert(m != *this);
	smart_assert(m.is_initialized());
	smart_assert(m.is_in_gpu_mode() == is_in_gpu_mode());
	smart_assert(row_idx < m.get_height());
	smart_assert(item_idx < m.get_width());

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
	
	/*
	if ((m.get_width() - item_count() - item_idx) < 0)
	{
		throw std::invalid_argument("the item count on this matrix must match the amount of items left in the given row");
	}
	*/

	smart_assert((m.get_width() - item_count() - item_idx) >= 0);

	delete_data_if_owning();
	
	host_data = m.get_ptr_item(m.host_data, item_idx, row_idx, 0);

	if (gpu_enabled)
	{
		device_data = m.get_ptr_item(m.device_data, item_idx, row_idx, 0);
	}
	owning_data = false;

	/*
	m.last_updated_data ==
		m.host_data ? set_host_as_last_updated() :
		m.device_data ? set_device_as_last_updated() :
		last_updated_data = nullptr;
	*/
}

void matrix::set_row_from_matrix(const matrix& m, size_t row_idx)
{
	set_row_from_matrix(m, row_idx, 0);
}
void matrix::set_row_from_matrix(const matrix& m, size_t row_idx, size_t item_idx)
{
	smart_assert(m != *this);
	smart_assert(is_initialized());
	smart_assert(m.is_initialized());
	smart_assert(is_owning_data());
	smart_assert(row_idx < get_height());
	smart_assert(item_idx < get_width());
	smart_assert(m.is_in_gpu_mode() == is_in_gpu_mode());

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
	/*
	if ((get_width() - m.item_count() - item_idx) < 0)
	{
		throw std::invalid_argument("the item count on this matrix must match the amount of items left in the given row");
	}
	*/
	smart_assert((get_width() - m.item_count() - item_idx) >= 0);

	if (is_in_gpu_mode())
	{
		float* new_ptr = get_ptr_item(device_data, item_idx, row_idx, 0);

		cudaMemcpy(new_ptr, m.device_data, m.item_count() * sizeof(float), cudaMemcpyDeviceToDevice);
		if_cuda_error_throw();
		set_device_as_last_updated();
	}
	else
	{
		float* new_ptr = get_ptr_item(host_data, item_idx, row_idx, 0);
		memcpy(new_ptr, m.host_data, m.item_count() * sizeof(float));

		set_host_as_last_updated();
	}
}

void matrix::set_at_host(vector3 pos, float value)
{
	smart_assert(is_initialized());
	smart_assert(is_owning_data());
	smart_assert(pos.is_in_bounds(format));

	host_data[pos.get_index(format)] = value;

	set_host_as_last_updated();
}

void matrix::add_at_host(vector3 pos, float value)
{
	set_at_host(pos, get_at_host(pos) + value);
}

float matrix::get_at_host(vector3 pos) const
{
	smart_assert(is_initialized());
	smart_assert(pos.is_in_bounds(format));

	return host_data[pos.get_index(format)];
}

bool matrix::contains_non_zero_items() {
	smart_assert(is_initialized());

	//tried to do this in cuda as well,
	//but it was slower than doing it on the cpu
	sync_device_and_host();
	
	for (int i = 0; i < item_count(); i++)
	{
		if (host_data[i] != 0)
		{
			return true;
		}
	}
	return false;
}

void matrix::dot_product_flat(const matrix& a, const matrix& flat, matrix& result_flat)
{
	smart_assert(a.is_initialized());
	smart_assert(flat.is_initialized());
	smart_assert(result_flat.is_initialized());
	smart_assert(a.get_width() == flat.item_count());
	smart_assert(a.get_height() == result_flat.item_count());
	smart_assert(a.get_depth() == 1); // i think doesnt have to be checked. it would work with depth > 1

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
		result_flat.set_at_flat_host(y, 0);
		for (int x = 0; x < a.get_width(); x++)
		{
			result_flat.add_at_flat(y, a.get_at_host(vector3(x, y)) * flat.get_at_flat_host(x));
		}
	}
	result_flat.set_host_as_last_updated();
}

void matrix::add(const matrix& a, const matrix& b, matrix& result)
{
	smart_assert(a.is_initialized());
	smart_assert(b.is_initialized());
	smart_assert(result.is_initialized());

	smart_assert(a.get_width() == b.get_width());
	smart_assert(a.get_height() == b.get_height());
	smart_assert(a.get_depth() == b.get_depth());
	
	smart_assert(a.get_width() == result.get_width());
	smart_assert(a.get_height() == result.get_height());
	smart_assert(a.get_depth() == result.get_depth());
	
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
	smart_assert(a.is_initialized());
	smart_assert(b.is_initialized());
	smart_assert(result.is_initialized());
	smart_assert(result.is_owning_data());
	smart_assert(a.item_count() == b.item_count());
	smart_assert(a.item_count() == result.item_count());

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
	smart_assert(a.is_initialized());
	smart_assert(b.is_initialized());
	smart_assert(result.is_initialized());
	smart_assert(result.is_owning_data());

	smart_assert(equal_format(a, b));
	smart_assert(equal_format(b, result));

	if (a.gpu_enabled &&
		b.gpu_enabled &&
		result.gpu_enabled)
	{
		gpu_subtract(
			a,
			b,
			result
		);
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
	smart_assert(input.is_initialized());
	smart_assert(output.is_initialized());
	smart_assert(output.is_owning_data());

	smart_assert(input.get_depth() == output.get_depth());
	smart_assert(convolution_output_size(input.get_width(), kernel_size, stride) == output.get_width());

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
					output.set_at_host(vector3(x, y, d), max);
					break;
				case min_pooling:
					output.set_at_host(vector3(x, y, d), min);
					break;
				case average_pooling:
					output.set_at_host(vector3(x, y, d), sum / (kernel_size * kernel_size));
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

void matrix::fully_connected_backprop(
	const matrix& activations,
	const matrix& weights,
	const matrix& input,
	const matrix& error,
	matrix* passing_error,
	matrix& weight_deltas,
	matrix& bias_deltas,
	e_activation_t activation_fn)
{
	smart_assert(activations.is_initialized());
	smart_assert(weights.is_initialized());
	smart_assert(input.is_initialized());
	smart_assert(error.is_initialized());
	smart_assert(weight_deltas.is_initialized());
	smart_assert(bias_deltas.is_initialized());
	
	smart_assert(bias_deltas.is_owning_data());
	smart_assert(weight_deltas.is_owning_data());
	smart_assert(passing_error == nullptr || passing_error->is_initialized());

	smart_assert(matrix::equal_format(activations, error));

	if (activations.is_in_gpu_mode() &&
		weights.is_in_gpu_mode() &&
		input.is_in_gpu_mode() &&
		error.is_in_gpu_mode() &&
		weight_deltas.is_in_gpu_mode() &&
		bias_deltas.is_in_gpu_mode())
	{
		gpu_fc_backprop(
			activations,
			weights,
			input,
			error,
			passing_error,
			weight_deltas,
			bias_deltas,
			activation_fn
		);

		weight_deltas.set_device_as_last_updated();
		bias_deltas.set_device_as_last_updated();
		if (passing_error != nullptr)
			passing_error->set_device_as_last_updated();
		return;
	}

	for (int neuron_idx = 0; neuron_idx < activations.item_count(); neuron_idx++)
	{
		float error_value = error.get_at_flat_host(neuron_idx);

		float unactivated_activation = INVERSE[activation_fn](activations.get_at_flat_host(neuron_idx));
		float activation_derivative = DERIVATIVE[activation_fn](unactivated_activation);

		//bias change
		float bias_change = error_value * activation_derivative;
		bias_deltas.add_at_flat(neuron_idx, bias_change);

		//iterate input layer
		for (int input_idx = 0; input_idx < input.item_count(); input_idx++)
		{
			float input_value = input.get_at_flat_host(input_idx);

			//this weight connects the current input node to the current neuron
			float weight = weights.get_at_host(vector3(input_idx, neuron_idx));

			weight_deltas.add_at_host(
				vector3(input_idx, neuron_idx),
				error_value * activation_derivative * input_value);

			//passing error is null when this is the first layer
			if (passing_error != nullptr)
			{
				passing_error->set_at_flat_host(input_idx, error_value * activation_derivative * weight);
			}
		}
	}

	weight_deltas.set_host_as_last_updated();
	bias_deltas.set_host_as_last_updated();
	if (passing_error != nullptr)
	{
		passing_error->set_host_as_last_updated();
	}
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
	size_t stride)
{
	smart_assert(input.is_initialized());
	smart_assert(output.is_initialized());
	smart_assert(output.is_owning_data());

	smart_assert(kernels.size() > 0);	

	bool use_gpu = true;
	use_gpu = use_gpu && input.gpu_enabled;
	use_gpu = use_gpu && output.gpu_enabled;

	for (const auto& curr_kernel : kernels)
	{
		smart_assert(curr_kernel.is_initialized());
		use_gpu = use_gpu && curr_kernel.gpu_enabled;
	}

	smart_assert(convolution_format_valid(
		input.get_format(),
		kernels[0].get_format(),
		stride,
		output.get_format()));

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
	for (size_t z = 0; z < output.get_depth(); z++)
	{
		for (size_t y = 0; y < output.get_height(); y++)
		{
			for (size_t x = 0; x < output.get_width(); x++)
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
				for (size_t curr_depth = 0; curr_depth < kernels[0].get_depth(); curr_depth++)
				{
					for (size_t i = 0; i < kernel_size; i++)
					{
						for (size_t j = 0; j < kernel_size; j++)
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

void matrix::apply_deltas(
	matrix& delta,
	matrix& momentum,
	size_t training_data_count,
	float learning_rate)
{
	smart_assert(is_initialized());
	smart_assert(is_owning_data());

	smart_assert(delta.is_initialized());
	smart_assert(delta.is_owning_data()); // this must be true, because we set the deltas to zero after applying them
	smart_assert(delta.gpu_enabled == gpu_enabled);

	smart_assert(momentum.is_initialized());
	smart_assert(momentum.is_owning_data());
	smart_assert(momentum.gpu_enabled == gpu_enabled);

	smart_assert(matrix::equal_format(*this, delta));
	smart_assert(matrix::equal_format(delta, momentum));

	if (gpu_enabled)
	{
		gpu_apply_deltas(
			*this,
			delta,
			momentum,
			training_data_count,
			learning_rate
		);
		set_device_as_last_updated();
		delta.set_device_as_last_updated();
		return;
	}
	for (int i = 0; i < item_count(); i++)
	{
		float beta = 0.9f;
		//the delta variable holds the sum of all desired changes. dividing it by the data count will return the average
		float curr_delta = delta.host_data[i] / (float)training_data_count;
		//gradient decent with momentum
		momentum.host_data[i] = beta * momentum.host_data[i] + (1 - beta) * curr_delta;
		host_data[i] -= (momentum.host_data[i] * learning_rate);
		delta.host_data[i] = 0;
	}
	set_host_as_last_updated();
	delta.set_host_as_last_updated();
}

void matrix::scalar_multiplication(float a)
{
	smart_assert(is_initialized());
	smart_assert(is_owning_data());

	if (gpu_enabled)
	{
		gpu_scalar_mult(
			*this,
			a,
			*this
		);
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
	smart_assert(is_initialized());
	smart_assert(is_owning_data());

	if (gpu_enabled)
	{
		gpu_activation_fn(*this, activation_fn);
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
	smart_assert(is_initialized());

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