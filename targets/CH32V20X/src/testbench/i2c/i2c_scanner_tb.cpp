#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"
#include "core/utils/Result.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include <bitset>
#include <ranges>


using namespace ymd;
using namespace ymd::hal;

// #define UART DEBUGGER_INST
#define UART hal::uart2

#if 0
#define SCL_GPIO hal::PD<1>()
#define SDA_GPIO hal::PD<0>()
#else
// #define SCL_GPIO hal::PB<6>()
// #define SDA_GPIO hal::PB<7>()
// #define SCL_GPIO hal::PB<3>()
// #define SDA_GPIO hal::PB<5>()
// #define SCL_GPIO hal::PD<2>()
// #define SDA_GPIO hal::PC<12>()

#define SCL_GPIO hal::PB<3>()
#define SDA_GPIO hal::PB<5>()
// #define SCL_GPIO hal::PC<12>()
// #define SDA_GPIO hal::PD<2>()
#endif
struct FoundInfo{
    uint8_t addr;
    uint32_t max_bbaud;
};


struct I2cTester{
    static constexpr uint32_t start_freq = 200_KHz;
    static constexpr auto grow_scale = 2;
    
    static Result<uint32_t, hal::HalError> get_max_baudrate(I2c & i2c, const uint8_t read_addr){
        hal::I2cDrv i2c_drv{&i2c, I2cSlaveAddr<7>::from_u7(read_addr >> 1)};

        const uint32_t max_baud = [&]{
            uint32_t baud = start_freq;
            while(baud < 10_MHz){
                // i2c_drv.set_baudrate(uint32_t(baud * grow_scale));
                i2c.set_baudrate(uint32_t(baud * grow_scale));
                const auto err = i2c_drv.validate();
                if(err.is_err()) break;

                baud = baud + (baud >> 1);
                clock::delay(1ms);
            }

            return baud;
        }();

        // DEBUG_PRINTLN("??");
        uart2.set_rx_strategy(CommStrategy::Blocking);

        return Ok{max_baud};
    }
    static Result<void, hal::HalError> validate(I2c & i2c, const uint8_t read_addr, const uint32_t bbaud = start_freq){
        const auto res = hal::I2cDrv{&i2c, I2cSlaveAddr<7>::from_u7(read_addr >> 1)}.validate();
        if(res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }

};


void i2c_scanner_main(){
    UART.init({
        .baudrate = 576_KHz,
    });
    DEBUGGER.retarget(&UART);
    // DEBUGGER.force_sync();
    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;
    I2cSw i2c{&scl_gpio_, &sda_gpio_};
    i2c.init({100_KHz});


    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Scan Start");


    {
        std::vector<FoundInfo> founded_devices;


        for(uint8_t i = 0; i < 128; i++){
            const uint8_t read_addr = i << 1;
            const auto res = I2cTester::validate(i2c, read_addr);
            (void)res.inspect([&]{
                const auto baud_res = I2cTester::get_max_baudrate(i2c, read_addr);
                if(baud_res.is_err()){
                    PANIC("addr buad can't be measured");
                }

                founded_devices.emplace_back(
                    read_addr, 
                    baud_res.unwrap()
                );
            });
            clock::delay(1ms);
        }

        clock::delay(10ms);

        if(founded_devices.size()){
            DEBUG_PRINTS("Found", founded_devices.size(), " devices" );
            DEBUG_PRINTLN("---------");
            {
                const auto guard = DEBUGGER.create_guard();
                DEBUGGER.set_indent(1);
                for(auto & [read_addr, baud] : founded_devices){
                    DEBUG_PRINTS(
                        std::bitset<8>(read_addr), '\t',
                        "|  Max baud is: ", baud, "Hz"
                    );

                    clock::delay(1ms);
                }
                // const Result res = Err{0};
                // res.loc().expect("unknown bug");

            }
            DEBUG_PRINTLN("---------");
        }else{
            DEBUG_PRINTLN("No device found");
        }

    }

    DEBUG_PRINTLN("Scan done, Click reset to restart");

    while(true);
}