#include "vector3.hpp"
#include <fstream>
vector3::vector3()
:x(0), y(0), z(0)
{}

vector3::vector3(size_t x, size_t y, size_t z)
	:x(x), y(y), z(z)
{}

vector3::vector3(size_t x, size_t y)
	:x(x), y(y), z(0)
{}

vector3::vector3(size_t x)
	:x(x), y(0), z(0)
{}

vector3::vector3(std::ifstream& file)
{
	file.read((char*)&x, sizeof(x));
	file.read((char*)&y, sizeof(y));
	file.read((char*)&z, sizeof(z));
}

bool vector3::is_in_bounds(const vector3& format) const
{
	return x < format.x && y < format.y && z < format.z;
}

size_t vector3::get_index(const vector3& format) const
{
	if(!this->is_in_bounds(format))
		throw std::invalid_argument("vector3::get_index: format is not in bounds");

	//x + y * width + z * width * height
	return x + y * format.x + z * format.x * format.y;
}

size_t vector3::item_count() const
{
	return x * y * z;
}

void vector3::write_to_ofstream(std::ofstream& file) const
{
	file.write((char*)&x, sizeof(x));
	file.write((char*)&y, sizeof(y));
	file.write((char*)&z, sizeof(z));
}

bool vector3::are_equal(const vector3 & v1, const vector3 & v2)
{
	return 
		v1.x == v2.x &&
		v1.y == v2.y &&
		v1.z == v2.z;
}

bool vector3::operator==(const vector3& other) const
{
	return are_equal(*this, other);
}

bool vector3::operator!=(const vector3& other) const
{
	return !are_equal(*this, other);
}
