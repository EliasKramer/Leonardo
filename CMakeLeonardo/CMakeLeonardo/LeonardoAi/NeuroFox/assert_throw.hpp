#pragma once
#include <stdexcept>

/*
#define NDEBUG

#ifdef NDEBUG
*/
#define smart_assert(expression) ((void)0)
/*
#else

#define smart_assert(expression) if(!(expression)) { throw std::runtime_error(""); }

#endif
*/
