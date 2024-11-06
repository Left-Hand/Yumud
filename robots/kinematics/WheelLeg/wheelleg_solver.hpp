#pragma once

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"
#include "types/vector3/vector3_t.hpp"
#include "types/plane/plane_t.hpp"
#include "types/aabb/aabb_t.hpp"
#include "transform3d/transform3d_t.hpp"

#define SQU(x) ((x) * (x)) 



namespace yumud{

template<typename T>
class WheelLegSolver_t{
public:
    struct Config{
        const T pelvis_length_meter;
        const T thigh_length_meter;
        const T shin_length_meter;
    };

protected:
// public:
    const Config & config;

    constexpr auto d2_forward_leg(const T hip_rad, const T knee_rad) const {
        auto knee_pos = Vector2_t<T>(config.thigh_length_meter, 0).rotated(-hip_rad);
        auto feet_pos = knee_pos + Vector2_t<T>(config.shin_length_mster, 0).rotated(T(PI)-hip_rad - knee_rad);
        return std::make_tuple(knee_pos, feet_pos);
    }

    constexpr auto d2_inverse_leg(const Vector2_t<T> & foot_pos) const {
        auto knee_temp = (SQU(config.thigh_length_meter) + SQU(config.shin_length_meter) - foot_pos.length_squared()) / 
            (2 * config.thigh_length_meter * config.shin_length_meter);
        if(ABS(knee_temp) > 1 + T(CMP_EPSILON)) return std::nullopt;
        
        auto knee_rad = acos(
            knee_temp
        );

        auto hip_temp = (SQU(config.thigh_length_meter) + foot_pos.length_squared() - SQU(config.shin_length_meter)) / 
            (2 * config.thigh_length_meter * foot_pos.length());

        if(ABS(hip_temp) > 1 + T(CMP_EPSILON)) return std::nullopt;

        auto hip_rad = acos(
            hip_temp
        ) + (- foot_pos.angle());

        return std::make_tuple(hip_rad, knee_rad);
    }

    struct Viewer{    
        const WheelLegSolver_t<T> & solver;
        Viewer(const WheelLegSolver_t<T> & _solver):solver(_solver) {;}
    };

    struct GroundViewer:public Viewer{
        GroundViewer(const WheelLegSolver_t<T> & _solver):Viewer(_solver) {;}
        auto get_pelvis_transform(const Vector3_t<T> & left_feet_pos, const Vector3_t<T> & right_feet_pos, const T pitch_rad) const {
            return Viewer::solver.transform_ground_to_pelvis(left_feet_pos, right_feet_pos, pitch_rad);
        }
    };

    struct PelvisViewer:public Viewer{
        PelvisViewer(const WheelLegSolver_t<T> & _solver):Viewer(_solver) {;}
        auto get_ground_transform(const Vector3_t<T> & left_feet_pos, const Vector3_t<T> & right_feet_pos, const T pitch_rad) const {
            return Viewer::solver.transform_pelvis_to_ground(left_feet_pos, right_feet_pos, pitch_rad);
        }
    };

    friend class GroundViewer;
    friend class PelvisViewer;

public:
    constexpr WheelLegSolver_t(const Config & _config): config(_config){;}

    constexpr auto d3_forward_leg(const T hip_rad, const T knee_rad, bool is_right) const {
        auto hip_pos = Vector3_t<T>(is_right ? (config.pelvis_length_meter / 2) : (- config.pelvis_length_meter / 2), 0, 0);
        auto [d2_knee_pos, d2_feet_pos] = d2_forward_leg(hip_rad, knee_rad);

        auto transform_d2_to_d3 = [](const Vector2_t<T> & d2_pos) -> Vector3_t<T> {return Vector3_t<T>(0, d2_pos.y, -d2_pos.x);};
        auto knee_pos = hip_pos + transform_d2_to_d3(d2_knee_pos);
        auto feet_pos = hip_pos + transform_d2_to_d3(d2_feet_pos);
        return std::make_tuple(knee_pos, feet_pos);
    }

    constexpr auto d3_inverse_leg(const Vector3_t<T> feet_pos, bool is_right) const {
        auto hip_pos = Vector3_t<T>(is_right ? (config.pelvis_length_meter / 2) : (- config.pelvis_length_meter / 2), 0, 0);
        auto delta_pos = feet_pos - hip_pos;
        auto transform_d3_to_d2 = [](const Vector3_t<T> & d3_pos) -> Vector2_t<T> {return Vector2_t<T>(-d3_pos.x, d3_pos.y);};
        return d2_inverse_leg(transform_d3_to_d2(delta_pos));
    }

    constexpr auto foot_plane(const Vector3_t<T> & left_feet_pos, const Vector3_t<T> & right_feet_pos, const T pitch_rad) const {
        auto d2_helper_point = Vector2_t<T>(1,0).rotated(pitch_rad);
        auto helper_point = Vector3_t<T>(0, d2_helper_point.y, -d2_helper_point.x);
        return Plane_t<T>(left_feet_pos, right_feet_pos, left_feet_pos + helper_point);
    }

    constexpr auto body_aabb(const Vector3_t<T> & left_feet_pos, const Vector3_t<T> & right_feet_pos) const {
        return AABB_t<T>(left_feet_pos).expand(right_feet_pos);
    }

    constexpr auto quat_pelvis_to_ground(const Vector3_t<T> & left_feet_pos, const Vector3_t<T> & right_feet_pos, const T pitch_rad) const {
        auto plane = foot_plane(left_feet_pos, right_feet_pos, pitch_rad);
        return Quat_t<T>(Vector3_t<T>(0, 1, 0), plane.normal);
    }

    constexpr auto transform_pelvis_to_ground(const Vector3_t<T> & left_feet_pos, const Vector3_t<T> & right_feet_pos, const T pitch_rad) const {
        auto midpoint_pos = (left_feet_pos + right_feet_pos)/2;
        return Transform3D_t<T>(Basis_t<T>(quat_pelvis_to_ground(left_feet_pos, right_feet_pos, pitch_rad)), midpoint_pos);
    }

    constexpr auto quat_ground_to_pelvis(const Vector3_t<T> & left_feet_pos, const Vector3_t<T> & right_feet_pos, const T pitch_rad) const {
        return quat_pelvis_to_ground(left_feet_pos, right_feet_pos, pitch_rad).inverse();
    }

    constexpr auto transform_ground_to_pelvis(const Vector3_t<T> & left_feet_pos, const Vector3_t<T> & right_feet_pos, const T pitch_rad) const {
        return transform_pelvis_to_ground(left_feet_pos, right_feet_pos, pitch_rad).inverse();
    }

    constexpr auto get_ground_viewer() const {
        return GroundViewer(*this);
    }

    constexpr auto get_pelvis_viewer() const {
        return PelvisViewer(*this);
    }
};

}

#undef SQU