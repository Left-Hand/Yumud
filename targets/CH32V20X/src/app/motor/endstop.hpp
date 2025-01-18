#pragma once

#include "sys/core/platform.h"

namespace ymd{
template<arithmetic T>
class Endstop {
public:
    Endstop(T min, T max) : min(min), max(max) {}
    T min;
    T max;
    T value;
    T get() const { return value; }
    void set(T v) { value = v; }
    void update(T v) { value = v; }
    void reset() { value = 0; }
    bool isMin() const { return value == min; }
}
}