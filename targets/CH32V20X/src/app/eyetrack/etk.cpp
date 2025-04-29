#include "etk.hpp"

#include "core/system.hpp"
#include "core/debug/debug.hpp"

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


#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"


#include "eyetrack.hpp"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "eyeparser.hpp"


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;
using namespace ymd::nvcv2;
using namespace etk;


#define WHARE_OK while(true){DEBUGGER.println(millis());};



// void etk_main(){

//     auto & logger = LOGGER_INST;

//     #ifdef CH32V30X
//     auto & spi = spi2;
//     #else
//     auto & spi = spi1;
//     #endif

//     logger.init(115200);
//     auto & lcd_blk = portC[7];
    
//     lcd_blk.outpp(HIGH);

//     auto & lcd_cs = portD[6];
//     auto & lcd_dc = portD[7];
//     auto & dev_rst = portB[7];

//     spi.bind_cs_pin(lcd_cs, 0);
//     spi.init(144_MHz, CommStrategy::Blocking);

//     ST7789 tftDisplayer({{spi, SpiSlaveIndex(0)}, lcd_dc, dev_rst}, {240, 135});

//     {//init tft
//         tftDisplayer.init();
//         // tftDisplayer.setDisplayOffset({51, 40}); 

//         tftDisplayer.set_flip_x(false);
//         tftDisplayer.set_flip_y(true);
//         if(true){
//             tftDisplayer.set_swap_xy(true);
//             tftDisplayer.set_display_offset({40, 52}); 
//         }else{
//             tftDisplayer.set_swap_xy(false);
//             tftDisplayer.set_display_offset({52, 40}); 
//         }
//         tftDisplayer.set_format_rgb(true);
//         tftDisplayer.set_flush_dir_h(false);
//         tftDisplayer.set_flush_dir_v(false);
//         tftDisplayer.set_inversion(true);
//     }

//     Painter<RGB565> painter = Painter<RGB565>();
//     painter.bindImage(tftDisplayer);

//     painter.setChFont(ymd::font7x7).unwrap();
//     painter.setEnFont(ymd::font8x5).unwrap();

    
//     I2cSw       i2c{portD[2], portC[12]};
//     i2c.init(125_KHz);


//     PCA9685 pca{i2c};
//     pca.init();
    
//     pca.set_frequency(50, real_t(1.09));


//     auto eye_conf =         Eyes::Config{
//             .l_center = Vector2i{60, 62},
//             .r_center = Vector2i{180, 62},

//             .eye_radius = 40,
//             .iris_radius = 13,
//             .pupil_radius = 7
//     };

//     Eyes eye{
//         eye_conf,
//         {
//             .yaw = pca[0],
//             .pitch = pca[1],

//             .upper_l = pca[2],
//             .lower_l = pca[3],

//             .upper_r = pca[4],
//             .lower_r = pca[5]
//         }
//     };


//     // auto x_pwm = pca[6];
//     // auto y_pwm = pca[7];
//     MG995 servo_x{pca[6]};
//     MG995 servo_y{pca[7]};

//     while(true){

//         #define EYE_TB
//         #ifdef EYE_TB
//         eye.render(painter);


//         delay(20);
//         painter.fill(ColorEnum::BLACK).unwrap();

//         painter.setColor(ColorEnum::RED).unwrap();
//         // painter.drawFilledRect({2,2, 12,12});

//         if(logger.available()){
//             EtkToken tk;
//             while(logger.available()){
//                 logger.read1(reinterpret_cast<char &>(tk));
//             }

//             eye.update(
//                 {Vector2_t<real_t>(Vector2i(tk).flipy())}, 
//                 std::to_array({EyelidInfo{{0,0}}, EyelidInfo{{0,0}}})
//             );

//             eye.move();

//             auto vec = Vector2_t<real_t>(eye.eyeInfo().pos) * real_t(0.2);
//             vec = Vector2_t<real_t>(real_t(PI/2), real_t(PI - 0.2)) + Vector2_t<real_t>(-vec.x, vec.y);
                                                                                                                                                                                                                                                                                    
//             servo_x.set_radian(+vec.x);
//             servo_y.set_radian(+vec.y);
//         }        
//         #endif
    
//     }
// }