#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/HID/TM1637/TM1637.hpp"
#include "drivers/HID/HT16K33/HT16K33.hpp"


#include "drivers/HID/keycode.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define UART hal::uart2
#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()


struct ThisKeyBoardLayout{
static constexpr hid::KeyCode map(const uint8_t x, const uint8_t y){
    switch(y){
        case 0: switch(x){
            case 0: return hid::KeyCode::from_char<'E'>();
            case 1: return hid::KeyCode::from_char<'W'>();
            case 2: return hid::KeyCode::from_char<'Q'>();
            case 3: return hid::KeyCode::from_char<'-'>();
            case 4: return hid::KeyCode::from_char<'3'>();
            case 5: return hid::KeyCode::from_char<'2'>();
            case 6: return hid::KeyCode::from_char<'1'>();
        }

        case 1: switch(x){
            case 0: return hid::KeyCode::from_char<'D'>();
            case 1: return hid::KeyCode::from_char<'S'>();
            case 2: return hid::KeyCode::from_char<'A'>();
            case 3: return hid::KeyCode::from_char<'.'>();
            case 4: return hid::KeyCode::from_char<'6'>();
            case 5: return hid::KeyCode::from_char<'5'>();
            case 6: return hid::KeyCode::from_char<'4'>();
        }

        case 2: switch(x){
            case 0: return hid::KeyCode::from_char<'C'>();
            case 1: return hid::KeyCode::from_char<'X'>();
            case 2: return hid::KeyCode::from_char<'Z'>();
            case 3: return hid::KeyCode::from_char<'0'>();
            case 4: return hid::KeyCode::from_char<'7'>();
            case 5: return hid::KeyCode::from_char<'8'>();
            case 6: return hid::KeyCode::from_char<'9'>();
        }
    }
}
};

static constexpr auto a = ThisKeyBoardLayout::map(0,0).to_char().unwrap();
class KeyBoardComponent{

};



static void HT16K33_tb(HT16K33 & ht16){
    ht16.init().examine();
    while(true){
        DEBUG_PRINTLN(ht16.get_key_data(), sizeof(sstl::vector<uint8_t, 4>));
        clock::delay(40ms);
    }
}



void ht16k33_main(){
    UART.init(576000);
    DEBUGGER.retarget(&UART);
    DEBUGGER.set_eps(4);

    hal::I2cSw i2c = {SCL_GPIO, SDA_GPIO};
    i2c.init(400_KHz);

    HT16K33 ht16{
        HT16K33::Settings::SOP20Settings{},
        hal::I2cDrv{i2c, HT16K33::DEFAULT_I2C_ADDR}};

    HT16K33_tb(ht16);
}