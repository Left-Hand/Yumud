#include "../tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"

#include "drivers/Adc/ADS1115/ads1115.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;

namespace ymd{
OutputStream & operator << (OutputStream & os, const Result<auto, auto> & result){
    if(result.is_ok()) return os << "[OK]:" << result.unwrap();
    else return os << "[ERR]:" << result.unwrap_err();
}
}

// Option result = Some(2);
// using ok_type = Result<int, const char *>::ok_type;
// using err_type = Result<int, const char *>::err_type;
// Result<void, int> result = Ok(2);

// auto squared = result
//     | [](auto x) { return (x > 0) ? Result<int, const char *>{Ok{x * x}} : Result<int, const char *>(Err<const char *>("minus")); }  // 返回新的Result
//     | [](auto x) { return (x > 9) ? Result<int, const char *>{Ok{x + 1}} : Result<int, const char *>(Err<const char *>("small")); }  // 返回新的Result
// ;
//注释
    // auto other = result
    //     .match([](auto x) {return x * x;}, [](auto str){DEBUG_PRINTLN(str);})
    // ;

    // squared.

    // Result<size_t, size_t> result2 = Ok<size_t>{0};
    // sizeof(result2);
    // using Sto =Result<size_t, size_t>::Storage;
    // DEBUG_PRINTLN(squared);
    
    // while(true);


// static __no_inline auto test_result(int _x){
//     Result<int, const char *> result = Ok(_x);

//     auto squared = result
//         .transform([](auto && x){
//             return rescond(x > 0, x * x, "minus");
//         })
//         .transform([](auto && x){
//             return rescond(x > 9, x + 1, "small");
//         });
    
//     return squared;
// }

void ads1115_main()
{

    hal::uart2.init({576000});
    DEBUGGER.retarget(&hal::uart2);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    // DEBUGGER.no_brackets();

    // DEBUG_PRINTLN(test_result(4));
    // DEBUG_PRINTLN(test_result(4).loc().expect("no"));
    while(true);
    auto i2c = hal::I2cSw(&hal::PA<12>(), &hal::PA<15>());
    i2c.init(400_KHz);

    drivers::ADS1115 ads{&i2c};

    ads.set_data_rate(ads.builder().datarate(860).unwrap()).examine();
    ads.set_mux(ads.builder().differential(2,3).unwrap()).examine();
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