#pragma once

#include "core/math/real.hpp"
#include <vector>
#include <span>

namespace ymd::ann{

template<arithmetic T>
class Neuron{
protected:
    std::vector<T> weights_;

    Neuron(const Neuron & other) = default;
    Neuron(Neuron && other) = default;
public:

    Neuron(size_t size) : weights_(size) {}

    Neuron(std::span<T> weights) {
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

    Neuron copy() const {
        return *this;
    }
};

using Neuron = Neuron<iq16>;

}