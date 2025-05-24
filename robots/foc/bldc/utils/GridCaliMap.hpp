#pragma once

#include "CaliMap.hpp"


namespace ymd::foc{

class GridCaliMap{
public:
    enum class ErrorCode{
        OK = 0,
        NO_SAMPLES,
        NO_SAMPLE_FOUND_IN_RANGE,
        SAMPLES_CLIMBD_FAST,
        SAMPLES_DISCONTINUOUS
    };

    class Error{
        ErrorCode code;  
    };
protected:
    using Sample = ymd::Vector2<real_t>;
    using Samples = std::vector<Sample>;
    Samples samples_;

    void resize(const size_t size);
    void reserve(const size_t size);

    bool ok_ = false;
public:
    GridCaliMap(){;}

    ErrorCode loadSamples(const Samples & samples);

    bool ok() const {
        return ok_;
    }
};

}