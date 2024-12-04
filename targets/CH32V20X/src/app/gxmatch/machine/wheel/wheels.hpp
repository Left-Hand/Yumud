#pragma once

#include "wheel.hpp"

namespace gxm{
class Wheels{
public:
    struct Config{
    };

    using Refs = std::array<std::reference_wrapper<Wheel>, 4>;
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
    std::array<Wheel *, 4> instances_;
public:
    Wheels(const Config & config, const Refs & refs):
        config_(config),
        instances_{
            &refs[0].get(),
            &refs[1].get(),
            &refs[2].get(),
            &refs[3].get()
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

    bool verify();


    void setPosition(const std::tuple<real_t, real_t, real_t, real_t> & pos);

    void setDelta(const std::tuple<real_t, real_t, real_t, real_t> & delta);

    void forward(const std::tuple<real_t, real_t, real_t, real_t> & delta);

    std::tuple<real_t, real_t, real_t, real_t> getPosition();


    Wheel & operator [](const size_t idx){
        if(idx > 3) HALT;
        return *instances_[idx];
    }
};

}