#pragma once
#include "matrix.hpp"
class data_space
{
private:
	size_t item_count = 0;
	bool copied_to_gpu = false;

	/*
		One piece of data is stored in the following format:
		+-------------+-----+
		| data        |label|
		+-------------+-----+
		data and label are stored in one dimension

		the data_iterator and label_iterator are used to
		point to current data and label
		these iterators also store the format of the data and label

		data_table looks like this:
		+-------------+-----+
		| data        |label|
		+-------------+-----+
		| data        |label|
		+-------------+-----+
		| data        |label|
		+-------------+-----+
	*/
	matrix data_table;

	size_t iterator_idx = 0;
	
	matrix data_iterator;
	matrix label_iterator;

	size_t label_item_count();
	size_t data_item_count();

	void set_data_in_table_at(const matrix& m, size_t idx);
	void set_label_in_table_at(const matrix& m, size_t idx);

	void if_not_initialized_throw() const;
	void allocate_data_table();
public:
	data_space();
	data_space(size_t given_item_count, vector3 data_format);
	data_space(size_t given_item_count, vector3 data_format, vector3 label_format);
	data_space(
		vector3 data_format,
		vector3 label_format,
		const std::vector<matrix>& given_data,
		const std::vector<matrix>& given_label);

	data_space(
		vector3 data_format,
		const std::vector<matrix>& given_data);

	data_space& operator=(const data_space& other);

	size_t get_item_count() const;

	void iterator_next();
	void iterator_reset();
	bool iterator_has_next() const;

	void set_iterator_idx(size_t idx);
	size_t get_iterator_idx() const;

	//TODO
	//load in file
	//save in file

	void copy_to_gpu();

	const matrix& get_current_data();
	const matrix& get_current_label();
	void set_current_data(const matrix& m);
	void set_current_label(const matrix& m);
};