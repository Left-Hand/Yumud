#include "tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/uart/hw_singleton.hpp"
#include "hal/bus/i2c/soft/soft_i2c.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/hw_singleton.hpp"

#include "drivers/Display/Monochrome/SSD1306/ssd1306.hpp"
#include "drivers/CommonIO/Key/Key.hpp"

#include "primitive/image/painter/painter.hpp"

#if 0

using namespace ymd;
using namespace ymd::drivers;

#define UART hal::usart2

class Menu{
protected:
    enum class Func:uint8_t{
        NONE,
        CALI,
        EZ,
        CF,
        SAVE,
        RST
    };

    Func func_ = Func::NONE;

    VerticalBinaryImage & frame_;
    OutputStream & os_;
    Painter<Binary> painter_;

    String send_str;
    iq16 para = 0;
public:
    Menu(VerticalBinaryImage & _frame, OutputStream & _os):frame_(_frame), os_(_os){
        painter_.bind(&frame_);
        painter_.set_color(Binary(true));
    }

    void render(){
        frame_.fill(0);
        painter_.draw_args({0,0}, clock::millis()).unwrap();

        String str;

        switch(func_){
            case Func::NONE:
                str = "none";
                break;
            case Func::CALI:
                str = "cali";
                break;
            case Func::EZ:
                str = "ez";
                break;
            case Func::CF:
                str = "c 0.6";
                break;
            case Func::SAVE:
                str = "save";
                break;
            case Func::RST:
                str = "reset";
                break;
        }

        // painter_.draw_args{0,8}, "func:" + str);
        painter_.draw_args({0,8}, "func:").unwrap();
        painter_.draw_args({0,16}, StringView(send_str)).unwrap();
    }

    void next(){
        func_ = (func_ == Func::RST) ? Func::NONE : Func(uint8_t(func_) + 1);
    }

    void invoke(){
        String str;

        switch(func_){
            case Func::NONE:
                str = "";
                break;
            case Func::CALI:
                str = "cali";
                break;
            case Func::EZ:
                // str = "ez " + toString(para);
                para += iq16(0.3);
                break;
            case Func::CF:
                str = "c 0.6";
                break;
            case Func::SAVE:
                str = "save";
                break;
            case Func::RST:
                str = "reset";
                break;
        }

        send_str = str;
        os_.println(send_str);
    }
};

static void oled_tb(){
    auto & SCL_PIN = hal::PB<0>();
    auto & SDA_PIN = hal::PB<1>();
    static constexpr auto I2C_BAUD = 2'000'000;
    // static constexpr auto MONITOR_HZ = 5000;

    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_splitter(' ');

    Key key_left{hal::PB<2>(), LOW};
    Key key_right{hal::PB<1>(), LOW};

    Font8x5 font;

    key_left.init();
    key_right.init();
    clock::delay(100ms);

    // SoftI2c i2c{hal::PB<13>(), hal::PB<15>()};
    hal::SoftI2c i2c{SCL_PIN, SDA_PIN};
    // i2c.init(0);
    i2c.init(I2C_BAUD);


    // auto oled = SSD13XX(oled_phy, SSD13XX_72X40_Config());
    constexpr auto cfg = SSD13XX_Presets::_128X64{{
        .flip_x = true,
        .flip_y = true
    }};

    auto oled = SSD13XX({i2c}, cfg);
    auto & frame = oled.fetch_frame();
    
    // DEBUG_PRINTLN("init started");

    // hal::timer1.init(MONITOR_HZ);
    // hal::timer1.attach(TimerIT::Update,{0,0},[&]{
    //     DEBUG_PRINTLN(bool(SCL_PIN.read()), ',', bool(SDA_PIN.read()));
    // });

    oled.init().unwrap();

    Menu menu {frame, DEBUGGER};

    Painter<Binary> painter;
    
    painter.set_en_font(&font).unwrap();
    painter.bind(oled.fetch_frame());
    painter.set_color(Binary(Binary::WHITE));

    while(true){
        painter.fill(Binary(Binary::BLACK)).unwrap();
        painter.set_color(Binary(Binary::WHITE));

        const math::Rect2u view = {0,0,128,48};

        painter.draw_hollow_rect(view).unwrap();
        painter.draw_fx(
            view.shrink(6).unwrap(), 
            [&](const iq16 x){
                return math::sinpu(4 * x + clock::seconds()) * 0.5_r + 0.5_r;
            }
        ).unwrap();

        painter.draw_args({0, 52}, clock::millis()).unwrap();

        if(const auto res = oled.update(); res.is_err())
            DEBUG_PRINTLN(res.unwrap_err().unwrap_as<hal::HalError>());


        key_left.update();
        key_right.update();

        if(key_right.just_pressed()){
            menu.invoke();
        }

        if(key_left.just_pressed()){
            menu.next();
        }
    }
}

void oled_main(){
    oled_tb();
}


#endif