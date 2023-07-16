#pragma once
#include <stdexcept>

#ifdef NDEBUG

#define smart_assert(expression, msg) ((void)0)
#define smart_assert(expression) ((void)0)

#else

#define smart_assert(expression, msg) if(!expression) { throw std::runtime_error(msg); }										
#define smart_assert(expression) if(!expression) { throw std::runtime_error(""); }

#endif