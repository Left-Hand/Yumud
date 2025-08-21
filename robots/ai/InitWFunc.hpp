#pragma once

#include "Neuron.hpp"

namespace ymd::ann {

namespace NeuronUtils{

enum class InitWFuncType {
    Random,
    Xavier,
    He,
    None
};

class InitWFunc {
public:

    template<arithmetic T>
    static void InitWeights(InitWFuncType initWFunc, Neuron_t<T> & neuron) {
        switch (initWFunc) {
            case InitWFuncType::Xavier:
                XavierInitWeights(neuron.weights());
                break;
            case InitWFuncType::He:
                HeInitWeights(neuron.weights());
                break;
            case InitWFuncType::Random:
                RandomInitWeights(neuron.weights());
                break;
            case InitWFuncType::None:
                NoneInitWeights(neuron.weights());
            default:
                break;
        }
    }

private:

    template<arithmetic T>
    static void RandomInitWeights(std::span<T> weightsList) {
        // std::random_device rd;
        // std::mt19937 gen(rd());
        // std::normal_distribution<> dis(0.0, 0.01);

        for (auto& weight : weightsList) {
        //     weight = static_cast<T>(dis(gen));
        }
    }

    template<arithmetic T>
    static void XavierInitWeights(std::span<T> weightsList) {
        // std::random_device rd;
        // std::mt19937 gen(rd());
        // std::uniform_real_distribution<> dis(-1.0, 1.0);

        // T scale = 1.0f / sqrt(weightsList.size());
        // for (auto& weight : weightsList) {
        //     weight = static_cast<T>(dis(gen) * 2 * scale - scale);
        // }
    }



    template<arithmetic T>
    static void HeInitWeights(std::span<T> weightsList) {
        // std::random_device rd;
        // std::mt19937 gen(rd());
        // std::normal_distribution<> dis(0.0, 1.0);

        // T stdDev = std::sqrt(2.0f / weightsList.size());
        // for (auto& weight : weightsList) {
        //     weight = static_cast<T>(dis(gen) * stdDev);
        // }
    }

    template<arithmetic T>
    static void NoneInitWeights(std::span<T> weightsList) {
        for(auto& weight : weightsList){
            weight = 0;
        }
    }
};

}

} // namespace ann