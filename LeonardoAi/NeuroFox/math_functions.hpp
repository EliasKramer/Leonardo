#pragma once
#include "enum_space.hpp"

float sigmoid(float x);
float relu(float x);

float sigmoid_derivative(float x);
float relu_derivative(float x);

//inverse sigmoid
float logit(float x);
float inverse_relu(float x);

//activation function pointer
using activation_fn = float(*)(float);

const activation_fn ACTIVATION[] =
{ sigmoid, relu };

const activation_fn DERIVATIVE[] =
{ sigmoid_derivative, relu_derivative };

const activation_fn INVERSE[]
{ logit, inverse_relu };