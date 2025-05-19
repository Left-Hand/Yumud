#pragma once

#include "render.hpp"


static constexpr real_t PIXELS_PER_METER = 10;
static constexpr real_t METERS_PER_PIXEL = 1 / PIXELS_PER_METER;
static constexpr auto ROAD_WIDTH = 0.475_r; 


namespace ymd::smc::sim{
class BlueprintBuilder{
public:
    constexpr BlueprintBuilder(const Gest2_t<real_t> & entrypoint):
        viewpoint_(entrypoint){;}

    [[nodiscard]] constexpr auto add_annular_sector(const real_t radius, const real_t rotation){
        ASSERT(radius > 0);
        const auto start_rad = ((rotation > 0) ? 
                (viewpoint_.rad - real_t(PI/2))
                : (viewpoint_.rad + rotation + real_t(PI/2)));
        const auto stop_rad = ((rotation > 0) ? 
                (viewpoint_.rad + rotation - real_t(PI/2))
                : (viewpoint_.rad + real_t(PI/2)));

        const auto ret = AnnularSector{
            .inner_radius = radius - ROAD_WIDTH / 2,
            .outer_radius = radius + ROAD_WIDTH / 2,
            
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

    [[nodiscard]] constexpr auto add_stright(const real_t length){
        ASSERT(length > 0);
        const auto ret = RotatedRect{
            .width = ROAD_WIDTH,
            .height = length,
            .rotation = viewpoint_.rad - real_t(PI / 2)
        } | Placement{
            .pos = viewpoint_.forward_move(length / 2).pos,
        };

        viewpoint_ = viewpoint_.forward_move(length);

        return ret;
    }
private:
    Gest2_t<real_t> viewpoint_;
};

struct Scenes{
    __no_inline static Image<Grayscale> render_scene1
        (const Gest2_t<real_t> viewpoint, const real_t scale);    
};

}