#include "src/testbench/tb.h"

#include "core/system.hpp"
#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"
#include "core/clock/time.hpp"
#include "core/stream/fixed_string_stream.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "algebra/vectors/quat.hpp"
#include "primitive/image/image.hpp"
#include "primitive/image/font/font.hpp"
#include "primitive/image/painter/painter.hpp"

#include "middlewares/nvcv2/shape/shape.hpp"
#include "middlewares/nvcv2/pixels/pixels.hpp"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"
#include "drivers/IMU/Axis6/MPU6050/mpu6050.hpp"
#include "drivers/IMU/Magnetometer/QMC5883L/qmc5883l.hpp"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

#include "render.hpp"
#include "scenes.hpp"

using namespace ymd;
using namespace ymd::drivers;


#define UART hal::uart6



static constexpr size_t MAX_COAST_ITEMS = 64;
using Pile = Range2<uint8_t>;
using Piles = std::map<uint8_t, Pile>;

using namespace ymd::smc::sim;


class Plotter{
    using Error = PainterBase::Error;

    template<typename T = void>
    using IResult = Result<T, Error>;

    Plotter(drivers::ST7789 & tft):
        tft_(tft)
    {
        // painter_.bind_image(tft);
    }

    IResult<> plot_rgb(const Image<RGB565> image, const Rect2u & area){
        tft_.put_texture(area, image.head_ptr()).examine();

        return Ok();
    };

    // IResult<> plot_coast(const Pixels & coast){
    //     if(coast.size() < 2){
    //         return Err(Error::PointsTooLess);
    //     }

    //     for(auto it = coast.begin(); 
    //         it != std::prev(coast.end()); 
    //         it = std::next(it)
    //     ){
    //         const auto & p_curr = *it;
    //         const auto & p_next = *std::next(it);
    //         if(const auto res = painter_.draw_line(p_curr, p_next);
    //             res.is_err()) return res;
    //     }

    //     return Ok();
    // };

    // IResult<> plot_segment(const PixelSegment seg){
    //     return painter_.draw_line(seg.first, seg.second);
    // };

    template<typename T>
    IResult<> plot_dots(const std::span<const T> pts){
        for(const auto & pt : pts){
            if(const auto res = painter_.draw_filled_circle({pt.x, pt.y}, 2);
                res.is_err()) return res;
        }
    };


    // IResult<> plot_pixels(const Pixels & pts){
    //     for(const auto pixel : pts){
    //         painter_.putpixel_unchecked(pixel);
    //     }
    //     return Ok();
    // };

    IResult<> plot_dot(const Vec2u pos, const uint radius = 2){
        painter_.putpixel_unchecked(pos);

        return Ok();
    };


    IResult<> plot_vec3(const Vec3<iq16> & vec3,  const Vec2u pos){
        static constexpr auto WINDOW_LENGTH = 50u;
        static constexpr auto ARROW_RADIUS = 3u;
        static constexpr auto X_UNIT = Vec2<iq16>::RIGHT;
        static constexpr auto Y_UNIT = Vec2<iq16>::RIGHT.rotated(60_deg);
        static constexpr auto Z_UNIT = Vec2<iq16>::DOWN;
        
        static constexpr RGB565 X_COLOR = color_cast<RGB565>(ColorEnum::RED);
        static constexpr RGB565 Y_COLOR = color_cast<RGB565>(ColorEnum::GREEN);
        static constexpr RGB565 Z_COLOR = color_cast<RGB565>(ColorEnum::BLUE);
        
        const auto arm_length = vec3.length();
        const auto x_axis = Vec3<iq16>::from_x00(arm_length);
        const auto y_axis = Vec3<iq16>::from_0y0(arm_length);
        const auto z_axis = Vec3<iq16>::from_00z(arm_length);

        const auto rot = Quat<iq16>::from_direction(vec3);
        const Vec2u center_point = pos + Vec2u(WINDOW_LENGTH, WINDOW_LENGTH) / 2;

        auto plot_vec3_to_plane = [&](
            const Vec3<iq16> & axis, const char chr, const RGB565 color)
        -> IResult<>{
            const Vec3<iq16> end = rot.xform(axis);
            const Vec2u end_point = center_point + (X_UNIT * end.x + Y_UNIT * end.y + Z_UNIT * end.z);
            painter_.set_color(color);
            if(const auto res = painter_.draw_line(center_point, end_point);
                res.is_err()) return res;
            if(const auto res = painter_.draw_filled_circle(end_point, ARROW_RADIUS);
                res.is_err()) return res;
        };

        const auto guard = painter_.create_color_guard();
        if(const auto res = painter_.draw_filled_rect(Rect2u{pos, Vec2u{WINDOW_LENGTH, WINDOW_LENGTH}});
            res.is_err()) return res;
        if(const auto res = plot_vec3_to_plane(x_axis, 'X', X_COLOR);
            res.is_err()) return res;
        if(const auto res = plot_vec3_to_plane(y_axis, 'Y', Y_COLOR);
            res.is_err()) return res;
        if(const auto res = plot_vec3_to_plane(z_axis, 'Z', Z_COLOR);
            res.is_err()) return res;
        return Ok();
    };
private:
    drivers::ST7789 & tft_;
    Painter<RGB565> painter_;
};

static auto init_mpu6050(MPU6050 & mpu) -> Result<void, MPU6050::Error> {
    mpu.set_package(MPU6050::Package::MPU6050);
    if(const auto res = mpu.init({});
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = mpu.set_acc_fs(MPU6050::AccFs::_2G);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = mpu.enable_direct_mode(EN);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

void smc2025_main(){

    UART.init({
        .remap = hal::UART6_REMAP_PC0_PC1, 
        .baudrate = hal::NearestFreq(576000),
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    auto & spi = hal::spi2;

    hal::spi2.init({
        .remap = hal::SPI2_REMAP_PB13_PB14_PB15_PB12,
        .baudrate = hal::NearestFreq(144_MHz)
    });
    
    auto lcd_blk = hal::PD<0>();
    lcd_blk.outpp(HIGH);

    auto lcd_dc = hal::PD<7>();
    auto dev_rst = hal::PB<7>();
    auto spi_cs_gpio = hal::PD<4>();
    const auto spi_rank = spi.allocate_cs_pin(&spi_cs_gpio).unwrap();

    drivers::ST7789 tft{
        drivers::ST7789_Transport{&spi, spi_rank, &lcd_dc, &dev_rst}, 
        {240, 240}
    };

    tft.init(drivers::st7789_preset::_320X170{}).examine();

    auto cam_i2c_scl = hal::PD<2>();
    auto cam_i2c_sda = hal::PC<12>();
    hal::I2cSw cam_i2c{&cam_i2c_scl, &cam_i2c_sda};
    cam_i2c.init({100_KHz});

    auto i2c_scl = hal::PB<3>();
    auto i2c_sda = hal::PB<5>();
    hal::I2cSw i2c{&i2c_scl, &i2c_sda};
    i2c.init({400_KHz});
    
    #if 0
    // drivers::MT9V034 camera{&cam_i2c};

    // camera.init().examine();
    // camera.set_exposure_value(1200).examine();
    // camera.set_gain(2.4_r).examine();
    #endif

    drivers::QMC5883L qmc{&i2c};
    retry(2, [&]{return qmc.init();}).examine();

    MPU6050 mpu{&i2c};

    init_mpu6050(mpu).examine();

    auto yaw_angle = Angular<iq16>::ZERO; 

    [[maybe_unused]] auto plot_gray = [&](
        const Image<Gray> & src, 
        const Rect2u & area
    ){
        tft.put_texture(
            ({
                const auto ins_opt = area.intersection(
                    Rect2u(area.top_left, src.size()));
                if(ins_opt.is_none()) return;
                ins_opt.unwrap();
            }), 
            src.head_ptr()
        ).examine();
    };

    [[maybe_unused]] auto plot_bina = [&](
        const Image<Gray> & src, 
        const Rect2u & area
    ){
        tft.put_texture(
            ({
                const auto ins_opt = area.intersection(
                    Rect2u(area.top_left, src.size()));
                if(ins_opt.is_none()) return;
                ins_opt.unwrap();
            }), 
            src.head_ptr()
        ).examine();
    };

    [[maybe_unused]] auto test_render = [&]{
    
        [[maybe_unused]]const auto now_secs = clock::time();
        const auto pose = Isometry2<iq16>{
            // .rotation = UnitComplex<iq16>::from_radians(now_secs + iq16(1 / TAU) * math::sinpu(now_secs)),
            // .translation = Vec2<iq16>(0, -1.5_r) + Vec2<iq16>(-1.9_r, 0).rotated(Angular<iq16>::from_radians(now_secs)), 
            .rotation = UnitComplex<iq16>::from_angle(yaw_angle),
            .translation = Vec2<iq16>(0, -1.5_r), 
        };
            // {1.0_r, -0.5_r}, 0.0_r};
            // {-1.0_r, -1.81_r}, 1.57_r};
            // {0, 0}, 1.57_r};

        const auto mbegin = clock::micros();
        // const auto gray_img = Scenes::render_scene2(pose, 0.02_r);
        // const auto gray_img = Scenes::render_scene2({pose, 0.07_r});
        const auto gray_img = Scenes::render_scene1({
            .pose = pose, 
            .zoom = 0.02_r
        });
        // const auto gray_img = Scenes::render_scene1(pose, 0.02_r);
        const auto render_uticks = clock::micros() - mbegin;
        plot_gray(gray_img, {Vec2u{0,6}, Vec2u{240,240}});

        // DEBUG_PRINTLN(render_uticks.count(), gray_img.size(), uint8_t(gray_img.mean()));
        // DEBUG_PRINTLN(render_uticks.count(), gray_img.size(), gray_img.size().to_rect().x_range());
        const auto rect = Rect2u::from_size(gray_img.size());
        const auto range = Range2<uint32_t>::from_start_and_length(rect.top_left.x, rect.size.x);

        DEBUG_PRINTLN(
            render_uticks.count(), 
            gray_img.size(), 
            rect.top_left.x, 
            rect.size.x, 
            range 
        );
    
        // DEBUG_PRINTLN(clock::millis(), qmc.read_mag().unwrap());
        // clock::delay(20ms);
        // DEBUG_PRINTLN(render_uticks.count(), pose);
    };


    [[maybe_unused]] auto test_fill = [&]{
        tft.fill(color_cast<RGB565>(ColorEnum::BRRED)).examine();
    };

    Image<RGB565> rgb_img{{tft.size().x, 20u}};
    Painter<RGB565> painter;
    Font7x7 enfont;
    painter.set_src_image(&rgb_img);
    [[maybe_unused]] auto test_paint = [&]{
        painter.set_color(ColorEnum::WHITE);
        painter.set_en_font(&enfont).examine();
        painter.fill(color_cast<RGB888>(ColorEnum::BLACK)).examine();

        FixedStringStream<64> ss;
        ss.println("helloword", clock::time());
        painter.draw_ascii_str({0,0}, ss.str()).examine();
        DEBUG_PRINTLN(ss.str());
        // painter.draw_hollow_rect({0,0,7,7}).examine();
        tft.put_texture(
            Rect2u16::from_size(rgb_img.size()),
            rgb_img.head_ptr()
        ).examine();

    };

    auto & timer = hal::timer1;
    timer.init({
        .remap = hal::TIM10_REMAP_B3_B4_B5_C14__A5_A6_A7,
        .count_freq = hal::NearestFreq(25),
        .count_mode = hal::TimerCountMode::Up,
    })        .unwrap()
        .alter_to_pins({
            hal::TimerChannelSelection::CH1,
            hal::TimerChannelSelection::CH2,
            hal::TimerChannelSelection::CH3,
        })
        .unwrap();

    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_handler([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            mpu.update().examine();
            qmc.update().examine();
            const auto gyr = mpu.read_gyr().examine();
            // const auto dir = qmc.read_mag().examine();
            yaw_angle = (yaw_angle + Angular<iq16>::from_radians(gyr.z) * 0.04_iq16).unsigned_normalized();
            break;
        }
        default: break;
        }
    });

    timer.start();
    while(true){
        // test_fill();
        test_render();


        // yaw_angle = Angular<iq16>::from_radians(atan2pu(dir.x, dir.y));
        // DEBUG_PRINTLN_IDLE(gyr.z);
        DEBUG_PRINTLN_IDLE(yaw_angle.to_degrees(), mpu.read_acc().examine());
        //     mpu.read_acc().examine(),
            
        // );
        // test_paint();
        // test_paint();
        // qmc.update().examine();
        // painter.set_color(HSV888{0, int(100 + 100 * math::sinpu(clock::time())), 255});
        // painter.draw_pixel(Vec2u(0, 0));
        // painter.draw_filled_rect(Rect2u(0, 0, 20, 40)).examine();

    }

}