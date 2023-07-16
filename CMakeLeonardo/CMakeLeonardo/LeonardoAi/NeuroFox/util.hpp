#pragma once
#include <cstdlib>
#include <ctime>
#include <random>
#include <string>
#include "vector3.hpp"
#include <iostream>

constexpr float FLOAT_TOLERANCE = 0.000001f;

// a random number between 0 and size-1
int random_idx(int size);

//returns true with a probability of true_bias, false with a probability of false_bias
bool biased_coin_toss(float true_bias, float false_bias);

float random_float_excl(float min, float max);

bool is_whole_number(float number);

/// <summary>
/// splits the pointer into multiple chunks
/// these chunks are the size of elements * sizeof(T)
/// and you get the index'th pointer
/// </summary>
/// <param name="ptr">the ptr that will be split</param>
/// <param name="elements">the amount of elements that each chunk has</param>
/// <param name="index">the index of the chunk</param>
/// <returns>pointer to the index'th chunk that got split</returns>
template<typename T>
T* sub_ptr(T* ptr, size_t elements, size_t index)
{
	//return (T*)(((char*)ptr) + index * elements * sizeof(T))
	return ptr + index * elements;
}

int swap_endian(int value);
//determins wether the system is little endian or big endian
bool is_little_endian();

std::string byte_size_to_str(size_t byte_size);

bool convolution_format_valid(
	size_t input_size, 
	size_t filter_size, 
	size_t stride);
bool convolution_format_valid(
	vector3 input_size,
	vector3 filter_size,
	size_t stride,
	vector3 output_size);

size_t convolution_output_size(
	size_t input_size,
	size_t filter_size,
	size_t stride);

std::string ms_to_str(size_t ms);
std::string ms_to_str(size_t ms, size_t time_unit_count);
std::string get_current_time_str();