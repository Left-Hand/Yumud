#include "tb.h"
#include "drivers/Display/Monochrome/SSD1306/ssd1306.hpp"
#include "types/image/painter.hpp"
#include "drivers/CommonIO/Key/Key.hpp"

#include <array>
using namespace GpioUtils;

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

        painter_.drawString({0,8}, "func:" + str);
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
                str = "ez " + toString(para);
                para += 0.3;
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
    auto & debugger = DEBUGGER;
    debugger.init(DEBUG_UART_BAUD);

    auto & printer = uart2;
    printer.init(DEBUG_UART_BAUD);

    Key key_left{portB[2], LOW};
    Key key_right{portB[1], LOW};

    key_left.init();
    key_right.init();
    delay(200);

    I2cSw i2c{portB[13], portB[15]};
    i2c.init(0);

    OledInterfaceI2c oled_if{i2c};

    SSD13XX_72X40 oled{oled_if};
    auto & frame = oled.fetchFrame();
    
    oled.init();
    oled.enableFlipX(false);
    oled.enableFlipY(false);

    Menu menu {frame, printer};
    while(true){

        menu.render();
        oled.update();

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