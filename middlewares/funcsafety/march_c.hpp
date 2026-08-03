#pragma once

#include <cstdint>


namespace ymd::funcsafety{

__attribute__((optimize("-Os")))
uint8_t * march_c_test(uint8_t * const begin, uint8_t * const end);

}