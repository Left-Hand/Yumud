#include "opa.hpp"

namespace ymd::hal{

#ifdef OPA1_PRESENT
Opa<1> opa1 = {};
#endif

#ifdef OPA2_PRESENT
Opa<2> opa2 = {};
#endif



#ifdef OPA2_PRESENT
    #define OPA2_N0_GPIO hal::PB<10>()
    #define OPA2_P0_GPIO hal::PB<14>()
    #define OPA2_O0_GPIO hal::PA<2>()

    #define OPA2_N1_GPIO hal::PA<5>()
    #define OPA2_P1_GPIO hal::PA<7>()
    #define OPA2_O1_GPIO hal::PA<4>()

#endif

#ifdef OPA1_PRESENT
    #define OPA1_N0_GPIO hal::PB<11>()
    #define OPA1_P0_GPIO hal::PB<15>()
    #define OPA1_O0_GPIO hal::PA<3>()

    #define OPA1_N1_GPIO hal::PA<6>()
    #define OPA1_P1_GPIO hal::PB<0>()
    #define OPA1_O1_GPIO hal::PB<1>()
#endif


}