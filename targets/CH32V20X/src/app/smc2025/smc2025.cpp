#include "src/testbench/tb.h"

#include "core/system.hpp"
#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"
#include "core/clock/time.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/i2c/i2csw.hpp"

#include "types/quat/Quat.hpp"
#include "types/image/image.hpp"
#include "types/image/font/font.hpp"
#include "types/image/painter.hpp"
#include "types/image/font/instance.hpp"

#include "nvcv2/shape/shape.hpp"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "drivers/Camera/MT9V034/mt9v034.hpp"
#include "drivers/IMU/Magnetometer/QMC5883L/qmc5883l.hpp"
#include "drivers/Display/Polychrome/ST7789/st7789.hpp"

#include "render.hpp"
#include "scenes.hpp"

using namespace ymd;
using namespace ymd::hal;

#define UART hal::uart6



struct HwPort{
};



using namespace ymd::smc::sim;

enum class AlignMode:uint8_t{
    LEFT,
    RIGHT,
    BOTH,
    UPPER
};


using Boundry = std::map<size_t, size_t>;
using Pile = std::pair<size_t, Range2i>;
using Piles = std::map<size_t, Range2i>;
using Point = Vector2i; 
using ymd::nvcv2::Shape::Seed;
using Segment = std::pair<const Point & ,const Point &>;


static constexpr size_t max_item_size = 64;
static constexpr size_t max_ranges_size = 16;

using CoastItem = Vector2_t<uint8_t>;
using Points = sstl::vector<Vector2_t<int16_t>, max_item_size>;
using Coast = sstl::vector<CoastItem, max_item_size>;
using Coasts = sstl::vector<Coast, 4>;
using Ranges = sstl::vector<Range2_t<int16_t>, max_ranges_size>;


class Plotter{
public:
    using Error = PainterBase::Error;

    template<typename T = void>
    using IResult = Result<T, Error>;

    Plotter(drivers::ST7789 & tft):
        tft_(tft)
    {
        painter_.bind_image(tft);
    }

    IResult<> plot_rgb(const Image<RGB565> image, const Rect2u & area){
        tft_.put_texture(area, image.get_data());

        return Ok();
    };

    IResult<> plot_coast(const Coast & coast,  const Vector2u pos){
        if(coast.size() <= 1){
            return Err(Error::PointsTooLess);
        }

        auto it = coast.begin();

        while(it != std::prev(coast.end())){
            auto & p_curr = *it;
            auto & p_next = *std::next(it);
            if(const auto res = painter_.draw_line(p_curr, p_next);
                res.is_err()) return res;
            it = std::next(it);
        }

        return Ok();
    };

    IResult<> plot_coast_colored (
        const Coast & coast, const Vector2u pos)
    {

        if(coast.size() <= 2){
            return Err(Error::PointsTooLess);
        }

        auto it = coast.begin();

        while(it != std::prev(coast.end())){
            auto & p_curr = *it;
            auto & p_next = *std::next(it);
            if(const auto res = painter_.draw_line(p_curr, p_next);
                res.is_err()) return res;
            it = std::next(it);
        }

        return Ok();
    };

    IResult<> plot_segment(const Segment seg, const Vector2u pos){
        return painter_.draw_line(seg.first + pos, seg.second + pos);
    };

    IResult<> plot_points(const std::span<const Point> pts,  const Vector2u pos){
        for(const auto & pt : pts){
            if(const auto res = painter_.draw_filled_circle(pt + pos, 2);
                res.is_err()) return res;
        }

    };


    IResult<> plot_coast_points(const Coast & pts,  const Vector2u pos){
        for(const auto & pt : pts){
            if(const auto res = painter_.draw_filled_circle(pt + pos, 2);
                res.is_err()) return res;
        }
    };

    IResult<> plot_bound(const Boundry & bound,  const Vector2u pos){
        for(auto && pt : bound){
            painter_.draw_pixel(Vector2u{pt.second, pt.first}+ pos);
        }

        return Ok();
    };

    IResult<> plot_point(const Vector2u pos, const uint radius = 2){
        painter_.draw_pixel(pos);

        return Ok();
    };

    // [[maybe_unused]] auto plot_pile = [&](const Pile & bound, const RGB565 & color = RGB565::RED){
    //     painter_.draw_line(Vector2u{bound.second.from, bound.first}, {bound.second.to, bound.first});
    // }; 


    IResult<> plot_vec3(const Vector3_t<real_t> & vec3,  const Vector2u pos){
        static constexpr auto square_length = 50;
        const auto arm_length = vec3.length();

        static constexpr auto radius = 3;
        static constexpr auto x_unit = Vector2_t(1.0_r, 0.0_r);
        static constexpr auto y_unit = Vector2_t(0.5_r, -0.73_r);
        static constexpr auto z_unit = Vector2_t(0.0_r, -1_r);

        const auto x_axis = Vector3_t<real_t>::from_x(arm_length);
        const auto y_axis = Vector3_t<real_t>::from_y(arm_length);
        const auto z_axis = Vector3_t<real_t>::from_z(arm_length);

        static constexpr RGB565 x_color = RGB565(ColorEnum::RED);
        static constexpr RGB565 y_color = RGB565(ColorEnum::GREEN);
        static constexpr RGB565 z_color = RGB565(ColorEnum::BLUE);
        // static constexpr RGB565 bg_color = RGB565(ColorEnum::BLACK);

        auto vec3n = vec3.normalized();
        const auto rot = Quat_t<real_t>::from_shortest_arc(Vector3_t<real_t>(0, 0, -1), vec3n);
        const Vector2u center_point = pos + Vector2u(square_length, square_length) / 2;

        auto plot_vec3_to_plane = [&](
            const Vector3_t<real_t> & axis, const char chr, const RGB565 color)
        -> IResult<>{
            Vector3_t<real_t> end = rot.xform(axis);
            Vector2u end_point = center_point + (x_unit * end.x + y_unit * end.y + z_unit * end.z);
            painter_.set_color(color);
            if(const auto res = painter_.draw_line(center_point, end_point);
                res.is_err()) return res;
            if(const auto res = painter_.draw_filled_circle(end_point, radius);
                res.is_err()) return res;
        };

        if(const auto res = painter_.draw_filled_rect(Rect2u{pos, Vector2u{square_length, square_length}});
            res.is_err()) return res;
        if(const auto res = plot_vec3_to_plane(x_axis, 'X', x_color);
            res.is_err()) return res;
        if(const auto res = plot_vec3_to_plane(y_axis, 'Y', y_color);
            res.is_err()) return res;
        if(const auto res = plot_vec3_to_plane(z_axis, 'Z', z_color);
            res.is_err()) return res;
        return Ok();
    };
private:
    drivers::ST7789 & tft_;
    Painter<RGB565> painter_;
};

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
    // Painter<RGB565> painter = {};


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

    [[maybe_unused]] auto plot_bina = [&](
        const Image<Grayscale> & src, 
        const Rect2u & area
    ){
        tft.put_texture(area.intersection(
                Rect2u(area.position, src.size())), 
                src.get_data());
    };




    while(true){
        // qmc.update().examine();
        // painter.bind_image(rgb_img);
        // painter.set_color(HSV888{0, int(100 + 100 * sinpu(clock::time())), 255});
        // painter.draw_pixel(Vector2u(0, 0));
        // painter.draw_filled_rect(Rect2u(0, 0, 20, 40)).examine();

        // const auto gray_img = camera.frame().clone();
        // const auto viewpoint = Gest2_t<real_t>{
        //     {sinpu(clock::time() / 3) * 2.8_r + 2.3_r, sinpu(clock::time() / 2) * 0.3_r}, 
        //     real_t(PI/2) + 0.09_r * sinpu(clock::time())};
        const auto t = clock::time();
        const auto viewpoint = Gest2_t<real_t>{
            Vector2_t<real_t>(0, -1.5_r) + Vector2_t<real_t>(-1.5_r, 0)
            .rotated(t), t + 0.2_r * sinpu(t)};
            // Vector2_t<real_t>(-0.1_r, 0), real_t(PI)};

        const auto mbegin = clock::micros();
        const auto gray_img = Scenes::render_scene1(viewpoint, 0.02_r);
        const auto render_use = clock::micros() - mbegin;
        plot_gray(gray_img, {0,6, 240,240});

        // DEBUG_PRINTLN(rgb_img.at(0, 0));
        tft.put_texture(rgb_img.rect(), rgb_img.get_data());
        // DEBUG_PRINTLN(millis(), gray_img.size(), uint8_t(gray_img.mean()));
        // DEBUG_PRINTLN(clock::millis(), qmc.read_mag().unwrap());
        // clock::delay(20ms);
        
        
        
        DEBUG_PRINTLN(render_use.count());
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

