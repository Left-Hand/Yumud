#pragma once

#include "wheel.hpp"

namespace ymd::robots{

class Wheels{
public:
    struct Config{
        Wheel::Config wheel_config;

        real_t max_curr;
    };

    using Motor = Wheel::Motor;
    using Refs = std::array<std::reference_wrapper<Motor>, 4>;
protected:

    const Config & config_;
    std::array<Wheel, 4> instances_;
    hal::Can & can_;
public:
    Wheels(const Config & config, const Refs & refs, hal::Can & can):
        config_(config),
        instances_{
            Wheel{config.wheel_config, refs[0]},
            Wheel{config.wheel_config, refs[1]},
            Wheel{config.wheel_config, refs[2]},
            Wheel{config.wheel_config, refs[3]}
        },
        can_(can)
        {;}

    void init();

    void request();

    bool verify();

    void setSpeed(const std::tuple<real_t, real_t, real_t, real_t> & spd);

    void setCurrent(const std::tuple<real_t, real_t, real_t, real_t> & curr);

    void setPosition(const std::tuple<real_t, real_t, real_t, real_t> & pos);

    void setDelta(const std::tuple<real_t, real_t, real_t, real_t> & delta);

    void forward(const std::tuple<real_t, real_t, real_t, real_t> & delta);

    void freeze();

    std::tuple<real_t, real_t, real_t, real_t> getPosition();


    Wheel & operator [](const size_t idx){
        if(idx > 3) HALT;
        return instances_[idx];
    }
};

}