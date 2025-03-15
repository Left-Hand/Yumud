
#include "sys/math/real.hpp"
#include "sys/math/fast/conv.hpp"
#include "sys/math/iq/iq_t.hpp"


#include "types/vector3/Vector3.hpp"
#include "types/quat/Quat.hpp"

#include "sys/debug/debug.hpp"
#include "sys/clock/time.hpp"

#include "sys/utils/Reg.hpp"


#define UART hal::uart2

using namespace ymd;

void matrix_main(){
    UART.init(576_KHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.noBrackets();
    delay(200);



    // static constexpr Matrix_t<real_t, 3, 3> m = Matrix_t<real_t, 3, 3>{
    //     1, 2, 3, 4, 5, 6, 7, 8, 9};

}