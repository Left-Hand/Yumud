#pragma once

#include "core/math/real.hpp"

namespace ymd::drivers{
    class FlowSensor{
    public:
        virtual void update() = 0;
        virtual void update(const real_t rad) = 0;
        virtual std::tuple<real_t, real_t> getPosition() = 0;
    };
}