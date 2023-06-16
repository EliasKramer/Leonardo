#include "data_space.hpp"

size_t data_space::label_item_count()
{
	return label_iterator.item_count();
}

size_t data_space::data_item_count()
{
	return data_iterator.item_count();
}

void data_space::set_data_in_table_at(const matrix& m, size_t idx)
{
	//checking for format issues is done in the matrix itself
	data_table.set_row_from_matrix(m, idx);
}

void data_space::set_label_in_table_at(const matrix& m, size_t idx)
{
	//checking for format issues is done in the matrix itself
	data_table.set_row_from_matrix(m, idx, data_item_count());
}

void data_space::if_not_initialized_throw() const
{
	if (data_table.item_count() == 0)
	{
		throw std::exception("data_space not initialized");
	}
}

void data_space::allocate_data_table()
{
	data_table = matrix(
		vector3(
			(size_t)(data_item_count() + label_item_count()),
			item_count,
			(size_t)1));
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
	data_iterator(data_format),
	label_iterator(label_format),
	item_count(given_item_count)
{
	allocate_data_table();
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
		throw std::exception("data and label size mismatch");
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
		data_table = other.data_table;
		data_iterator = other.data_iterator;
		label_iterator = other.label_iterator;
		item_count = other.item_count;
		iterator_idx = other.iterator_idx;
		copied_to_gpu = other.copied_to_gpu;
	}
	return *this;
}

size_t data_space::get_item_count() const
{
	return item_count;
}

void data_space::iterator_next()
{
	if_not_initialized_throw();
	iterator_idx = (iterator_idx + 1) % item_count;
}

void data_space::iterator_reset()
{
	if_not_initialized_throw();
	iterator_idx = 0;
}

bool data_space::iterator_has_next() const
{
	if_not_initialized_throw();

	return iterator_idx + 1 < item_count;
}

void data_space::set_iterator_idx(size_t idx)
{
	if_not_initialized_throw();
	if (idx >= item_count)
	{
		throw std::exception("iterator index out of range");
	}
	iterator_idx = idx;
}

size_t data_space::get_iterator_idx() const
{
	return iterator_idx;
}

void data_space::copy_to_gpu()
{
	if_not_initialized_throw();

	if (copied_to_gpu)
		return;

	data_table.enable_gpu_mode();
	data_iterator.enable_gpu_mode();
	label_iterator.enable_gpu_mode();

	copied_to_gpu = true;
}

const matrix& data_space::get_current_data()
{
	if_not_initialized_throw();
	/*
	float* data_ptr = data_table.get_ptr_row(iterator_idx, 0);
	data_iterator.set_ptr_as_source(data_ptr);
	*/
	data_iterator.observe_row(data_table, iterator_idx);

	return data_iterator;
}

const matrix& data_space::get_current_label()
{
	if_not_initialized_throw();

	if (label_iterator.item_count() == 0)
	{
		throw std::exception("trying to access label, which is not set");
	}
	/*
	float* label_ptr = data_table.get_ptr_item(data_item_count(), iterator_idx, 0);
	label_iterator.set_ptr_as_source(label_ptr);
	*/
	label_iterator.observe_row(data_table, iterator_idx, data_item_count());

	return label_iterator;
}

void data_space::set_current_data(const matrix& m)
{
	if_not_initialized_throw();
	set_data_in_table_at(m, iterator_idx);
}

void data_space::set_current_label(const matrix& m)
{
	if_not_initialized_throw();
	set_label_in_table_at(m, iterator_idx);
}
