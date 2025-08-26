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
    [[nodiscard]] constexpr auto spawn_annular_sector(const real_t radius, const real_t orientation){
        ASSERT(radius > 0);
        const auto start_angle = ((orientation > 0) ? 
                (viewpoint_.orientation - real_t(PI/2))
                : (viewpoint_.orientation + orientation + real_t(PI/2)));
        const auto stop_angle = ((orientation > 0) ? 
                (viewpoint_.orientation + orientation - real_t(PI/2))
                : (viewpoint_.orientation + real_t(PI/2)));

        const auto ret = AnnularSector<q16>{
            .inner_radius = radius - road_width_ / 2,
            .outer_radius = radius + road_width_ / 2,
            
            .angle_range = {start_angle, stop_angle}
        } | Placement{
            .position = viewpoint_.side_move((orientation > 0) ? (radius) : (-radius)).position
        };

        viewpoint_ = viewpoint_.revolve_by_radius_and_rotation(
            radius, orientation
        );

        return ret;
    }

    [[nodiscard]] constexpr auto spawn_stright(const real_t length){
        ASSERT(length > 0);
        const auto ret = RotatedRect<real_t>{
            .width = road_width_,
            .height = length,
            .orientation = viewpoint_.orientation - real_t(PI / 2)
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
            .orientation = viewpoint_.orientation - real_t(PI / 2)
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