#pragma once

#include "wheel.hpp"

namespace gxm{
class Wheels{
public:
    struct Config{
        real_t wheel_radius;
        real_t kp;
        real_t kd;
    };

protected:
    using Refs = std::array<std::reference_wrapper<Wheel>, 4>;

    const Config & config_;
    std::array<Wheel *, 4> instances_;
public:
    Wheels(const Config & config, const Refs & refs):
        config_(config),
        instances_{&refs[0].get(), &refs[1].get(), &refs[2].get(), &refs[3].get()}
        {;}

    void init();

    bool verify();

    void setSpeed(const std::tuple<real_t, real_t, real_t, real_t> & spds);

    void setPosition(const std::tuple<real_t, real_t, real_t, real_t> & pos);

    void setDelta(const std::tuple<real_t, real_t, real_t, real_t> & pos);

    std::tuple<real_t, real_t, real_t, real_t> getPosition();

    std::tuple<real_t, real_t, real_t, real_t> getSpeed();

    Wheel & operator [](const size_t idx){
        if(idx > 3) HALT;
        return *instances_[idx];
    }
};

}