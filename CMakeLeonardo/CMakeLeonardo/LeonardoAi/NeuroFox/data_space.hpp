#pragma once
#include "matrix.hpp"
#include <mutex>
#include <algorithm>

class data_space
{
private:
	size_t item_count = 0;

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
	std::vector<size_t> shuffle_table;
	
	vector3 data_format;
	vector3 label_format;

	std::mutex data_mutex;
	std::mutex label_mutex;

	size_t label_item_count();
	size_t data_item_count();

	void set_data_in_table_at(const matrix& m, size_t idx);
	void set_label_in_table_at(const matrix& m, size_t idx);

	void allocate_data_table();

	void init_shuffle_table();

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

	bool is_initialized() const;
	bool is_in_gpu_mode() const;
	
	size_t get_item_count() const;
	vector3 get_data_format() const;
	vector3 get_label_format() const;

	void shuffle();

	size_t byte_size() const;

	void observe_data_at_idx(matrix& observer_matrix, size_t idx);
	void observe_label_at_idx(matrix& observer_matrix, size_t idx);
	void set_data(const matrix& m, size_t idx);
	void set_label(const matrix& m, size_t idx);

	void copy_to_gpu();

	void clear();

	std::string to_string();
};