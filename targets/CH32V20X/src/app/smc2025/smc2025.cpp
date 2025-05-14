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

using namespace ymd;
using namespace ymd::hal;

#define UART hal::uart6

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
    
    auto & lcd_blk = portC[7];
    lcd_blk.outpp(HIGH);

    auto & lcd_dc = portD[7];
    auto & dev_rst = portB[7];

    const auto spi_fd = spi.attach_next_cs(portD[6]).value();

    drivers::ST7789 tft({spi, spi_fd, lcd_dc, dev_rst}, {240, 240});

        {
        tft.init();

        tft.set_flip_x(true);
        tft.set_flip_y(false);
        tft.set_swap_xy(true);
        tft.set_display_offset({320-240, 0}); 

        tft.set_format_rgb(true);
        tft.set_flush_dir_h(false);
        tft.set_flush_dir_v(false);
        tft.set_inversion(true);
    }



    I2cSw i2c{hal::portC[12], hal::portD[2]};
    i2c.init(100_KHz);
    
    drivers::MT9V034 camera{i2c};
    if(const auto res = camera.init();
        res.is_err()) PANIC(res.unwrap_err());

    
    Image<RGB565> rgb_img{{tft.rect().w, 4u}};
    Renderer renderer = {};

    [[maybe_unused]] auto plot_gray = [&](const Image<Grayscale> & src, const Rect2i & area){
        tft.put_texture(area.intersection(
                Rect2i(area.position, src.size())), 
                src.get_data());
    };

    while(true){
        
        renderer.bind(rgb_img);
        renderer.set_color(HSV888{0, int(100 + 100 * sinpu(time())), 255});
        renderer.draw_pixel(Vector2u(0, 0));
        renderer.draw_rect(Rect2i(20, 0, 20, 40));

        const auto gray_img = camera.clone();
        // plot_gray(gray_img, {0,0, 240,240});

        tft.put_texture(rgb_img.rect(), rgb_img.get_data());
        DEBUG_PRINTLN(millis(), gray_img.size(), uint8_t(gray_img.mean()));
        delay(20);
    }

    // timer4.init(24000);
    // timer5.init(24000);
    // timer8.init(234, 1);
    // timer8.oc(1).init();
    // timer8.oc(2).init();

    // delay(200);

    // delay(200);

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