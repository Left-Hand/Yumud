#include "core/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/HID/TM1637/TM1637.hpp"
#include "drivers/HID/HT16K33/HT16K33.hpp"


#include "drivers/HID/keycode.hpp"
#include "drivers/HID/axis_input.hpp"
#include "drivers/HID/button_input.hpp"

#include <atomic>
#include "core/sync/spinlock.hpp"
#include "core/sync/barrier.hpp"
#include <barrier>


using namespace ymd;
using namespace ymd::drivers;

#define UART hal::uart2
#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()





// static constexpr auto a = ThisKeyBoardLayout::map(0,0).to_char().unwrap();

namespace ymd::hid{

template<typename T>
class KeyBoardComponent{
};


template<typename T>
struct KeyBoardLayout{
};

template<>
struct KeyBoardLayout<HT16K33>{
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
            default: break;
        }

        case 1: switch(x){
            case 0: return hid::KeyCode::from_char<'D'>();
            case 1: return hid::KeyCode::from_char<'S'>();
            case 2: return hid::KeyCode::from_char<'A'>();
            case 3: return hid::KeyCode::from_char<'.'>();
            case 4: return hid::KeyCode::from_char<'6'>();
            case 5: return hid::KeyCode::from_char<'5'>();
            case 6: return hid::KeyCode::from_char<'4'>();
            default: break;
        }

        case 2: switch(x){
            case 0: return hid::KeyCode::from_char<'C'>();
            case 1: return hid::KeyCode::from_char<'X'>();
            case 2: return hid::KeyCode::from_char<'Z'>();
            case 3: return hid::KeyCode::from_char<'0'>();
            case 4: return hid::KeyCode::from_char<'9'>();
            case 5: return hid::KeyCode::from_char<'8'>();
            case 6: return hid::KeyCode::from_char<'7'>();
            default: break;
        } 
        default: break;
    }
    PANIC(x,y);
}
};

template<>
class KeyBoardComponent<drivers::HT16K33>{
public:
    using Error = drivers::HT16K33::Error;

    template<typename T = void>
    using IResult = Result<T, Error>;

    KeyBoardComponent(
        HT16K33 & inst):
        inst_(inst)
    {;}

    IResult<> update(){
        if(const auto res = inst_.any_key_pressed();
            res.is_err()) return Err(res.unwrap_err());
        else if(res.unwrap() == false){
            input_.update(None);
            return Ok();
        }

        const auto next_key_data = ({
            const auto res = inst_.get_key_data();
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        const auto may_keycode = [&] -> Option<KeyCode>{
            const auto may_xy = next_key_data.to_first_xy();;
            if(may_xy.is_none()) return None;
            const auto [x,y] = may_xy.unwrap();
            return Some(KeyBoardLayout<HT16K33>::map(x,y));
        }();

        // DEBUG_PRINTLN(may_keycode.map([](const KeyCode code){
        //     return code.to_char();
        // }).flatten());

        input_.update(may_keycode);
        return Ok();
    }
    constexpr const auto & input() const noexcept { return input_; }
private:
    HT16K33 & inst_;
    ButtonInput<KeyCode> input_;
};

}

// template<size_t N>
// class FixedString{
//     // static constexpr Option<FixedString> from_str(const StringView sv){

//     // }

//     constexpr FixedString(const StringView str):
//         len_(MIN(str.length(), N))
//     {
//         // memcpy(buf_, str.data(), len_);
//         for(size_t i = 0; i < len_){
//             buf_[i] = str.data()[i];
//         }
//     }

//     constexpr operator StringView() const {
//         return StringView(buf_, len);
//     }
// private:

//     char[N] buf_ = {};
//     size_t len_;
// };

// class LineInput{
// public:
//     void input(hid::KeyCode code){
        
//     }
// private:
//     FixedString str_;
//     size_t cursor_;
// }



static void HT16K33_tb(HT16K33 & ht16){
    ht16.init({.pulse_duty = HT16K33::PulseDuty::_15_16}).examine();


    uint8_t cnt = 0;
    // sync::SpinLock locker;
    // locker.lock();
    // locker.unlock();
    // locker.lock();

    // std::atomic_bool a{false};

    hid::KeyBoardComponent<drivers::HT16K33> comp{ht16};

    while(true){
        cnt++;

        // const auto keydata = ht16.get_key_data().examine();
        // const auto may_xy = keydata
        //     .to_first_xy();

        // const auto may_token = may_xy 
        //     .map([](std::tuple<uint8_t, uint8_t> xy){
        //         const auto [x,y] = xy;
        //         return hid::KeyBoardLayout<HT16K33>::map(x,y).to_char();
        //     })
        //     .flatten()
        //     ;
        comp.update().examine();

        {
            auto test_pattern = std::array<uint8_t, 16>{};
            test_pattern.fill(cnt);
            ht16.update_displayer(0, std::span(test_pattern)).examine();
        }


        // DEBUG_PRINTLN(may_xy, may_token);
        DEBUG_PRINTLN(comp.input());
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