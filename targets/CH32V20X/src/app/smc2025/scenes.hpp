#pragma once

#include "render.hpp"
#include "types/gesture/camview2.hpp"


static constexpr real_t PIXELS_PER_METER = 10;
static constexpr real_t METERS_PER_PIXEL = 1 / PIXELS_PER_METER;


namespace ymd::smc::sim{

namespace details{
template<typename T>
[[nodiscard]] constexpr Isometry2<T> forward_move(const Isometry2<T> iso, const T length)  {
    const auto delta = iso.rotation.to_vec2(length);
    return Isometry2<T>{
        iso.rotation,
        iso.translation + delta
    };
}
template<typename T>
[[nodiscard]] constexpr Isometry2<T> side_move(const Isometry2<T> iso, const T length)  {
    const auto delta = iso.rotation.to_vec2(length).forward_90deg();
    return Isometry2<T>{
        iso.rotation,
        iso.translation + delta
    };
}
template<typename T>
[[nodiscard]] constexpr Isometry2<T> revolve_by_radius_and_rotation(
        const Isometry2<T> iso, const T radius, const Angle<T> angle)  {

    const auto v2 = iso.rotation.to_vec2(radius);
    const auto ar = angle.is_positive() ? v2.forward_90deg() : v2.backward_90deg();

    const auto org = iso.translation + ar;
    const auto delta = (-ar).rotated(angle);
    return Isometry2<T>{
        .rotation = iso.rotation * UnitComplex<T>::from_angle(angle),
        .translation =  org + delta, 
    };
}
}

class BlueprintSpawner{
public:
    struct Config{
        real_t road_width;
    };

    constexpr BlueprintSpawner(const Config & cfg, const Isometry2<real_t> & entrypoint):
        viewpoint_(entrypoint){
            reconf(cfg);
        }

    constexpr void reconf(const Config & cfg){
        road_width_ = cfg.road_width;
    }
    [[nodiscard]] constexpr ElementWithPlacement<AnnularSector<iq16, iq16>> spawn_annular_sector(
        const real_t radius, const Angle<real_t> angle){
        ASSERT(radius > 0);
        const auto v_angle = viewpoint_.rotation.to_angle();
        const Angle<real_t> start_angle = (((angle > 0_deg) ? 
                (v_angle - 90_deg)
                : (v_angle + angle + 90_deg)));

        const Angle<real_t> stop_angle = ((angle > 0_deg) ? 
                (v_angle + angle - 90_deg)
                : (v_angle + 90_deg));



        const auto shape = AnnularSector<iq16, iq16>{
            .center = Vec2<iq16>::ZERO,
            .radius_range = Range2<iq16>::from_center_and_half_length(
                radius, road_width_ / 2),
            
            .angle_range = AngleRange<iq16>::from_start_and_stop(start_angle, stop_angle)
        }; 
        
        const auto place = Placement{
            .translation = details::side_move(viewpoint_,
                (angle > 0_deg) ? (radius) : (-radius)).translation
        };

        viewpoint_ = details::revolve_by_radius_and_rotation(
            viewpoint_, radius, angle
        );

        return shape | place;
    }

    [[nodiscard]] constexpr auto spawn_stright(const real_t length){
        ASSERT(length > 0);
        const auto v_angle = viewpoint_.rotation.to_angle();
        const auto ret = RotatedRect<real_t>{
            .width = road_width_,
            .height = length,
            .rotation = v_angle - 90_deg
        } | Placement{
            .translation = details::forward_move(viewpoint_, length / 2).translation,
        };

        viewpoint_ = details::forward_move(viewpoint_, length);

        return ret;
    }

    [[nodiscard]] constexpr auto spawn_zebra_stright(const real_t length){
        ASSERT(length > 0);
        const auto v_angle = viewpoint_.rotation.to_angle();
        const auto ret = RotatedZebraRect{
            .width = road_width_,
            .height = length,
            .rotation = v_angle - 90_deg
        } | Placement{
            .translation = details::forward_move(viewpoint_, length / 2).translation,
        };

        viewpoint_ = details::forward_move(viewpoint_, length);

        return ret;
    }
private:
    Isometry2<real_t> viewpoint_;
    real_t road_width_;
};

struct Scenes{
    __no_inline static Image<Gray> render_scene1
        (const CamView2<real_t> & viewport);    
    __no_inline static Image<Gray> render_scene2
        (const CamView2<real_t> & viewport);    
};

}