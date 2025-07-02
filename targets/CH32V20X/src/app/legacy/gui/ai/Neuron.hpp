#pragma once

#include "core/math/real.hpp"
#include <vector>
#include <span>

namespace ymd::ann{

template<arithmetic T>
class Neuron_t{
protected:
    std::vector<T> weights_;

    Neuron_t(const Neuron_t & other) = default;
    Neuron_t(Neuron_t && other) = default;
public:

    Neuron_t(size_t size) : weights_(size) {}

    Neuron_t(std::span<T> weights) {
        std::copy(weights.begin(), weights.end(), weights_.begin());
    }

    std::span<const T> weights() const {
        return std::span<const T>(weights_);
    }

    std::span<T> weights() {
        return std::span<T>(weights_);
    }

    T sum(std::span<const T> inputs){
        if(inputs.size() != weights_.size()){
            HALT;
        }

        T sum = 0;
        for(size_t i = 0; i < weights_.size(); i++){
            sum += inputs[i] * weights_[i];
        }
        return sum;
    }

    Neuron_t copy() const {
        return *this;
    }
};

using Neuron = Neuron_t<real_t>;

}