#pragma once

#include "spi.hpp"


extern"C"{
#ifdef SPI1_PRESENT
__interrupt void SPI1_IRQHandler(void);
#endif

#ifdef SPI2_PRESENT
__interrupt void SPI2_IRQHandler(void);
#endif

#ifdef SPI3_PRESENT
__interrupt void SPI3_IRQHandler(void);
#endif
}


namespace ymd::hal{

#ifdef SPI1_PRESENT
extern Spi spi1;
#endif

#ifdef SPI2_PRESENT
extern Spi spi2;
#endif

#ifdef SPI3_PRESENT
extern Spi spi3;
#endif

}