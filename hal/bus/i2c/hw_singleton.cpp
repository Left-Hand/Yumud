#include "hw_singleton.hpp"


#if 0
#ifdef I2C1_PRESENT
    #if I2C1_REMAP == 0

        #define I2C1_SCL_PIN hal::PB<6>()
        #define I2C1_SDA_PIN hal::PB<7>()

    #elif I2C1_REMAP == 1

        #define I2C1_SCL_PIN hal::PB<8>()
        #define I2C1_SDA_PIN hal::PB<9>()

    #endif
#endif


#ifdef I2C2_PRESENT

    #if I2C2_REMAP == 0

    #define I2C2_SCL_PIN hal::PB<10>()
    #define I2C2_SDA_PIN hal::PB<11>()

    #endif
#endif

namespace ymd::hal{

#ifdef I2C1_PRESENT
I2c i2c1{I2C1};
#endif

#ifdef I2C2_PRESENT
I2c i2c2{I2C2};
#endif

}


#endif
