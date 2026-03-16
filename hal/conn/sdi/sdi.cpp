#include "sdi.hpp"

using namespace ymd;
using namespace ymd::hal;


void Sdi::blocking_write_byte(const uint8_t byte){
    return lld::sdi_blocking_write_byte(byte);
}

void Sdi::blocking_write_bytes(std::span<const uint8_t> bytes){
    return lld::sdi_blocking_write_bytes(bytes);
}

size_t Sdi::free_capacity(){
    return lld::sdi_free_capacity();
}

void Sdi::init(){
    return lld::sdi_init();
}

namespace ymd::hal{
#ifdef ENABLE_SDI
Sdi sdi;
#endif
}