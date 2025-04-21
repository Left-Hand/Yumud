#pragma once

#include "../kinematics_utils.hpp"

#include <cmath>
#include <array>
#include "types/ray2/ray2.hpp"
#include "types/vector3/Vector3.hpp"
#include "types/quat/Quat.hpp"



namespace ymd::robots{

template<arithmetic T>
// class Kinematics<T, 2, KinematicPairs::Revolute, KinematicPairs::Revolute>{
class RR2_Kinematics{
public:
    // static constexpr auto name = "Cylindrical";
    // static constexpr auto num_joints = 2;
    // static constexpr auto num_links = 3;
    // static constexpr auto num_dofs = 1;
    // static constexpr auto num_params = 0;

    // static constexpr auto joint_types = std::array{JointType::Revolute, JointType::Prismatic};

    struct Config{
        T base_length;
        T link_length;
    };

    struct Vars{
        T j1_rad;
        T j2_rad;
    };

    static constexpr Vector2_t<T> forward(const Config &config, const Vars &vars, const Vector2_t<T> org){
        const auto [B, L] = config;
        const auto [R1, R2] = vars;
        return Vector2_t<T>(B, 0).rotated(R1) + Vector2_t<T>(L, 0).rotated(R2);
    }

    static constexpr Option<Vars> inverse(const Config &config, const Vector2_t<T> dest, const Vector2_t<T> org){
        #if false
        // const auto [B, L] = config;
        // using std::atan2; using std::acos; using std::sqrt; using std::cos; using std::sin;
        const auto pos = dest - org;
        T B = config.base_length;
        T L = config.link_length;
        T x = pos.x;
        T y = pos.y;
    
        // Check reachability (simplified for brevity)
        T numerator = x*x + y*y - B*B - L*L;
        T denominator = 2 * B * L;
        T cos_delta = numerator / denominator;
        if (std::abs(cos_delta) > 1 + 1e-6) {
            // Handle unreachable position (e.g., throw exception)
            // For brevity, assume valid input
        }
    
        // Compute angles
        T delta = acos(cos_delta);
        T R = sqrt(x*x + y*y);
        T phi = atan2(y, x);
        T C = numerator / (2 * B);
        T cos_term = C / R;
        T angle_diff = acos(cos_term);
    
        // Two possible solutions for θ₁
        T theta1_1 = phi + angle_diff;
        T theta1_2 = phi - angle_diff;
    
        // Compute corresponding θ₂ values
        auto compute_theta2 = [&](T theta1) {
            T dx = x - B * cos(theta1);
            T dy = y - B * sin(theta1);
            return atan2(dy, dx);
        };
    
        T theta2_1 = compute_theta2(theta1_1);
        T theta2_2 = compute_theta2(theta1_2);
    
        // Return first solution (modify to return all solutions if needed)
        return Vars{theta1_1, theta2_1};
        #else
        const auto pos = dest - org;
        return Some(Vars{0, 0});
        // return None;
        #endif
    }
};


template<arithmetic T>
class RRS_Kinematics{
public:
    struct Config{
        T base_length;//基摇臂长度
        T link_length;///摇臂长度
        T base_plate_radius;//基座半径
        T top_plate_radius;///顶座半径
    };

    struct  Gesture{
        Quat_t<T> orientation;
        T z;
    };


    using Vars2 = typename RR2_Kinematics<T>::Vars;

    //evil fn
    constexpr Option<std::array<Vars2, 3>> inverse(const Gesture & gest) const{
        const auto vars_0 = inverse_side_axis(cfg_, norms_[0], gest);
        const auto vars_1 = inverse_side_axis(cfg_, norms_[1], gest);
        const auto vars_2 = inverse_side_axis(cfg_, norms_[2], gest);
        if(vars_0.is_some() && vars_1.is_some() && vars_2.is_some()){
            return Some(std::array<Vars2, 3>{
                vars_0.unwrap(),
                vars_1.unwrap(),
                vars_2.unwrap(),
            });
        }else{
            return None;
        }
    }

    constexpr RRS_Kinematics(const Config & cfg){
        reconf(cfg);
    }

    //evil fn
    constexpr void reconf(const Config & cfg){
        cfg_ = cfg;

        for(size_t i = 0; i < 3; i++)
            norms_[i] = get_xynorm_from_idx(cfg, i);
    }
private:
    Config cfg_ {};

    std::array<Vector2_t<T>, 3> norms_ {};

    static constexpr Vector2_t<T> get_xynorm_from_idx(const Config & cfg, const size_t idx){
        return Vector2_t<T>(cfg.base_plate_radius,0).rotated(idx * T(TAU / 3));
    };

    //pure fn
    static constexpr Vector3_t<T> get_base_point(const Config & cfg, const Vector2_t<T> xy_norm){
        const auto base_point_2 = Vector2_t<T>{cfg.base_plate_radius, 0}.improduct(xy_norm);
        return Vector3_t<T>{base_point_2.x, base_point_2.y, 0};
    }

    //pure fn
    static constexpr Vector3_t<T> get_top_point(const Config & cfg, const Vector2_t<T> xy_norm, const Gesture & gest){
        const auto top_point_2 = Vector2_t<T>{cfg.top_plate_radius, 0}.improduct(xy_norm);
        return gest.orientation.xform(Vector3_t<T>{top_point_2.x, top_point_2.y, 0}) + Vector3_t<T>(0, 0, gest.z);
    }

    // struct RhPoint{
    //     T r;
    //     T h;
    // };
    struct Ball{
        Vector3_t<T> org;
        T radius;
    };

    struct CirclePassZAxis{
        //这个圆所在的平面包含z轴 自由度特殊化
        Vector3_t<T> org;
        T radius;
    };

    //pure fn
    static constexpr Ball get_top_ball(const Config & cfg, const Vector2_t<T> xy_norm, const Gesture & gest){
        return Ball{get_top_point(cfg, xy_norm, gest), cfg.link_length};
    }

    //pure fn
    static constexpr CirclePassZAxis project_ball_to_circle(const Ball & ball, const Vector2_t<T> xy_norm){
        // [x', y'] = xy_norm

        // x / x' = y / y'
        // => x' * x - y' * y = 0
        // => dist = |x' * x - y' * y| / sqrt(x'^2 + y'^2)
        // => dist = |x' * x - y' * y|
        const T xy_dist = std::abs(xy_norm.x * ball.org.x - xy_norm.y * ball.org.y);
        const T circle_radius = std::sqrt(ball.radius * ball.radius - xy_dist * xy_dist);

        return CirclePassZAxis{
            ball.org, 
            circle_radius
        };
    }


    // static constexpr Vector2_t<T> rhpoint_to_vec2(const RhPoint & rhpoint){
    //     return Vector2_t<T>{rhpoint.r, rhpoint.h};
    // }

    //pure fn
    static constexpr Option<Vars2> inverse_side_axis(const Config & cfg, const Vector2_t<T> xy_norm, const Gesture & gest){
        const auto ball = get_top_ball(cfg, xy_norm, gest);
        const auto circle = project_ball_to_circle(ball, xy_norm);
        return RR2_Kinematics<T>::inverse(
            //大臂长度和小臂长度
            {.base_length = cfg.base_length, .link_length = circle.radius}, 

            //目的位置
            Vector2_t<T>{imag(circle.org.x, circle.org.y), circle.org.z},
            //基位置
            Vector2_t<T>{cfg.base_plate_radius, 0}
        );
    }

};

}