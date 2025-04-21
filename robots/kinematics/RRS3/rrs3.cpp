
#include "core/math/realmath.hpp"
#include "core/math/iq/iqmath.hpp"
#include "core/utils/option.hpp"
#include "rrs3_kinematics.hpp"

using namespace ymd;

using RRS3 = robots::RRS_Kinematics<real_t>;
using Config = RRS3::Config;
using Gesture = RRS3::Gesture;

using T = real_t;

[[maybe_unused]] 
static void test(){

    [[maybe_unused]] 
    constexpr const Config cfg{
        .base_length = 1,
        .link_length = 1,
        .base_plate_radius = 1,
        .top_plate_radius = 1
    };


    [[maybe_unused]] 
    constexpr const RRS3 rrs3{cfg};

    constexpr auto res = rrs3.inverse(Gesture{
        .orientation = Quat_t<T>::from_euler({0, 0, 0}),
        .z = 4.0_r,
    }).unwrap();

    static_assert(res[0].j1_rad == 0.0_r);
    
}