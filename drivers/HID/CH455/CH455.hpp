#pragma once

// 这个驱动已经完成
// 这个驱动还未测试

// CH455 是数码管显示驱动和键盘扫描控制芯片。CH455内置时钟振荡电路，可以动态驱动4位数
// 码管或者32只LED；同时还可以进行28键的键盘扫描；CH455通过SCL和SDA组成的2线串行接口
// 与单片机等交换数据。 

// 接口
    // I2c(1MHz)
    //  Scl: Gpio;
    //  Sda: Gpio;
    //  ~Interrupt: Option<Gpio>;

// 错误码:
    // BrightnessGreaterThanMax,
    // BrightnessLessThanMin,
    // ReadKeyBit2VerifyFailed,
    // ReadKeyBit7VerifyFailed,


#include "details/CH455_Prelude.hpp"
#include "drivers/HID/Event.hpp"


namespace ymd::drivers{

class CH455 final:public CH455_Prelude{
public:
    struct Config{
        Borrow<hal::Gpio> scl_gpio;
        Borrow<hal::Gpio> sda_gpio;
        Option<hal::Gpio &> int_gpio;
    };

    CH455(const Config & cfg)
        : phy_(cfg.scl_gpio.unwrap(), cfg.sda_gpio.unwrap()){;}

    IResult<> init();

    IResult<> enable_seg7_mode(Enable en = EN);

    IResult<> set_digit(const uint8_t digit, const uint8_t code);

    IResult<> display_digit(const uint8_t digit, const uint8_t value);

    IResult<KeyEvent> get_key();

    template<typename Fn>
    void register_key_callback(Fn && fn) {

    }
private:
    IResult<> set_brightness(const uint8_t brightness);
    CH455_phy phy_;

    // using Callback = std::function<void(MatrixKeyEvent)>;
};

}