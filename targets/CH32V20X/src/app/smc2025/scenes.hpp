#pragma once

#include "render.hpp"
#include "types/gesture/camview2.hpp"


static constexpr real_t PIXELS_PER_METER = 10;
static constexpr real_t METERS_PER_PIXEL = 1 / PIXELS_PER_METER;


namespace ymd::smc::sim{



class BlueprintSpawner{
public:
    struct Config{
        real_t road_width;
    };

    constexpr BlueprintSpawner(const Config & cfg, const Pose2<real_t> & entrypoint):
        viewpoint_(entrypoint){
            reconf(cfg);
        }

    constexpr void reconf(const Config & cfg){
        road_width_ = cfg.road_width;
    }
    [[nodiscard]] constexpr ElementWithPlacement<AnnularSector<q16, q16>> spawn_annular_sector(const real_t radius, const Angle<real_t> orientation){
        ASSERT(radius > 0);
        const Angle<real_t> start_angle = ((orientation > 0_deg) ? 
                (viewpoint_.orientation - 90_deg)
                : (viewpoint_.orientation + orientation + 90_deg));

        const Angle<real_t> stop_angle = ((orientation > 0_deg) ? 
                (viewpoint_.orientation + orientation - 90_deg)
                : (viewpoint_.orientation + 90_deg));



        const auto shape = AnnularSector<q16, q16>{
            .center = Vec2<q16>::ZERO,
            .radius_range = Range2<q16>::from_center_and_half_length(
                radius, road_width_ / 2),
            
            .angle_range = AngleRange<q16>::from_start_and_stop(start_angle, stop_angle)
        }; 
        
        const auto place = Placement{
            .position = viewpoint_.side_move(
                (orientation > 0_deg) ? (radius) : (-radius)).position
        };

        viewpoint_ = viewpoint_.revolve_by_radius_and_rotation(
            radius, orientation
        );

        return shape | place;
    }

    [[nodiscard]] constexpr auto spawn_stright(const real_t length){
        ASSERT(length > 0);
        const auto ret = RotatedRect<real_t>{
            .width = road_width_,
            .height = length,
            .orientation = viewpoint_.orientation - 90_deg
        } | Placement{
            .position = viewpoint_.forward_move(length / 2).position,
        };

        viewpoint_ = viewpoint_.forward_move(length);

        return ret;
    }

    [[nodiscard]] constexpr auto spawn_zebra_stright(const real_t length){
        ASSERT(length > 0);
        const auto ret = RotatedZebraRect{
            .width = road_width_,
            .height = length,
            .orientation = viewpoint_.orientation - 90_deg
        } | Placement{
            .position = viewpoint_.forward_move(length / 2).position,
        };

        viewpoint_ = viewpoint_.forward_move(length);

        return ret;
    }
private:
    Pose2<real_t> viewpoint_;
    real_t road_width_;
};

struct Scenes{
    __no_inline static Image<Gray> render_scene1
        (const CamView2<real_t> & viewport);    
    __no_inline static Image<Gray> render_scene2
        (const CamView2<real_t> & viewport);    
};

}