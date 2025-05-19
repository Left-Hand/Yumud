#include "core/system.hpp"
#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"
#include "core/clock/time.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter.hpp"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"

#include "nvcv2/shape/shape.hpp"
#include "image/font/instance.hpp"

#include "src/testbench/tb.h"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"

#include "image/render/renderer.hpp"
#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"

#include "drivers/IMU/Magnetometer/HMC5883L/hmc5883l.hpp"
#include "drivers/IMU/Magnetometer/QMC5883L/qmc5883l.hpp"

#include "render.hpp"

using namespace ymd;
using namespace ymd::hal;

#define UART hal::uart6


struct HwPort{
};



using namespace ymd::smc::sim;

// DynamicScene make_scene(){
//     DynamicScene scene;
//     scene.add_element(AnnularSector{
//         .x = 0,
//         .y = 0,

//         .inner_radius = 0.6_r - 0.45_r/2,
//         .outer_radius = 0.6_r + 0.45_r/2,
        
//         .start_rad = -0.0_r,
//         .stop_rad = 2.5_r
//     });

//     scene.add_element(RectBlob{
//         .x = -0.2_r,
//         .y = -0.1_r,
//         .width = 0.45_r,
//         .height = 0.8_r
//     });
//     return scene;
// }



static constexpr auto ROAD_WIDTH = 0.45_r; 
class BlueprintFactory{
public:
    auto make_annular_sector(const real_t radius, const real_t rotation){
        const auto ret = AnnularSector{
            .inner_radius = radius - ROAD_WIDTH / 2,
            .outer_radius = radius + ROAD_WIDTH / 2,
            
            .start_rad = viewpoint_.rad,
            .stop_rad = viewpoint_.rad + rotation
        } | Placement{
            .pos = viewpoint_.pos,
        };

        return ret;
    }

    auto make_stright(const real_t length){
        const auto ret = RotatedRect{
            .width = ROAD_WIDTH,
            .height = length,
            .rotation = viewpoint_.rad
        } | Placement{
            .pos = viewpoint_.pos,
        };

        return ret;
    }
private:
    Gest2_t<real_t> viewpoint_;
};

constexpr auto make_scene2(){
    return make_static_scene(
        // AnnularSector{
        //     .inner_radius = 0.6_r - ROAD_WIDTH/2,
        //     .outer_radius = 0.6_r + ROAD_WIDTH/2,
            
        //     .start_rad = 0.0_r,
        //     .stop_rad = real_t(3 * PI / 2)
        //     // .stop_rad = real_t(PI)
        // } | Placement{
        //     .pos = {0.0_r, 0.0_r}
        // },

        // AnnularSector{
        //     .inner_radius = 0.6_r - ROAD_WIDTH/2,
        //     .outer_radius = 0.6_r + ROAD_WIDTH/2,
            
        //     .start_rad = 0.0_r,
        //     // .stop_rad = real_t(3 * PI / 2)
        //     .stop_rad = real_t(PI)
        // } | Placement{
        //     .pos = {-10.3_r, 0.0_r}
        // },

        // AnnularSector{
        //     .inner_radius = 0.6_r - ROAD_WIDTH/2,
        //     .outer_radius = 0.6_r + ROAD_WIDTH/2,
            
        //     .start_rad = 0.0_r,
        //     // .stop_rad = real_t(3 * PI / 2)
        //     .stop_rad = real_t(PI)
        // } | Placement{
        //     .pos = {-20.3_r, 0.0_r}
        // },

        // AnnularSector{
        //     .inner_radius = 0.6_r - ROAD_WIDTH/2,
        //     .outer_radius = 0.6_r + ROAD_WIDTH/2,
            
        //     .start_rad = 0.0_r,
        //     // .stop_rad = real_t(3 * PI / 2)
        //     .stop_rad = real_t(PI)
        // } | Placement{
        //     .pos = {5.3_r, 0.0_r}
        // },
        
        // RectBlob{
        //     .width = ROAD_WIDTH,
        //     .height = 0.8_r
        // } | Placement{
        //     .pos = {0.6_r, -0.4_r}
        // },

        // RectBlob{
        //     .width = ROAD_WIDTH,
        //     .height = 0.8_r
        // } | Placement{
        //     .pos = {-0.6_r, -0.4_r}
        // },

        // RectBlob{
        //     .width = ROAD_WIDTH,
        //     .height = 0.8_r
        // } | Placement{
        //     .pos = {-0.6_r, -0.4_r}
        // },

        // RectBlob{
        //     .width = 1.8_r,
        //     .height = ROAD_WIDTH
        // } | Placement{
        //     .pos = {0.4_r, - 0.7_r}
        // }

        // RectBlob{
        //     .width = 1.8_r,
        //     .height = ROAD_WIDTH
        // } | Placement{
        //     .pos = {0, 0}
        // }

        RotatedRect{
            .width = ROAD_WIDTH,
            .height = 1.1_r,
            .rotation = 0.3_r
        } | Placement{
            .pos = {0.0_r, -0.0_r}
        }

    );
}
void smc2025_main(){

    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(true);


    // bkp.init();
    
    // {
    //     portD[4].outpp(1);
    // }

    // printRecordedRunStatus();
    auto & spi = spi2;

    spi2.init(144_MHz);
    
    auto & lcd_blk = portD[0];
    lcd_blk.outpp(HIGH);

    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];

    const auto spi_fd = spi.attach_next_cs(portD[4]).value();

    drivers::ST7789 tft({spi, spi_fd, lcd_dc, dev_rst}, {240, 240});

    drivers::init_lcd(tft, drivers::ST7789_Presets::_320X170);

    I2cSw cam_i2c{hal::portD[2], hal::portC[12]};
    cam_i2c.init(100_KHz);

    I2cSw i2c{hal::portB[3], hal::portB[5]};
    i2c.init(800_KHz);
    
    drivers::MT9V034 camera{cam_i2c};

    camera.init().examine();

    drivers::QMC5883L qmc{i2c};
    qmc.init().examine();
    
    Image<RGB565> rgb_img{{tft.rect().w, 4u}};
    Renderer renderer = {};


    camera.set_exposure_value(1200).unwrap();
    camera.set_gain(2.4_r).unwrap();

    [[maybe_unused]] auto plot_gray = [&](
        const Image<Grayscale> & src, 
        const Rect2u & area
    ){
        tft.put_texture(area.intersection(
                Rect2u(area.position, src.size())), 
                src.get_data());
    };

    // const auto scene = make_scene();
    constexpr auto scene = make_scene2();

    while(true){
        // qmc.update().examine();
        renderer.bind(rgb_img);
        renderer.set_color(HSV888{0, int(100 + 100 * sinpu(clock::time())), 255});
        renderer.draw_pixel(Vector2u(0, 0));
        renderer.draw_rect(Rect2u(0, 0, 20, 40));

        // const auto gray_img = camera.frame().clone();
        // const auto viewpoint = Gest2_t<real_t>{
        //     {sinpu(clock::time() / 3) * 2.8_r + 2.3_r, sinpu(clock::time() / 2) * 0.3_r}, 
        //     real_t(PI/2) + 0.09_r * sinpu(clock::time())};
        const auto viewpoint = Gest2_t<real_t>{
            Vector2_t<real_t>(0, 0), real_t(PI/2)};

        const auto mbegin = clock::micros();
        const auto gray_img = scene.render(viewpoint);
        const auto render_use = clock::micros() - mbegin;
        plot_gray(gray_img, {0,6, 240,240});
        DEBUG_PRINTLN(render_use.count(), sizeof(scene));
        // DEBUG_PRINTLN(rgb_img.at(0, 0));
        tft.put_texture(rgb_img.rect(), rgb_img.get_data());
        // DEBUG_PRINTLN(millis(), gray_img.size(), uint8_t(gray_img.mean()));
        // DEBUG_PRINTLN(clock::millis(), qmc.read_mag().unwrap());
        // clock::delay(20ms);



    }

    // timer4.init(24000);
    // timer5.init(24000);
    // timer8.init(234, 1);
    // timer8.oc(1).init();
    // timer8.oc(2).init();

    // clock::delay(200ms);

    // clock::delay(200ms);

    // i2c.init(400000);
    // sccb.init(10000);

    // measurer.init();

    // start_key.init();
    // stop_key.init();

    // tft.init();
    // tft.setFlipX(true);
    // tft.setFlipY(false);
    // tft.setSwapXY(true);
    // tft.setFormatRGB(true);
    // tft.setFlushDirH(false);
    // tft.setFlushDirV(false);
    // tft.setInversion(true);

    // painter.bindImage(tft);
    // uint8_t camera_init_retry_times = 0;
    // constexpr uint8_t camera_retry_timeout = 7;
    // sccb.init(10000);
    // while(!camera.init()){
    //     camera_init_retry_times++;
    //     if(camera_init_retry_times >= camera_retry_timeout){
    //         DEBUG_LOG("camera init failed");
    //         while(true);
    //     }
    // }

    // cali();

    // GenericTimer & timer = timer2;
    // timer.init(ctrl_freq);
    // timer.bindCb(TimerIT::Update, [this](){this->ctrl();});
    // timer.enableIt(TimerIT::Update, NvicPriority{0,0});

}

