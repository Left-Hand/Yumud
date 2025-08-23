#include <atomic>
#include "core/sync/spinlock.hpp"
#include "core/sync/barrier.hpp"
#include <barrier>

#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "src/testbench/tb.h"

#include "drivers/HID/prelude/keycode.hpp"
#include "drivers/HID/prelude/axis_input.hpp"
#include "drivers/HID/prelude/segcode.hpp"
#include "drivers/HID/prelude/button_input.hpp"
#include "drivers/HID/ft6336u/ft6336u.hpp"
#include "drivers/Display/Polychrome/ST7789/st7789v3_phy.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using drivers::FT6336U;

#define UART hal::uart2

#define SCL_GPIO hal::PB<0>()
#define SDA_GPIO hal::PB<1>()

template<typename T, typename Iter>
struct MyU18BurstPixelDataIter{
    static constexpr bool DATA_BIT = 1;

    constexpr explicit MyU18BurstPixelDataIter(const Iter iter):
        iter_(iter) {}

    constexpr bool has_next() const {
        return is_runout_ == false;
    };

    constexpr uint16_t next() {
        const bool iter_has_next = iter_.has_next();
        if((queue_.writable_capacity() > 18) and iter_has_next){
            const uint16_t next_u16 = iter_.next();
            queue_.push_bit(DATA_BIT);
            queue_.push_bits<8>(next_u16 >> 8);
            queue_.push_bit(DATA_BIT);
            queue_.push_bits<8>(next_u16 & 0xff);
        }


        {
            const auto ava = queue_.available();
            if(ava >= 16){
                return queue_.pop_bits<16>();
            }else{
                if(not iter_has_next) is_runout_ = true;
                return queue_.pop_remaining() << (16 - ava);
            }
        }
    };
private:
    Iter iter_;
    BitsQueue queue_;
    bool is_runout_ = false;
};

void ft6336_main(){
    // UART.init({576_KHz});
    UART.init({
        .baudrate = 576000,
        .tx_strategy = CommStrategy::Dma
    });
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();

    
    auto & led = hal::PA<15>();
    led.outpp();

    // while(true){
    //     DEBUG_PRINTLN(clock::millis());
    //     clock::delay(30ms);
    //     led.toggle();
    // }

    // test_result();
    hal::I2cSw i2c{&SCL_GPIO, &SDA_GPIO};
    i2c.init(200_KHz);

    clock::delay(1ms);
    clock::delay(100ms);

    FT6336U ft6336{&i2c};
    ft6336.init().examine();

    while(true){
        DEBUG_PRINTLN(
            // ft6336.get_gesture_id().examine(),
            ft6336.get_touch_points().examine().iter(),
            std::hex, 
            std::showbase,
            // RepeatIter<uint16_t>{0X5555, 3}
            // OnceIter(0X5555)
            // RepeatIter<uint16_t>{0X5555, 3},
            MyU18BurstPixelDataIter<uint16_t, RepeatIter<uint16_t>>(RepeatIter<uint16_t>{0X5555, 2})
        );

        clock::delay(5ms);
    }
}