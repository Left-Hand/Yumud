#include "sdi.hpp"

#define DEBUG_DATA0_ADDRESS  ((volatile uint32_t*)0xE0000380)
#define DEBUG_DATA1_ADDRESS  ((volatile uint32_t*)0xE0000384)

void Sdi::write(const char data){
    while( (*(DEBUG_DATA0_ADDRESS) != 0u));
    *(DEBUG_DATA1_ADDRESS) = 0;
    *(DEBUG_DATA0_ADDRESS) = (0x01 << 24)| (data << 16);
    while(*(DEBUG_DATA0_ADDRESS));
}

void Sdi::write(const char * data_ptr, const size_t len){
    int i = 0;

    int writeSize = len;

    #define GET_DATA(n) ((size_t(i + n) < len) ? (*(data_ptr+i)) : 0)

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

void Sdi::init(){
    *DEBUG_DATA0_ADDRESS = 0u;
}