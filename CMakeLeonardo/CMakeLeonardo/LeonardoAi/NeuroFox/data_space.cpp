#include "data_space.hpp"

size_t data_space::label_item_count()
{
	return label_format.item_count();
}

size_t data_space::data_item_count()
{
	return data_format.item_count();
}

void data_space::set_data_in_table_at(const matrix& m, size_t idx)
{
	smart_assert(vector3::are_equal(data_format, m.get_format()));
	data_table.set_row_from_matrix(m, idx);
}

void data_space::set_label_in_table_at(const matrix& m, size_t idx)
{
	smart_assert(vector3::are_equal(label_format, m.get_format()));
	data_table.set_row_from_matrix(m, idx, data_item_count());
}

void data_space::allocate_data_table()
{
	data_table = matrix(
		vector3(
			(size_t)(data_item_count() + label_item_count()),
			item_count,
			(size_t)1));
	data_table.set_all(0);
}

void data_space::init_shuffle_table()
{
	smart_assert(is_initialized());
	shuffle_table.clear();
	shuffle_table.resize(item_count);
	for (size_t i = 0; i < item_count; i++)
	{
		shuffle_table[i] = i;
	}
}

data_space::data_space()
{}

data_space::data_space(
	size_t given_item_count,
	vector3 data_format
) :
	data_space(
		given_item_count,
		data_format,
		vector3(0, 0, 0))
{}

data_space::data_space(
	size_t given_item_count,
	vector3 data_format,
	vector3 label_format
) :
	data_format(data_format),
	label_format(label_format),
	item_count(given_item_count)
{
	allocate_data_table();
	init_shuffle_table();
}

data_space::data_space(
	vector3 data_format,
	vector3 label_format,
	const std::vector<matrix>& given_data,
	const std::vector<matrix>& given_label
) :
	data_space(
		given_data.size(),
		data_format,
		label_format)
{
	if (given_data.size() != given_label.size())
	{
		throw std::runtime_error("data and label size mismatch");
	}

	for (size_t i = 0; i < given_data.size(); i++)
	{
		set_data_in_table_at(given_data[i], i);
		set_label_in_table_at(given_label[i], i);
	}
}

data_space::data_space(
	vector3 data_format,
	const std::vector<matrix>& given_data
) :
	data_space(
		given_data.size(),
		data_format)
{
	for (size_t i = 0; i < given_data.size(); i++)
	{
		set_data_in_table_at(given_data[i], i);
	}
}

data_space& data_space::operator=(const data_space& other)
{
	if (this != &other)
	{
		shuffle_table = other.shuffle_table;
		data_table = other.data_table;
		data_format = other.data_format;
		label_format = other.label_format;
		item_count = other.item_count;
	}
	return *this;
}

bool data_space::is_in_gpu_mode() const
{
	return data_table.is_in_gpu_mode();
}

bool data_space::is_initialized() const
{
	return data_table.item_count() != 0;
}

size_t data_space::get_item_count() const
{
	return item_count;
}

vector3 data_space::get_data_format() const
{
	return data_format;
}

vector3 data_space::get_label_format() const
{
	return label_format;
}

void data_space::shuffle()
{
	smart_assert(is_initialized());

	std::random_device rd;
	std::mt19937 generator(rd());
	std::shuffle(shuffle_table.begin(), shuffle_table.end(), generator);
}

size_t data_space::byte_size() const
{
	smart_assert(is_initialized());
	return data_table.item_count() * sizeof(float);
}

void data_space::observe_data_at_idx(matrix& observer_matrix, size_t idx)
{
	smart_assert(is_initialized());
	smart_assert(observer_matrix.is_initialized());
	smart_assert(idx < item_count);
	smart_assert(vector3::are_equal(observer_matrix.get_format(), data_format));

	//if the observer matrix is owning a matrix, then it gets deleted
	//it also handles gpu mode

	std::lock_guard<std::mutex> lock(data_mutex); //this could be improved, because more than one thread can read at the same time
	observer_matrix.observe_row(data_table, shuffle_table[idx]);
}

void data_space::observe_label_at_idx(matrix& observer_matrix, size_t idx)
{
	smart_assert(is_initialized());
	smart_assert(observer_matrix.is_initialized());
	smart_assert(idx < item_count);
	smart_assert(vector3::are_equal(observer_matrix.get_format(), label_format));

	std::lock_guard<std::mutex> lock(label_mutex);
	observer_matrix.observe_row(data_table, shuffle_table[idx], data_item_count()); //this could be improved, because more than one thread can read at the same time
}

void data_space::set_data(const matrix& m, size_t idx)
{
	smart_assert(is_initialized());
	smart_assert(idx < item_count);

	std::lock_guard<std::mutex> lock(data_mutex);
	set_data_in_table_at(m, shuffle_table[idx]);
}

void data_space::set_label(const matrix& m, size_t idx)
{
	smart_assert(is_initialized());
	smart_assert(idx < item_count);

	std::lock_guard<std::mutex> lock(label_mutex);
	set_label_in_table_at(m, shuffle_table[idx]);
}

void data_space::copy_to_gpu()
{
	smart_assert(is_initialized());
	data_table.enable_gpu_mode();
}

void data_space::clear()
{
	smart_assert(is_initialized());
	
	std::lock_guard<std::mutex> lock1(label_mutex);
	std::lock_guard<std::mutex> lock2(data_mutex);

	data_table.set_all(0);
}

std::string data_space::to_string()
{
	std::string result = "";

	result += "data space:\n";
	result += "item count: " + std::to_string(item_count) + "\n";
	result += "data format: " + data_format.to_string() + "\n";
	result += "label format: " + label_format.to_string() + "\n";
	result += "data table:\n";

	matrix data_observer(get_data_format());
	matrix label_observer(get_label_format());

	if (is_in_gpu_mode())
	{
		data_observer.enable_gpu_mode();
		label_observer.enable_gpu_mode();
	}

	data_table.sync_device_and_host();

	for (int i = 0; i < item_count; i++)
	{
		observe_data_at_idx(data_observer, i);
		observe_label_at_idx(label_observer, i);
		result += std::to_string(i) + ":\n";
		result += "data: \n" + data_observer.get_string() + "\n";
		result += "label: \n" + label_observer.get_string() + "\n";
		result += "-------------\n";
	}

	return result;
}
