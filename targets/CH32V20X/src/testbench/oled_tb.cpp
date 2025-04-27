#include "tb.h"

#include "core/debug/debug.hpp"
#include "core/string/string.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/Display/Monochrome/SSD1306/ssd1306.hpp"
#include "drivers/CommonIO/Key/Key.hpp"


#include "types/image/painter.hpp"
#include "hal/bus/uart/uarthw.hpp"


using namespace GpioUtils;
using namespace ymd::drivers;
using namespace ymd;


#define UART hal::uart2

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
    real_t para = 0;
public:
    Menu(VerticalBinaryImage & _frame, OutputStream & _os):frame_(_frame), os_(_os){
        painter_.bindImage(frame_);
        painter_.setColor(Binary(true));
    }

    void render(){
        frame_.fill(0);
        painter_.drawString({0,0}, String(millis()));

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

        // painter_.drawString({0,8}, "func:" + str);
        painter_.drawString({0,8}, "func:");
        painter_.drawString({0,16}, send_str);
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
                para += real_t(0.3);
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
    auto & SCL_GPIO = portB[6];
    auto & SDA_GPIO = portB[7];
    static constexpr auto I2C_BAUD = 1000;
    static constexpr auto MONITOR_HZ = 5000;

    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_splitter(' ');

    Key key_left{portB[2], LOW};
    Key key_right{portB[1], LOW};

    key_left.init();
    key_right.init();
    delay(100);

    // I2cSw i2c{portB[13], portB[15]};
    I2cSw i2c{SCL_GPIO, SDA_GPIO};
    // i2c.init(0);
    i2c.init(I2C_BAUD);

    DisplayerPhyI2c oled_phy{i2c};

    // auto oled = SSD13XX(oled_phy, SSD13XX_72X40_Config());
    constexpr auto cfg = SSD13XX_128X64_Config();

    auto oled = SSD13XX(oled_phy, cfg);
    auto & frame = oled.fetch_frame();
    
    // DEBUG_PRINTLN("init started");

    hal::timer1.init(MONITOR_HZ);
    hal::timer1.attach(TimerIT::Update,{0,0},[&]{
        DEBUG_PRINTLN(bool(SCL_GPIO.read()), ',', bool(SDA_GPIO.read()));
    });

    oled.init().unwrap();
    oled.enable_flip_x(false).unwrap();
    oled.enable_flip_y(false).unwrap();

    Menu menu {frame, DEBUGGER};

    // DEBUG_PRINTLN("app started");
    while(true){
        bool i = false;
        i = !i;
        oled.turn_display(i).unwrap();
        delay(1);
        menu.render();
        // oled.update().unwrap();
        
        // oled.fill(Binary::WHITE);
        // if(res.is_err()) DEBUG_PRINTLN("err: ", res.unwrap_err().as<hal::BusError>().unwrap());
        // DEBUG_PRINTLN(millis());
        // DEBUG_PRINTLN("err: ", hal::BusError(hal::BusError::AckTimeout));
        // DEBUG_PRINTLN("err: ", hal::BusError(hal::BusError::Ok()));

        key_left.update();
        key_right.update();

        if(key_right.pressed()){
            menu.invoke();
        }

        if(key_left.pressed()){
            menu.next();
        }
    }
}

void oled_main(){
    oled_tb();
}