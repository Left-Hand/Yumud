#pragma once

#include "im6014_primitive.hpp"
#include "im6014_utils.hpp"
#include "primitive/arithmetic/angular.hpp"

namespace ymd::robots::unitree::im6014{

struct [[nodiscard]] StatusCode final{
    uint8_t id:4;
    uint8_t status:3;
    uint8_t timeout:1;
};

struct ScaleProfile_N6014B12d6{
    static constexpr uint32_t TAU_RATIO = 2560;
    static constexpr uint32_t X2_RATIO = 64;
    static constexpr uint32_t X1_RATIO = 32768;
    static constexpr uint32_t KP_RATIO = 12800;
    static constexpr uint32_t KD_RATIO = 51200;
};

using ScaleProfile = ScaleProfile_N6014B12d6;



struct [[nodiscard]] CodeFactory final{

    template<typename T>
    static constexpr TorqueCode make_tau_code_from_nm(const T torque_nm){
        const auto bits = utils::bounded_scale<int16_t, ScaleProfile::TAU_RATIO>(torque_nm);
        return TorqueCode{static_cast<int16_t>(bits)};
    }

    template<typename T>
    static constexpr X2Code make_x2code_from_speed(const Angular<T> speed){
        const auto bits = utils::bounded_scale<int16_t, ScaleProfile::X2_RATIO>(speed.to_turns());
        return X2Code{static_cast<int16_t>(bits)};
    }

    template<typename T>
    static constexpr X1Code make_x1code_from_turns(const T turns){
        const auto bits = utils::bounded_scale<int32_t, ScaleProfile::X1_RATIO>(turns);
        return X1Code{static_cast<int32_t>(bits)};
    }

    template<typename T>
    static constexpr KpCode make_kpcode(const T val){
        const auto bits = utils::bounded_scale<uint16_t, ScaleProfile::KP_RATIO>(val);
        return KpCode{static_cast<uint16_t>(bits)};
    }

    template<typename T>
    static constexpr KdCode make_kdcode(const T val){
        const auto bits = utils::bounded_scale<uint16_t, ScaleProfile::KD_RATIO>(val);
        return KdCode{static_cast<uint16_t>(bits)};
    }
};





}