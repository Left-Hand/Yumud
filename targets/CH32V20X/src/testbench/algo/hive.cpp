#include "../tb.h"

#include "core/math/real.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"

#include "core/utils/cpp_stl/hive.hpp"
#include "algebra/vectors/vec2.hpp"


#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;


void hive_main(){
    hal::usart2.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000
    });
    DEBUGGER.retarget(&hal::usart2);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");
    DEBUGGER.no_brackets(EN);

    using num_type = int;
    std::hive<num_type> i_hive;

    // Total the remaining ints:
    num_type total = 0;


    // Insert 100 ints:
    for(size_t i = 0; i != 300; ++i){
        i_hive.insert(num_type(i));
    }

    const auto begin_ms = clock::micros();
    // for(size_t i = 0; i != 300; ++i)
    {
    
        // Erase half of them:
        // sizeof(std::hive<num_type>::);
        for (auto it = i_hive.begin(); it != i_hive.end(); ++it){
            it = i_hive.erase(it);
        }
    

    
        for (auto it = i_hive.begin(); it != i_hive.end(); ++it){
            total += *it;
        }
    }

    DEBUG_PRINTLN(total, clock::micros() - begin_ms);

    // DEBUG_PRINTLN(Vec2(1,2));
    std::terminate();
    DEBUG_PRINTLN("Hello?");
    while(true);
}