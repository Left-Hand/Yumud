#pragma once

#include "core/platform.hpp"

template<typename T, unsigned int N>
class AverageFilter {
protected:
    static constexpr T invN = T(1.0 / N);

    T data_[N];
    T sum = T(0);
    int index = 0;
    bool inited = false;
    

public:
    void reset(const T x) {
        for(unsigned int i = 0; i < N; i++) data_[i] = x;
        sum = x * N;
        inited = true;
    }

    T update(const T x) {
        if (unlikely(!inited)) {
            inited = true;
            reset(x);
            return x;
        }

        T old_value = data_[index];
        data_[index] = x;
        index = (index + 1) % N;

        sum += x - old_value;

        return sum * invN;
    }
};