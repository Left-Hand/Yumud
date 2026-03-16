#include "sdi.hpp"

#include <cstdint>

using namespace ymd;
using namespace ymd::hal;


#define DEBUG_DATA0_ADDRESS  ((volatile uint32_t*)0xE0000380)
#define DEBUG_DATA1_ADDRESS  ((volatile uint32_t*)0xE0000384)

namespace ymd::lld{
void sdi_blocking_write_byte(const uint8_t byte){
    while( (*(DEBUG_DATA0_ADDRESS) != 0u));
    *(DEBUG_DATA1_ADDRESS) = 0;
    *(DEBUG_DATA0_ADDRESS) = (0x01 << 24)| (byte << 16);
    while(*(DEBUG_DATA0_ADDRESS));
}

void sdi_blocking_write_bytes(std::span<const uint8_t> bytes){
    int i = 0;

    size_t len = bytes.size();
    int writeSize = len;

    #define GET_DATA(n) ((size_t(i + n) < len) ? (bytes[i]) : 0)

    #define WRITE(size) \
    *(DEBUG_DATA1_ADDRESS) = GET_DATA(3) | (GET_DATA(4)<<8) | (GET_DATA(5)<<16) | (GET_DATA(6)<<24);\
    *(DEBUG_DATA0_ADDRESS) = (size) | (GET_DATA(0)<<8) | (GET_DATA(1)<<16) | (GET_DATA(2)<<24);\

    do{
        while( (*(DEBUG_DATA0_ADDRESS) != 0u));

        if(writeSize>7){
            WRITE(7);

            i += 7;
            writeSize -= 7;
        }else{
            WRITE(writeSize);

            writeSize = 0;
        }

    } while (writeSize);

    while(*(DEBUG_DATA0_ADDRESS));
}

size_t sdi_free_capacity(){
    return (*(DEBUG_DATA0_ADDRESS) == 0u) ? 0 : 1;
}

void sdi_init(){
    *DEBUG_DATA0_ADDRESS = 0u;
}

}