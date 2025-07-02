#pragma once

#include "render.hpp"


static constexpr real_t PIXELS_PER_METER = 10;
static constexpr real_t METERS_PER_PIXEL = 1 / PIXELS_PER_METER;


namespace ymd::smc::sim{

template<typename T>
struct CameraViewport2_t{
    Pose2<T> pose;
    T zoom;
};


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
    [[nodiscard]] constexpr auto spawn_annular_sector(const real_t radius, const real_t rotation){
        ASSERT(radius > 0);
        const auto start_rad = ((rotation > 0) ? 
                (viewpoint_.rad - real_t(PI/2))
                : (viewpoint_.rad + rotation + real_t(PI/2)));
        const auto stop_rad = ((rotation > 0) ? 
                (viewpoint_.rad + rotation - real_t(PI/2))
                : (viewpoint_.rad + real_t(PI/2)));

        const auto ret = AnnularSector{
            .inner_radius = radius - road_width_ / 2,
            .outer_radius = radius + road_width_ / 2,
            
            .start_rad = start_rad,
            .stop_rad = stop_rad,
        } | Placement{
            .pos = viewpoint_.side_move((rotation > 0) ? (radius) : (-radius)).pos
        };

        viewpoint_ = viewpoint_.revolve_by_radius_and_rotation(
            radius, rotation
        );

        return ret;
    }

    [[nodiscard]] constexpr auto spawn_stright(const real_t length){
        ASSERT(length > 0);
        const auto ret = RotatedRect{
            .width = road_width_,
            .height = length,
            .rotation = viewpoint_.rad - real_t(PI / 2)
        } | Placement{
            .pos = viewpoint_.forward_move(length / 2).pos,
        };

        viewpoint_ = viewpoint_.forward_move(length);

        return ret;
    }

    [[nodiscard]] constexpr auto spawn_zebra_stright(const real_t length){
        ASSERT(length > 0);
        const auto ret = RotatedZebraRect{
            .width = road_width_,
            .height = length,
            .rotation = viewpoint_.rad - real_t(PI / 2)
        } | Placement{
            .pos = viewpoint_.forward_move(length / 2).pos,
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
        (const CameraViewport2_t<real_t> & viewport);    
    __no_inline static Image<Gray> render_scene2
        (const CameraViewport2_t<real_t> & viewport);    
};

}