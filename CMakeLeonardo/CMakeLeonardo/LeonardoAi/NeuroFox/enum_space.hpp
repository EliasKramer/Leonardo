#pragma once

enum _pooling_type
{
	max_pooling = 0,
	min_pooling = 1,
	average_pooling = 2
} typedef e_pooling_type_t;

typedef enum _layer_type {
	convolutional,
	pooling,
	fully_connected
} e_layer_type_t;

enum _activation {
	sigmoid_fn = 0,
	relu_fn = 1,
	leaky_relu_fn = 2
} typedef e_activation_t;
