#include "hw_singleton.hpp"

namespace ymd::hal{
#ifdef SPI1_PRESENT
Spi spi1{ral::SPI1_Inst};
#endif

#ifdef SPI2_PRESENT
Spi spi2{ral::SPI2_Inst};
#endif

#ifdef SPI3_PRESENT
Spi spi3{ral::SPI3_Inst};
#endif
}

#ifdef SPI1_PRESENT
void SPI1_IRQHandler(void){
    //TODO
    __builtin_trap();
}

#endif

#ifdef SPI2_PRESENT
void SPI2_IRQHandler(void){
    //TODO
    __builtin_trap();
}
#endif

#ifdef SPI3_PRESENT
void SPI3_IRQHandler(void){
    //TODO
    __builtin_trap();
}
#endif