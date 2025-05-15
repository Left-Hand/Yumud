#include "src/testbench/tb.h"
#include "core/math/realmath.hpp"
#include "core/math/iq/iqmath.hpp"
#include "core/utils/option.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/clock.hpp"

#include "hal/bus/uart/uarthw.hpp"

#include "rrs3_kinematics.hpp"

using namespace ymd;

using T = real_t;
// using T = float;

using RRS3 = robots::RRS_Kinematics<T>;
using RR2 = robots::RR2_Kinematics<T>;
using Config = RRS3::Config;
using Gesture = RRS3::Gesture;



[[maybe_unused]] 
void rr2_tb(){
    RR2::Config cfg{
        .base_length = 2,
        .link_length = 2
    };

    const auto solu = RR2::inverse(cfg, {0_r, 2_r}, {0, 0});
    if(solu.is_some()){
        const auto [j1, j2] = solu.unwrap().to_absolute();
        DEBUG_PRINTLN(
            robots::to_degrees(j1),
            robots::to_degrees(j2)
        );
    }
    else DEBUG_PRINTLN("/");
    while(true);
}


void rrs3_tb(){

    [[maybe_unused]] 
    constexpr const Config cfg{
        .base_length = 5,
        .link_length = 5,
        .base_plate_radius = 1,
        .top_plate_radius = 1
    };


    [[maybe_unused]] 
    constexpr const RRS3 rrs3{cfg};
    constexpr const auto head = Quat_t<T>::from_shortest_arc(
        Vector3_t<T>{0, 0, 1},
        // Vector3_t<T>{-0.1_r, -0.1_r, 1_r}.normalized()
        Vector3_t<T>{T(-0.1), T(-0.1), 1}.normalized()
        // Vector3_t<T>{T(0), T(0), 1}.normalized()
    );


    [[maybe_unused]] 
    constexpr Gesture gest{
        .orientation = head,
        .z = 5,
    };
    
    // DEBUG_PRINTLN(rrs3.get_top_point(cfg, Vector2_t<T>::RIGHT.rotated(T(TAU  / 3)), gest));
    // DEBUG_PRINTLN(rrs3.get_top_point(cfg, Vector2_t<T>::RIGHT.rotated(T(-TAU / 3)), gest));
    // DEBUG_PRINTLN(rrs3.get_top_point(cfg, Vector2_t<T>::RIGHT.rotated(T(0)), gest));
    // DEBUG_PRINTLN(rrs3.inverse_single_axis(cfg, Vector2_t<T>::RIGHT.rotated(T(TAU  / 3)), gest).unwrap().to_absolute());
    // DEBUG_PRINTLN(rrs3.inverse_single_axis(cfg, Vector2_t<T>::RIGHT.rotated(T(-TAU / 3)), gest).unwrap().to_absolute());
    // DEBUG_PRINTLN(rrs3.inverse_single_axis(cfg, Vector2_t<T>::RIGHT.rotated(T(0)), gest).unwrap().to_absolute());
    
    DEBUG_PRINTLN(Vector2_t<T>::RIGHT.rotated(T(TAU  / 3)));
    DEBUG_PRINTLN(Vector2_t<T>::RIGHT.rotated(T(-TAU / 3)));

    const auto u = micros();
    const auto res_opt = rrs3.inverse(gest);
    DEBUG_PRINTLN(micros() - u);
    if(res_opt.is_some()){
        const auto res = res_opt.unwrap();
        for(const auto & solu : res){
            const auto [j1, j2] = solu.to_absolute();
            DEBUG_PRINTLN(
                robots::to_degrees(j1),
                robots::to_degrees(j2)
            );
        }
    }

    // DEBUG_PRINTLN(head);

    while(true){
        // DEBUG_PRINTLN(millis());
        // delay(1);
    }
    
}

[[maybe_unused]] 
void rrs3_main(){

    DEBUGGER_INST.init(576000, CommStrategy::Blocking);
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets();
    DEBUGGER.force_sync();

    DEBUG_PRINTLN(std::setprecision(4));
    // rr2_tb();
    rrs3_tb();
}
