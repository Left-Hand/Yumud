#pragma once

#include "core/platform.hpp"

template<integral T>
__inline T fast_sqrt_i(const T n) {
	if (n == 0 || n == 1) return n;
	
	T x = n;
	T y = (x + 1) / 2;
	while (y < x) {
		x = y;
		y = (x + n / x) / 2;
	}
	// Check if x is the correct square root
	if (x * x > n) {
		return x - 1;
	} else {
		return x;
	}
}

template<integral T>
__inline T sqrt(const T n){
	return fast_sqrt_i(n);
}