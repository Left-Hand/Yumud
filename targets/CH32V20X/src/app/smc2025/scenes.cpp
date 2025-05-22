#include "scenes.hpp"

namespace ymd::smc::sim{

static constexpr auto make_scene1(){

    BlueprintBuilder builder = {
        Pose2_t<real_t>{Vector2_t<real_t>{0, 0}, real_t(PI)}
    };
    return make_static_scene(
        builder.add_stright(4.00_r)
        ,builder.add_annular_sector(0.5_r, 90_deg)
        ,builder.add_stright(3.55_r)
        ,builder.add_annular_sector(0.5_r, 90_deg)
        ,builder.add_stright(0.75_r)
        ,builder.add_annular_sector(0.5_r, 90_deg)
        ,builder.add_stright(1.45_r)
        ,builder.add_annular_sector(0.5_r, 360_deg)
        ,builder.add_stright(1.45_r)
        ,builder.add_annular_sector(0.5_r, -90_deg)
        ,builder.add_stright(1.075_r)
        ,builder.add_annular_sector(0.5_r, -90_deg)
        ,builder.add_stright(1.0_r + 2 * ROAD_WIDTH)
        ,builder.add_annular_sector(0.7_r, 270_deg)
        ,builder.add_stright(1.0_r + ROAD_WIDTH)
        ,builder.add_annular_sector(0.5_r, 90_deg)
        ,builder.add_stright(1.175_r)
        ,builder.add_annular_sector(0.5_r, 90_deg)
        ,builder.add_stright(3.0_r)
        ,builder.add_annular_sector(0.5_r, 90_deg)
        ,builder.add_stright(0.55_r)
        ,builder.add_annular_sector(0.5_r, 30_deg)
        ,builder.add_annular_sector(0.5_r, -60_deg)
        ,builder.add_annular_sector(0.5_r, 120_deg)
        ,builder.add_annular_sector(0.5_r, -180_deg)
        ,builder.add_annular_sector(0.5_r, 180_deg)
        ,builder.add_stright(1.43_r)
        ,builder.add_zebra_stright(0.12_r)
        ,Aurora{} | Placement{.pos = {1.0_r, -0.8_r}}
        ,Aurora{} | Placement{.pos = {-2.0_r, -2.8_r}}

    );
}

static constexpr auto scene1 = make_scene1();



Image<Grayscale> Scenes::render_scene1(const Pose2_t<real_t> viewpoint, const real_t scale){
    return scene1.render(viewpoint, scale);
} 

}