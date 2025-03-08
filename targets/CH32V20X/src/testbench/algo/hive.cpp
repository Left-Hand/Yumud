#include "../tb.h"

#include "sys/math/real.hpp"

#include "sys/debug/debug.hpp"
#include "sys/clock/time.hpp"
#include "sys/clock/clock.h"

#include "sys/utils/hive.hpp"
#include "types/vector2/vector2_t.hpp"

void hive_main(){
    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");
    DEBUGGER.noBrackets();

    using num_type = int;
    std::hive<num_type> i_hive;

    // Total the remaining ints:
    num_type total = 0;


    // Insert 100 ints:
    for(size_t i = 0; i != 300; ++i){
        i_hive.insert(num_type(i));
    }

    const auto begin_m = micros();
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

    DEBUG_PRINTLN(total, uint32_t(micros() - begin_m));

    // DEBUG_PRINTLN(Vector2(1,2));
    std::terminate();
    DEBUG_PRINTLN("Hello?");
    while(true);
}