#include "scenes.hpp"
#include "primitive/arithmetic/angular.hpp"

namespace ymd::smc::sim{

static constexpr auto make_scene1(){
    constexpr auto ROAD_WIDTH = 0.45_iq16;
    BlueprintSpawner spawner = {
        {.road_width = ROAD_WIDTH}
        ,math::Isometry2<iq16>{ math::UnitComplex<iq16>::from_angle(180_deg), math::Vec2<iq16>{0, 0}}
    };

    return make_static_scene(
        spawner.spawn_stright(4.00_iq16)
        ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(3.55_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(0.75_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(1.45_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 360_deg)
        // ,spawner.spawn_stright(1.45_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, -90_deg)
        // ,spawner.spawn_stright(1.075_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, -90_deg)
        // ,spawner.spawn_stright(1.0_iq16 + 2 * ROAD_WIDTH)
        // ,spawner.spawn_annular_sector(0.7_iq16, 270_deg)
        // ,spawner.spawn_stright(1.0_iq16 + ROAD_WIDTH)
        // ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(1.175_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(3.0_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(0.55_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 30_deg)
        // ,spawner.spawn_annular_sector(0.5_iq16, -60_deg)
        // ,spawner.spawn_annular_sector(0.5_iq16, 120_deg)
        // ,spawner.spawn_annular_sector(0.5_iq16, -180_deg)
        // ,spawner.spawn_annular_sector(0.5_iq16, 180_deg)
        // ,spawner.spawn_stright(1.43_iq16)
        // ,spawner.spawn_zebra_stright(0.12_iq16)
        // ,SpotLight{} | Placement{.translation = {1.0_iq16, -0.8_iq16}}
        // ,SpotLight{} | Placement{.translation = {-2.0_iq16, -2.8_iq16}}

    );
}

static constexpr auto make_scene2(){
    constexpr auto ROAD_WIDTH = 0.05_iq16;
    BlueprintSpawner spawner = {
        {.road_width = ROAD_WIDTH}
        ,math::Isometry2<iq16>{math::UnitComplex<iq16>::from_angle(180_deg), math::Vec2<iq16>{0, 0}}
    };
    
    return make_static_scene(
        spawner.spawn_stright(4.00_iq16)
        ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        ,spawner.spawn_stright(3.55_iq16)
        ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(0.75_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(1.45_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 360_deg)
        // ,spawner.spawn_stright(1.45_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, -90_deg)
        // ,spawner.spawn_stright(1.075_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, -90_deg)
        // ,spawner.spawn_stright(1.0_iq16 + 2 * ROAD_WIDTH)
        // ,spawner.spawn_annular_sector(0.7_iq16, 270_deg)
        // ,spawner.spawn_stright(1.0_iq16 + ROAD_WIDTH)
        // ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(1.175_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(3.0_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 90_deg)
        // ,spawner.spawn_stright(0.55_iq16)
        // ,spawner.spawn_annular_sector(0.5_iq16, 30_deg)
        // ,spawner.spawn_annular_sector(0.5_iq16, -60_deg)
        // ,spawner.spawn_annular_sector(0.5_iq16, 120_deg)
        // ,spawner.spawn_annular_sector(0.5_iq16, -180_deg)
        // ,spawner.spawn_annular_sector(0.5_iq16, 180_deg)
        // ,spawner.spawn_stright(1.43_iq16)
        // ,spawner.spawn_zebra_stright(0.12_iq16)
        // ,SpotLight{} | Placement{.translation = {1.0_iq16, -0.8_iq16}}
        // ,SpotLight{} | Placement{.translation = {-2.0_iq16, -2.8_iq16}}

    );
}





Image<Gray> Scenes::render_scene1(const math::CamView2<iq16> & viewport){
    static constexpr auto scene = make_scene1();
    return scene.render(viewport.pose, viewport.zoom);
} 
Image<Gray> Scenes::render_scene2(const math::CamView2<iq16> & viewport){
    static constexpr auto scene = make_scene2();
    return scene.render(viewport.pose, viewport.zoom);
}



}