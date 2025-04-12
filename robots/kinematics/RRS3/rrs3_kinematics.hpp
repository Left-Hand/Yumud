#pragma once

#include "../kinematics_utils.hpp"
#include "types/ray2/ray2.hpp"
#include "types/vector3/Vector3.hpp"
#include "types/quat/Quat.hpp"


namespace ymd::robots{

template<arithmetic T>
// class Kinematics<T, 2, KinematicPairs::Revolute, KinematicPairs::Revolute>{
class RR_Kinematics2{
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

    static constexpr Vector2_t<T> forward(const Config &config, const Vars &vars){
        const auto [B, L] = config;
        const auto [R1, R2] = vars;
        return Vector2_t<T>(B, 0).rotated(R1) + Vector2_t<T>(L, 0).rotated(R2);
    }

    static constexpr Vars inverse(const Config &config, const Vector2_t<T> &pos){
        // const auto [B, L] = config;
        // using std::atan2; using std::acos; using std::sqrt; using std::cos; using std::sin;

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
    }
};


template<arithmetic T>
class RRS_Kinematics{
private:
    struct Config{
        T base_length;
        T link_length;
        T base_plate_radius;
        T top_plate_radius;
    };

    struct  Gesture{
        Quat_t<T> orientation;
        T height;
    };
    

    static constexpr Vector3_t<T> get_base_point(const Config & cfg, const uint idx){
        const auto base_point_2 = Vector2_t<T>{cfg.base_plate_radius, 0}.rotated(idx * T(2 * PI / 3));
        return Vector3_t<T>{base_point_2.x, base_point_2.y, 0};
    }

    static constexpr Vector3_t<T> get_top_point(const Config & cfg, const uint idx, const Gesture & gest){
        // const auto p_ori = gest.orientation * 
        // const auto up_center = Vector3_t<T>(0, 0, gest.height) + gest.orientation.xform(Vector3_t<T>(0, 0, cfg.base_plate_radius));
        const auto top_point_2 = Vector2_t<T>{cfg.top_plate_radius, 0}.rotated(idx * T(2 * PI / 3));
        return Vector3_t<T>{top_point_2.x, top_point_2.y, 0};
    }

};

}