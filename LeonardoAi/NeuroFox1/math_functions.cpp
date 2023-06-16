#include "math_functions.hpp"
#include <cmath>
#include <stdexcept>

float sigmoid(float x)
{
	return 1.0f / (1.0f + exp(-x));
}

float relu(float x)
{
	return x > 0 ? x : 0;
}

float sigmoid_derivative(float x)
{
	float sig = sigmoid(x);
	return sig * (1.0f - sig);
}

float relu_derivative(float x)
{
	return x > 0 ? 1.0f : 0;
}

float logit(float x)
{
	return log(x / (1.0f - x));
}

float inverse_relu(float x)
{
	throw std::runtime_error("inverse relu does not exist");
}
