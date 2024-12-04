#pragma once

#include "wheel.hpp"

namespace gxm{
class Wheels{
public:
    struct Config{
        Wheel::Config wheel_config;
    };

    using Motor = Wheel::Motor;
    using Refs = std::array<std::reference_wrapper<Motor>, 4>;
    // using Refs = std::tuple<
    //     // std::reference_wrapper<Wheel>,
    //     // std::reference_wrapper<Wheel>,
    //     // std::reference_wrapper<Wheel>,
    //     // std::reference_wrapper<Wheel>
    //     Wheel *,
    //     Wheel *,
    //     Wheel *,
    //     Wheel *
    // >;
protected:

    const Config & config_;
    std::array<Wheel, 4> instances_;

public:
    Wheels(const Config & config, const Refs & refs):
        config_(config),
        instances_{
            Wheel{config.wheel_config, refs[0]},
            Wheel{config.wheel_config, refs[1]},
            Wheel{config.wheel_config, refs[2]},
            Wheel{config.wheel_config, refs[3]}
            // &std::get<0>(refs).get(), 
            // &std::get<1>(refs).get(), 
            // &std::get<2>(refs).get(), 
            // &std::get<3>(refs).get() 
            // std::get<0>(refs),
            // std::get<1>(refs),
            // std::get<2>(refs),
            // std::get<3>(refs)
        }
        {;}

    void init();

    void update();

    bool verify();


    void setPosition(const std::tuple<real_t, real_t, real_t, real_t> & pos);

    void setDelta(const std::tuple<real_t, real_t, real_t, real_t> & delta);

    void forward(const std::tuple<real_t, real_t, real_t, real_t> & delta);

    std::tuple<real_t, real_t, real_t, real_t> getPosition();


    Wheel & operator [](const size_t idx){
        if(idx > 3) HALT;
        return instances_[idx];
    }
};

}