#include "src/testbench/tb.h"
#include "sys/debug/debug.hpp"

#include "sys/math/real.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "utils/rustlike/Result.hpp"
#include <bitset>

using namespace ymd;

struct FoundInfo{
    uint8_t addr;
    uint max_bbaud;
};

struct I2cTester{
    static constexpr uint start_freq = 200_KHz;
    static constexpr auto grow_scale = 2;

    static Result<uint, BusError> getMaxBaudRate(I2c & i2c, const uint8_t read_addr){
        auto i2c_drv = hal::I2cDrv{i2c, read_addr};
        // if(auto err = i2c_drv.verify(); err.wrong()){
        //     return err; 
        // }
        const uint max_baud = [&]{
            uint baud = start_freq;
            while(baud < 10_MHz){
                i2c_drv.setBaudRate(uint(baud * grow_scale));
                const auto err = i2c_drv.verify();
                if(err.wrong()) break;

                baud = baud + (baud >> 1);
                delay(2);
            }

            return baud;
        }();


        return max_baud;
    }
    static BusError verify(I2c & i2c, const uint8_t read_addr, const uint bbaud = start_freq){
        return hal::I2cDrv{i2c, read_addr}.verify();
    }

};




auto i2c_addresses() -> std::ranges::view auto {
    return std::views::iota(0, 128) 
        | std::views::transform([](uint8_t i) { return i << 1; });
}


void i2c_scanner_functional(){

    auto & logger = uart1;
    logger.init(576_KHz);
    DEBUGGER.retarget(&logger);

    // log("i2c_scanner_main") | log("1");
    
    I2cSw i2c = {portA[12], portA[15]};
    i2c.init(100_KHz);

    const uint8_t read_addr = 0x08;

    
    I2cTester::getMaxBaudRate(i2c, read_addr)
        .then([](uint baud) { 
            return (baud > 100_KHz) ? Ok(baud) : Result<uint, BusError>(BusError::OCCUPIED); 
        });
}


template<typename ... Args>
auto log(Args && ... args) {
    return std::views::transform(
        [&args...](auto&& value) {
            DEBUG_PRINTS(std::forward<Args>(args)..., value);
            return std::forward<decltype(value)>(value);
        }
    );
}

void i2c_scanner_main(){
    auto & logger = uart1;
    logger.init(576_KHz);
    DEBUGGER.retarget(&logger);
    
    I2cSw i2c = {portA[12], portA[15]};
    i2c.init(100_KHz);
    
    // auto data = std::vector{1, 2, 3};
    // 遍历时打印每个元素
    // for (int n : data | log("Value: ")) { /* ... */ }
    // 输出：
    // Value: 1
    // Value: 2
    // Value: 3

    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Scan Start");
    DEBUG_PRINTLN("---------");

    {
        std::vector<FoundInfo> founded_devices;

        for(uint8_t i = 0; i < 128; i++){
            auto read_addr = (i << 1);
            auto err = I2cTester::verify(i2c, read_addr);
            if(err.ok()){
                const auto result = I2cTester::getMaxBaudRate(i2c, read_addr);
                if(result.ok()){
                    founded_devices.emplace_back(read_addr, result.unwrap());
                }else{
                    DEBUG_PRINTS("Device ", read_addr, " is not responding" );
                }
            }
            delay(1);
        }
        
        BREAKPOINT;
        if(founded_devices.size()){
            DEBUG_PRINTS("Found", founded_devices.size(), " devices" );
            for(auto & [read_addr, baud] : founded_devices){
                DEBUG_PRINTS(
                    std::bitset<8>(read_addr), '\t',
                    "|  Max baud is: ", baud, "Hz"
                );

                delay(1);
            }
        }else{
            DEBUG_PRINTLN("No device found");
        }

    }
    DEBUG_PRINTLN("---------");
    DEBUG_PRINTLN("Scan done, Click reset to restart");

    exit(0);
}