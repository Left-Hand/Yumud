#include "VL6180X.hpp"

// #define VL6180X_DEBUG

#ifdef VL6180X_DEBUG
#undef VL6180X_DEBUG
#define VL6180X_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define VL6180X_PANIC(...) PANIC(__VA_ARGS__)
#define VL6180X_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define VL6180X_DEBUG(...)
#define VL6180X_PANIC(...)  PANIC()
#define VL6180X_ASSERT(cond, ...) ASSERT(cond)
#endif

using namespace ymd;

hal::HalResult VL6180X::write_reg(const RegAddress command, const uint8_t data){
    return i2c_drv_.write_reg(command, data);
}


hal::HalResult VL6180X::read_reg(const RegAddress command, uint8_t & data){
    return i2c_drv_.write_reg(command, data);
}

