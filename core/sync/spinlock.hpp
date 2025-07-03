#pragma once


#include "atomic.hpp"


namespace ymd::sync{
struct SpinLock {
    __inline void lock() {
		while (locked.test_and_set(std::memory_order_acquire)) {
			;
		}
	}
    __inline void unlock() {
		locked.clear(std::memory_order_release);
	}
private:
	std::atomic_flag locked = {0};
};
};