#include "march_c.hpp"


namespace ymd::funcsafety{

uint8_t * march_c_test(uint8_t * const begin, uint8_t * const end){
    uint8_t * p = begin;
    while (p < end) {
        *p = 0x00;
        p++;
    }
    p = begin;
    while (p < end) {
        if (*p != 0x00)
            return p;
        *p = 0xff;
        p++;
    }
    p = begin;
    while (p < end) {
        if (*p != 0xff)
            return p;
        *p = 0x00;
        p++;
    }
    p = end - 1;
    while (p >= begin) {
        if (*p != 0x00)
            return p;
        *p = 0xff;
        p--;
    }
    p = end - 1;
    while (p >= begin) {
        if (*p != 0xff)
            return p;
        *p = 0x00;
        p--;
    }
    p = end - 1;
    while (p >= begin) {
        if (*p != 0x00)
            return p;
        p--;
    }
    return nullptr;
}

}