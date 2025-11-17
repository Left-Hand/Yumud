#include "scenes.hpp"
#include "primitive/angle.hpp"

namespace ymd::smc::sim{

static constexpr auto make_scene1(){
    constexpr auto ROAD_WIDTH = 0.45_r;
    BlueprintSpawner spawner = {
        {.road_width = ROAD_WIDTH}
        ,Isometry2<real_t>{ UnitComplex<real_t>::from_angle(180_deg), Vec2<real_t>{0, 0}}
    };

    return make_static_scene(
        spawner.spawn_stright(4.00_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(3.55_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(0.75_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(1.45_r)
        ,spawner.spawn_annular_sector(0.5_r, 360_deg)
        ,spawner.spawn_stright(1.45_r)
        ,spawner.spawn_annular_sector(0.5_r, -90_deg)
        ,spawner.spawn_stright(1.075_r)
        ,spawner.spawn_annular_sector(0.5_r, -90_deg)
        ,spawner.spawn_stright(1.0_r + 2 * ROAD_WIDTH)
        ,spawner.spawn_annular_sector(0.7_r, 270_deg)
        ,spawner.spawn_stright(1.0_r + ROAD_WIDTH)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(1.175_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(3.0_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(0.55_r)
        ,spawner.spawn_annular_sector(0.5_r, 30_deg)
        ,spawner.spawn_annular_sector(0.5_r, -60_deg)
        ,spawner.spawn_annular_sector(0.5_r, 120_deg)
        ,spawner.spawn_annular_sector(0.5_r, -180_deg)
        ,spawner.spawn_annular_sector(0.5_r, 180_deg)
        ,spawner.spawn_stright(1.43_r)
        ,spawner.spawn_zebra_stright(0.12_r)
        ,SpotLight{} | Placement{.translation = {1.0_r, -0.8_r}}
        ,SpotLight{} | Placement{.translation = {-2.0_r, -2.8_r}}

    );
}

static constexpr auto make_scene2(){
    constexpr auto ROAD_WIDTH = 0.05_r;
    BlueprintSpawner spawner = {
        {.road_width = ROAD_WIDTH}
        ,Isometry2<real_t>{UnitComplex<real_t>::from_angle(180_deg), Vec2<real_t>{0, 0}}
    };
    
    return make_static_scene(
        spawner.spawn_stright(4.00_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(3.55_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(0.75_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(1.45_r)
        ,spawner.spawn_annular_sector(0.5_r, 360_deg)
        ,spawner.spawn_stright(1.45_r)
        ,spawner.spawn_annular_sector(0.5_r, -90_deg)
        ,spawner.spawn_stright(1.075_r)
        ,spawner.spawn_annular_sector(0.5_r, -90_deg)
        ,spawner.spawn_stright(1.0_r + 2 * ROAD_WIDTH)
        ,spawner.spawn_annular_sector(0.7_r, 270_deg)
        ,spawner.spawn_stright(1.0_r + ROAD_WIDTH)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(1.175_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(3.0_r)
        ,spawner.spawn_annular_sector(0.5_r, 90_deg)
        ,spawner.spawn_stright(0.55_r)
        ,spawner.spawn_annular_sector(0.5_r, 30_deg)
        ,spawner.spawn_annular_sector(0.5_r, -60_deg)
        ,spawner.spawn_annular_sector(0.5_r, 120_deg)
        ,spawner.spawn_annular_sector(0.5_r, -180_deg)
        ,spawner.spawn_annular_sector(0.5_r, 180_deg)
        ,spawner.spawn_stright(1.43_r)
        ,spawner.spawn_zebra_stright(0.12_r)
        ,SpotLight{} | Placement{.translation = {1.0_r, -0.8_r}}
        ,SpotLight{} | Placement{.translation = {-2.0_r, -2.8_r}}

    );
}





Image<Gray> Scenes::render_scene1(const CamView2<real_t> & viewport){
    static constexpr auto scene = make_scene1();
    return scene.render(viewport.pose, viewport.zoom);
} 
Image<Gray> Scenes::render_scene2(const CamView2<real_t> & viewport){
    static constexpr auto scene = make_scene2();
    return scene.render(viewport.pose, viewport.zoom);
}



}