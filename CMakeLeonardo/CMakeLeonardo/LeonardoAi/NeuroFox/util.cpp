#include "util.hpp"
#include <iomanip>

int random_idx(int size)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution
		<int> dis(0, size);
	return dis(gen) % size;
}

bool biased_coin_toss(float true_bias, float false_bias)
{
	if (true_bias <= 0 || false_bias <= 0)
		throw std::invalid_argument("true_bias and false_bias must be greater than 0");

	// Seed the random number generator with the current time
	std::srand((int)std::time(nullptr));
	// Generate a random number between 0 and 1
	float random_number = (float)std::rand() / (float)RAND_MAX;
	// Return true if the random number is less than the bias, false otherwise
	return random_number < true_bias / (true_bias + false_bias);
}

float random_float_excl(float min, float max)
{
	if (min > max)
		throw std::invalid_argument("min must be less than max");

	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(
		min,
		max - std::numeric_limits<float>::epsilon()
	);

	float ret_val = dis(gen);

	if (ret_val < min || ret_val >= max)
	{
		//why does this get called TODO FIX
		std::cout << "random_float_excl returned a value outside of the range"
			<< " min: " << min
			<< " max: " << max
			<< " ret_val: " << ret_val
			<< std::endl;
		return min;
	}

	return ret_val;
}

bool is_whole_number(float number)
{
	return number == (int)number;
}

int swap_endian(int value) {
	int result = 0;
	result |= (value & 0xFF) << 24;
	result |= ((value >> 8) & 0xFF) << 16;
	result |= ((value >> 16) & 0xFF) << 8;
	result |= ((value >> 24) & 0xFF);
	return result;
}
//determins wether the system is little endian or big endian
bool is_little_endian()
{
	int num = 1;
	return (*(char*)&num == 1);
}

std::string byte_size_to_str(size_t byte_size)
{
	size_t giga = 1024 * 1024 * 1024;
	size_t mega = 1024 * 1024;
	size_t kilo = 1024;

	size_t giga_count = byte_size / giga;
	size_t mega_count = (byte_size - giga_count * giga) / mega;
	size_t kilo_count = (byte_size - giga_count * giga - mega_count * mega) / kilo;
	size_t byte_count = byte_size - giga_count * giga - mega_count * mega - kilo_count * kilo;

	std::string result = "";
	if (giga_count > 0)
		result += std::to_string(giga_count) + "GB ";
	if (mega_count > 0)
		result += std::to_string(mega_count) + "MB ";
	if (kilo_count > 0)
		result += std::to_string(kilo_count) + "KB ";
	if (byte_count > 0)
		result += std::to_string(byte_count) + "B ";

	return result == "" ? "0 B " : result;
}

bool convolution_format_valid(
	size_t input_size,
	size_t filter_size,
	size_t stride)
{
	if (input_size <= 0 || filter_size <= 0 || stride <= 0 ||
		input_size < filter_size || filter_size < stride)
	{
		return false;
	}

	const float output_size = (input_size - filter_size) / (float)stride + 1;

	return is_whole_number(output_size);
}

bool convolution_format_valid(
	vector3 input_size,
	vector3 filter_size,
	size_t stride,
	vector3 output_size)
{
	return
		convolution_output_size(input_size.x, filter_size.x, stride) == output_size.x &&
		convolution_output_size(input_size.y, filter_size.y, stride) == output_size.y &&
		input_size.z == filter_size.z;
}

size_t convolution_output_size(
	size_t input_size,
	size_t filter_size,
	size_t stride)
{
	if (!convolution_format_valid(input_size, filter_size, stride))
	{
		throw std::invalid_argument("convolution format is invalid");
	}

	return (input_size - filter_size) / stride + 1;
}

std::string ms_to_str(size_t ms)
{
	return ms_to_str(ms, 2);
}

std::string ms_to_str(size_t given_ms, size_t max_unit_count)
{
	//this whole thing is a hack, i know
	const static size_t millisecond = 1;
	const static size_t second = 1000;
	const static size_t minute = 60 * second;
	const static size_t hour = 60 * minute;
	const static size_t day = 24 * hour;
	const static size_t week = 7 * day;
	const static size_t month = 30 * day;
	const static size_t year = 365 * day;

	const static size_t time_lengths[] = { year, month, week, day, hour, minute, second, millisecond };
	const static std::string time_unit_names[] = { "y", "m", "w", "d", "h", "min", "s", "ms" };

	size_t ms_remaining = given_ms;

	std::string result = "";

	size_t time_unit_count_remaining = max_unit_count;

	for (int i = 0; i < 8 && time_unit_count_remaining > 0; i++)
	{
		//hours, minutes, seconds, etc
		size_t current_unit = time_lengths[i];
		//how many units fit into our given ms count
		size_t time_units = ms_remaining / time_lengths[i];
		//how many ms are left over after we take out the units
		ms_remaining -= time_units * time_lengths[i];

		if (time_units > 0)
		{
			result += std::to_string(time_units) + time_unit_names[i] + " ";
			time_unit_count_remaining--;
		}
	}

	return result == "" ? "0ms" : result;
}

//quick and dirty
std::string get_current_time_str()
{
	// Get the current time
	std::time_t currentTime = std::time(nullptr);
	std::tm localTime;
	localtime_s(&localTime, &currentTime);

	// Format the current time
	char timeBuffer[6]; // Format: hh:mm
	std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M", &localTime);

	char dateBuffer[6]; // Format: dd.mm
	std::strftime(dateBuffer, sizeof(dateBuffer), "%d.%m", &localTime);

	// Return the formatted time as a string
	return  std::string(dateBuffer) + " " + std::string(timeBuffer);
}