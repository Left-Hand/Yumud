#pragma once

#include "algebra/gesture/twist2.hpp"

namespace ymd::robots::kinematics{
//这是一个差速双轮模型
struct [[nodiscard]] DifferentialDriveKinematics final { 
    using Self = DifferentialDriveKinematics;

    struct [[nodiscard]] Config final{
        uq16 wheel_radius;
        uq16 wheel_padding;

        constexpr Self into() const {
            const auto angular_body2wheel_ratio = wheel_padding / wheel_radius / iq16(2);
            const auto angular_wheel2body_ratio = iq16(1) / angular_body2wheel_ratio;
            
            const auto linear_body2wheel_ratio = 1 / wheel_radius;
            const auto linear_wheel2body_ratio = wheel_radius;
            return Self{
                .angular_body2wheel_ratio = angular_body2wheel_ratio,
                .angular_wheel2body_ratio = angular_wheel2body_ratio,
                .linear_body2wheel_ratio = linear_body2wheel_ratio,
                .linear_wheel2body_ratio = linear_wheel2body_ratio
            };
        }
    };

    iq16 angular_body2wheel_ratio;
    iq16 angular_wheel2body_ratio;

    iq16 linear_body2wheel_ratio;
    iq16 linear_wheel2body_ratio;

    //逆运动学 输入机体的横向和纵向任意阶量 返回左右轮的等阶量
    constexpr std::tuple<Angular<iq16>, Angular<iq16>> inverse(
        iq16 body_linear_xn, Angular<iq16> body_angular_xn 
    ) const { 

        const iq16 body_xn_radians = body_angular_xn.to_radians();

        //共模速度
        const auto wheel_common_xn_radians = linear_body2wheel_ratio * body_linear_xn;
        //差模速度
        const auto wheel_diff_xn_radians = angular_body2wheel_ratio * body_xn_radians;

        const auto left_motor_xn = Angular<iq16>::from_radians(
            wheel_common_xn_radians - wheel_diff_xn_radians);

        //右轮的正向旋转方向和车身正方向相反，需要镜像过来
        const auto right_motor_xn = Angular<iq16>::from_radians(
            -(wheel_common_xn_radians + wheel_diff_xn_radians));

        return {left_motor_xn, right_motor_xn};
    }
};

}