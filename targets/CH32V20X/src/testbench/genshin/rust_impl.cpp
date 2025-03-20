#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"
#include "sys/clock/time.hpp"


#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/wireless/Radio/LT8960/LT8960L.hpp"
#include "src/testbench/algo/utils.hpp"

#include "drivers/Actuator/SVPWM/svpwm3.hpp"

using namespace ymd;

namespace ymd{
template<typename T, typename U>
struct ImplFor{};

struct MyStruct {
    int private_data;

    // template<typename T>
    // friend struct ImplFor<T, MyStruct>;
    template<typename T, typename U>
    friend struct ImplFor;
};



// template<typename T>
// struct ImplFor<T, MyStruct>{
//     static void method(U &) { 
//         DEBUGGER << "Default method\n";
//     }
// };

template<>
struct ImplFor<int, MyStruct> {
    static void method(MyStruct& obj) {
        obj.private_data = 42;
        DEBUGGER << "Set to 42 via int\n";
    }
};

template<>
struct ImplFor<real_t, MyStruct> {
    static void method(MyStruct& obj) {
        obj.private_data = int(42.0_r);
        DEBUGGER << "Set to 42.0 via real_t\n";
    }
};

template<typename T>
void my_method(MyStruct & obj) {
    ImplFor<T, MyStruct>::method(obj);
}

void genshin_main() {
    MyStruct obj;
    my_method<int>(obj); // 输出：Set to 42 via int
    my_method<real_t>(obj); // 输出：Set to 42.0 via real_t
}

}