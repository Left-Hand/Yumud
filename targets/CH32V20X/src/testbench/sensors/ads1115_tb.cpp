#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/Adc/ADS1115/ads1115.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

#define SCL_GPIO hal::PA<12>();
#define SDA_GPIO hal::PA<15>();

struct ConfigBuilder:public drivers::ADS111X_Prelude{

    [[nodiscard]]
    static constexpr Option<MUX> singleend(const size_t N){
        switch(N){
            case 0: return Some(MUX::P0NG);
            case 1: return Some(MUX::P1NG);
            case 2: return Some(MUX::P2NG);
            case 3: return Some(MUX::P3NG);
        }
        return None;
    }

    [[nodiscard]] 
    static constexpr Option<MUX> differential(const size_t P, const size_t N){

        constexpr std::array mappings{
            std::tuple{0UL,1UL,MUX::P0N1},
            std::tuple{0UL,3UL,MUX::P0N3},
            std::tuple{1UL,3UL,MUX::P1N3},
            std::tuple{2UL,3UL,MUX::P2N3}
        };
    
        // 使用范围遍历+模式匹配
        for (const auto& [valid_P, valid_N, mux_val] : mappings) {
            if (P == valid_P && N == valid_N) {
                return Some(mux_val);
            }
        }
        return None;
    }

    [[nodiscard]] 
    static constexpr Option<DataRate> datarate(const size_t dr){
        switch(dr){
            case 8: return Some(DataRate::_8);
            case 16: return Some(DataRate::_16);
            case 32: return Some(DataRate::_32);
            case 64: return Some(DataRate::_64);
            case 128: return Some(DataRate::_128);
            case 250: return Some(DataRate::_250);
            case 475: return Some(DataRate::_475);
            case 860: return Some(DataRate::_860);
        }
        return None;
    }

    Result<void, void> apply(){
        return Ok{};
    }
};

void ads1115_main()
{

    hal::uart2.init({576000});
    DEBUGGER.retarget(&hal::uart2);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");

    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;
    auto i2c = hal::I2cSw(&scl_gpio_, &sda_gpio_);
    i2c.init({400_KHz});

    drivers::ADS1115 ads{&i2c};

    static constexpr auto datarate = ConfigBuilder().datarate(860).unwrap();
    static constexpr auto mux = ConfigBuilder().differential(2,3).unwrap();
    ads.set_data_rate(datarate).examine();
    ads.set_mux(mux).examine();
    // ads.setMux(ads.builder().singleend(0).unwarp());
    ads.set_pga(drivers::ADS1115::PGA::_1_024V).examine();
    ads.enable_cont_mode(EN).examine();
    ads.start_conv().examine();

    while(true){
        // if(ads.ready()){
            DEBUG_PRINTLN(clock::millis(), ads.get_voltage().unwrap());
        // }

        // ads.
        clock::delay(2ms);
    }

}