#pragma once
#include <string>
#include "util.hpp"

class test_result
{
public:
	size_t data_count = 0;
	long long time_in_ms = 0;
	float avg_cost = 0;
	float accuracy = 0;

	std::string to_string();
};