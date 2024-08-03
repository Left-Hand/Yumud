#include "tb.h"
#include "drivers/Display/Monochrome/SSD1306/ssd1306.hpp"
#include "drivers/CommonIO/Key/Key.hpp"

#include <array>
using namespace GpioUtils;

// auto funcs = std::to_array({
//     "cali",
//     "cd",
//     "save",
// });

class Menu{
protected:
    enum class Func:uint8_t{
        NONE,
        CALI,
        CD,
        SAVE
    };

    Func func_ = Func::NONE;

    VerticalBinaryImage & frame_;
    OutputStream & os_;
    Painter<Binary> painter_;

public:
    Menu(VerticalBinaryImage & _frame, OutputStream & _os):frame_(_frame), os_(_os){
        painter_.bindImage(frame_);
        painter_.setColor(Binary(true));
    }

    void render(){
        frame_.fill(0);
        painter_.drawString({0,0}, "func");

        String str;

        switch(func_){
            case Func::NONE:
                str = "none";
                break;
            case Func::CALI:
                str = "cali";
                break;
            case Func::CD:
                str = "cd";
                break;
            case Func::SAVE:
                str = "save";
                break;
        }

        painter_.drawString({0,8}, str);
    }

    void next(){
        switch(func_){
            case Func::NONE:
                func_ = Func::CALI;
                break;
            case Func::CALI:
                func_ = Func::CD;
                break;
            case Func::CD:
                func_ = Func::SAVE;
                break;
            case Func::SAVE:
                func_ = Func::NONE;
               break;
        }
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
            case Func::CD:
                str = "cd";
                break;
            case Func::SAVE:
                str = "save";
                break;
        }

        os_.println(str);
    }
};

static void oled_tb(){
    auto & debugger = uart1;
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

        if(key_right){
            menu.invoke();
        }


        if(key_left){
            menu.next();
        }
    }
}

void oled_main(){
    oled_tb();
}