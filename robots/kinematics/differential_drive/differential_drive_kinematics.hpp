#pragma once

#include "algebra/gesture/twist2.hpp"

namespace ymd::robots::kinematics{
//这是一个差速双轮模型
struct [[nodiscard]] DifferentialDriveKinematics final { 
    using Self = DifferentialDriveKinematics;

    struct [[nodiscard]] Config final{
        uq16 wheel_radius;
        uq16 track_width;

        constexpr Self into() const {
            const uq16 angular_body2wheel_ratio = (track_width / (wheel_radius * 2));
            const uq16 angular_wheel2body_ratio = uq16(1) / angular_body2wheel_ratio;
            
            const uq16 linear_body2wheel_ratio = 1 / wheel_radius;
            const uq16 linear_wheel2body_ratio = wheel_radius;
            return Self{
                .angular_body2wheel_ratio = angular_body2wheel_ratio,
                .angular_wheel2body_ratio = angular_wheel2body_ratio,
                .linear_body2wheel_ratio = linear_body2wheel_ratio,
                .linear_wheel2body_ratio = linear_wheel2body_ratio
            };
        }
    };

    uq16 angular_body2wheel_ratio;
    uq16 angular_wheel2body_ratio;

    uq16 linear_body2wheel_ratio;
    uq16 linear_wheel2body_ratio;

    //逆运动学 输入机体的纵向速度和自转速度 得到轮速度
    constexpr std::tuple<Angular<iq16>, Angular<iq16>> inverse(
        iq16 body_linear_x2, Angular<iq16> body_angular_x2 
    ) const { 

        const iq16 body_xn_radians = body_angular_x2.to_radians();

        //共模速度
        const auto wheel_common_xn_radians = linear_body2wheel_ratio * body_linear_x2;
        //差模速度
        const auto wheel_diff_xn_radians = angular_body2wheel_ratio * body_xn_radians;

        const auto left_motor_xn = Angular<iq16>::from_radians(
            wheel_common_xn_radians - wheel_diff_xn_radians);

        const auto right_motor_xn = Angular<iq16>::from_radians(
            (wheel_common_xn_radians + wheel_diff_xn_radians));

        return {left_motor_xn, right_motor_xn};
    }
};


struct [[nodiscard]] DifferentialDriveKinematicsF final { 
    using Self = DifferentialDriveKinematicsF;

    struct [[nodiscard]] Config final{
        float wheel_radius;
        float track_width;

        constexpr Self into() const {
            const float angular_body2wheel_ratio = (track_width / (wheel_radius * 2));
            const float angular_wheel2body_ratio = float(1) / angular_body2wheel_ratio;
            
            const float linear_body2wheel_ratio = 1 / wheel_radius;
            const float linear_wheel2body_ratio = wheel_radius;
            return Self{
                .angular_body2wheel_ratio = angular_body2wheel_ratio,
                .angular_wheel2body_ratio = angular_wheel2body_ratio,
                .linear_body2wheel_ratio = linear_body2wheel_ratio,
                .linear_wheel2body_ratio = linear_wheel2body_ratio
            };
        }
    };

    float angular_body2wheel_ratio;
    float angular_wheel2body_ratio;

    float linear_body2wheel_ratio;
    float linear_wheel2body_ratio;

    
    //逆运动学 输入机体的纵向速度和自转速度 得到左右轮速度
    constexpr std::tuple<Angular<float>, Angular<float>> inverse(
        float body_linear_x2, 
        Angular<float> body_angular_x2 
    ) const { 

        const float body_xn_radians = body_angular_x2.to_radians();

        //共模速度
        const auto wheel_common_xn_radians = linear_body2wheel_ratio * body_linear_x2;
        //差模速度
        const auto wheel_diff_xn_radians = angular_body2wheel_ratio * body_xn_radians;

        const auto left_motor_xn = Angular<float>::from_radians(
            wheel_common_xn_radians - wheel_diff_xn_radians);

        const auto right_motor_xn = Angular<float>::from_radians(
            (wheel_common_xn_radians + wheel_diff_xn_radians));

        return {left_motor_xn, right_motor_xn};
    }
};

}