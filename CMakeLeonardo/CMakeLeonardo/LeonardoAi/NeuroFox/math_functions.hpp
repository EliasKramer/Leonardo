#pragma once
#include "enum_space.hpp"

constexpr float LEAKY_RELU_FACTOR = 0.01f;

float sigmoid(float x);
float relu(float x);
float leaky_relu(float x);

float sigmoid_derivative(float x);
float relu_derivative(float x);
float leaky_relu_derivative(float x);

//inverse sigmoid
float logit(float x);
float inverse_relu(float x);
float inverse_leaky_relu(float x);

//activation function pointer
using activation_fn = float(*)(float);

const activation_fn ACTIVATION[] =
{ sigmoid, relu, leaky_relu };

const activation_fn DERIVATIVE[] =
{ sigmoid_derivative, relu_derivative, leaky_relu_derivative };

const activation_fn INVERSE[]
{ logit, inverse_relu, leaky_relu, inverse_leaky_relu };