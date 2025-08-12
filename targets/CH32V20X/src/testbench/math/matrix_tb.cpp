
#include "core/math/real.hpp"
#include "core/math/fast/conv.hpp"
#include "core/math/iq/iq_t.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/uart/uart.hpp"

#include "types/vectors/vector3.hpp"
#include "types/vectors/quat.hpp"

#include "hal/bus/uart/uarthw.hpp"

#define UART hal::uart2

using namespace ymd;

void matrix_main(){
    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();
    clock::delay(200ms);



    // static constexpr Matrix<real_t, 3, 3> m = Matrix<real_t, 3, 3>{
    //     1, 2, 3, 4, 5, 6, 7, 8, 9};

}