#pragma once

#include "core/math/real.hpp"
#include "algebra/vectors/vec2.hpp"
#include "algebra/vectors/vec3.hpp"
#include "algebra/regions/plane.hpp"
#include "algebra/regions/aabb.hpp"
#include "algebra/transforms/transform3d.hpp"




namespace ymd::robots::kinematics{

template<typename T>
struct [[nodiscard]] WheelLegKinematics{
public:
    struct [[nodiscard]] Config{
        const T pelvis_length;
        const T thigh_length;
        const T shin_length;
    };

protected:
// public:
    const Config & cfg_;

    constexpr auto d2_forward_leg(const T hip_rad, const T knee_rad) const {
        auto knee_position = math::Vec2<T>(cfg_.thigh_length, 0).rotated(-hip_rad);
        auto feet_position = knee_position + math::Vec2<T>(cfg_.shin_length_mster, 0).rotated(T(M_PI)-hip_rad - knee_rad);
        return std::make_tuple(knee_position, feet_position);
    }

    constexpr auto d2_inverse_leg(const math::Vec2<T> & foot_position) const {
        auto knee_temp = (square(cfg_.thigh_length) + square(cfg_.shin_length) - foot_position.length_squareared()) / 
            (2 * cfg_.thigh_length * cfg_.shin_length);
        if(ABS(knee_temp) > 1 + T(CMP_EPSILON)) return std::nullopt;
        
        auto knee_rad = acos(
            knee_temp
        );

        auto hip_temp = (square(cfg_.thigh_length) + foot_position.length_squareared() - square(cfg_.shin_length)) / 
            (2 * cfg_.thigh_length * foot_position.length());

        if(ABS(hip_temp) > 1 + T(CMP_EPSILON)) return std::nullopt;

        auto hip_rad = acos(
            hip_temp
        ) + (- foot_position.angle());

        return std::make_tuple(hip_rad, knee_rad);
    }

    struct [[nodiscard]] Viewer{    
        const WheelLegKinematics<T> & solver;
    };

    struct [[nodiscard]] GroundViewer:public Viewer{
        constexpr GroundViewer(const WheelLegKinematics<T> & solver):Viewer{solver} {;}
        auto pelvis_transform(
            const math::Vec3<T> & left_feet_position, const math::Vec3<T> & right_feet_position, const Angular<T> pitch_angle) const {
            return Viewer::solver.transform_ground_to_pelvis(left_feet_position, right_feet_position, pitch_angle);
        }
    };

    struct [[nodiscard]] PelvisViewer:public Viewer{
        constexpr PelvisViewer(const WheelLegKinematics<T> & solver):Viewer{solver} {;}
        constexpr auto ground_transform(
            const math::Vec3<T> & left_feet_position, const math::Vec3<T> & right_feet_position, const Angular<T> pitch_angle) const {
            return Viewer::solver.transform_pelvis_to_ground(left_feet_position, right_feet_position, pitch_angle);
        }
    };

    friend struct GroundViewer;
    friend struct PelvisViewer;

public:
    constexpr WheelLegKinematics(const Config & cfg): cfg_(cfg){;}

    [[nodiscard]] constexpr auto d3_forward_leg(const T hip_rad, const T knee_rad, bool is_right) const {
        auto hip_position = math::Vec3<T>(is_right ? (cfg_.pelvis_length / 2) : (- cfg_.pelvis_length / 2), 0, 0);
        auto [d2_knee_position, d2_feet_position] = d2_forward_leg(hip_rad, knee_rad);

        auto transform_d2_to_d3 = [](const math::Vec2<T> & d2_position) -> math::Vec3<T> {return math::Vec3<T>(0, d2_position.y, -d2_position.x);};
        auto knee_position = hip_position + transform_d2_to_d3(d2_knee_position);
        auto feet_position = hip_position + transform_d2_to_d3(d2_feet_position);
        return std::make_tuple(knee_position, feet_position);
    }

    [[nodiscard]] constexpr auto d3_inverse_leg(const math::Vec3<T> feet_position, bool is_right) const {
        auto hip_position = math::Vec3<T>(is_right ? (cfg_.pelvis_length / 2) : (- cfg_.pelvis_length / 2), 0, 0);
        auto delta_position = feet_position - hip_position;
        auto transform_d3_to_d2 = [](const math::Vec3<T> & d3_position) -> math::Vec2<T> {return math::Vec2<T>(-d3_position.x, d3_position.y);};
        return d2_inverse_leg(transform_d3_to_d2(delta_position));
    }

    [[nodiscard]] constexpr auto foot_plane(const math::Vec3<T> & left_feet_position, const math::Vec3<T> & right_feet_position, const Angular<T> pitch_angle) const {
        auto d2_helper_point = math::Vec2<T>(1,0).rotated(pitch_angle);
        auto helper_point = math::Vec3<T>(0, d2_helper_point.y, -d2_helper_point.x);
        return Plane<T>(left_feet_position, right_feet_position, left_feet_position + helper_point);
    }

    [[nodiscard]] constexpr auto body_aabb(const math::Vec3<T> & left_feet_position, const math::Vec3<T> & right_feet_position) const {
        return AABB<T>(left_feet_position).expand(right_feet_position);
    }

    [[nodiscard]] constexpr auto quat_pelvis_to_ground(
        const math::Vec3<T> & left_feet_position, 
        const math::Vec3<T> & right_feet_position, 
        const Angular<T> pitch_angle
    ) const {
        auto plane = foot_plane(left_feet_position, right_feet_position, pitch_angle);
        return Quat<T>(math::Vec3<T>(0, 1, 0), plane.normal);
    }

    [[nodiscard]] constexpr auto transform_pelvis_to_ground(
        const math::Vec3<T> & left_feet_position, 
        const math::Vec3<T> & right_feet_position, 
        const Angular<T> pitch_angle
    ) const {
        auto midpoint_position = (left_feet_position + right_feet_position)/2;
        return Transform3D<T>(Basis<T>(quat_pelvis_to_ground(left_feet_position, right_feet_position, pitch_angle)), midpoint_position);
    }

    [[nodiscard]] constexpr auto quat_ground_to_pelvis(
        const math::Vec3<T> & left_feet_position, 
        const math::Vec3<T> & right_feet_position, 
        const Angular<T> pitch_angle
    ) const {
        return quat_pelvis_to_ground(left_feet_position, right_feet_position, pitch_angle).inverse();
    }

    [[nodiscard]] constexpr auto transform_ground_to_pelvis(
        const math::Vec3<T> & left_feet_position, 
        const math::Vec3<T> & right_feet_position, 
        const Angular<T> pitch_angle
    ) const {
        return transform_pelvis_to_ground(left_feet_position, right_feet_position, pitch_angle).inverse();
    }

    [[nodiscard]] constexpr auto ground_viewer() const {
        return GroundViewer(*this);
    }

    [[nodiscard]] constexpr auto pelvis_viewer() const {
        return PelvisViewer(*this);
    }
};

}
