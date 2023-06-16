#include <stdexcept>

#pragma once
class vector3
{
public:
	size_t x;
	size_t y;
	size_t z;

	vector3();
	vector3(size_t x, size_t y, size_t z);
	vector3(size_t x, size_t y);
	vector3(size_t x);

	vector3(std::ifstream& file);

	bool is_in_bounds(const vector3& format) const;
	size_t get_index(const vector3& format) const;
	size_t item_count() const;

	void write_to_ofstream(std::ofstream& file) const;

	static bool are_equal(const vector3& v1, const vector3& v2);
	bool operator==(const vector3& other) const;
	bool operator!=(const vector3& other) const;
};