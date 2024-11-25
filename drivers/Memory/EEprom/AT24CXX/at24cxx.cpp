#include "at24cxx.hpp"

using namespace ymd;

// #define AT24CXX_DEBUG

#ifdef AT24CXX_DEBUG
#undef AT24CXX_DEBUG
#define AT24CXX_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define AT24CXX_PANIC(...) PANIC(__VA_ARGS__)
#define AT24CXX_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define AT24CXX_DEBUG(...)
#define AT24CXX_PANIC(...)  PANIC()
#define AT24CXX_ASSERT(cond, ...) ASSERT(cond)
#endif


#define ACCESS_STRICT_PROTECT

#define WRITE_POOL(addr, ptr, len)\
if(is_small_chip())\
    i2c_drv_.writeMulti((uint8_t)addr, ptr, len);\
else\
    i2c_drv_.writeMulti((uint16_t)addr, ptr, len);\

#define READ_POOL(addr, ptr, len)\
if(is_small_chip())\
    i2c_drv_.readMulti((uint8_t)addr, ptr, len);\
else\
    i2c_drv_.readMulti((uint16_t)addr, ptr, len);\

#define WRITE_REG(loc, data)\
if(is_small_chip()) i2c_drv_.writeReg((uint8_t)loc, data, LSB);\
else i2c_drv_.writeReg((uint16_t)loc, data, LSB);\

#define READ_REG(loc, data)\
if(is_small_chip()) i2c_drv_.readReg((uint8_t)loc, data, LSB);\
else i2c_drv_.readReg((uint16_t)loc, data, LSB);\

#ifdef ACCESS_STRICT_PROTECT
#define CHECK_ADDR(loc)\
AT24CXX_ASSERT((loc <= capacity_), "invalid addr", loc, capacity_)
#else

#define CHECK_ADDR(loc)\
if(loc > m_capacity){\
    AT24CXX_ASSERT("invalid addr");\
    return;\
}

#endif


void AT24CXX::wait_for_free(){
    uint32_t delays;
    if(last_entry_ms == 0){
        delays = min_duration_ms;
        update_entry_ms();
    }else{
        delays = min_duration_ms;
    }

    AT24CXX_DEBUG("wait for", delays, "ms");
    delay(delays);
}

void AT24CXX::storeBytes(const Address loc, const void * data, const Address len){
    auto full_end = loc + len; 
    CHECK_ADDR(full_end);

    AT24CXX_DEBUG("multi store entry", store_window);
    
    AddressView store_window = AddressView{loc,loc + len};
    AddressView op_window = {0,0};
    do{
        op_window = store_window.grid_forward(op_window, m_pagesize);
        if(op_window){
            wait_for_free();
            const uint8_t * ptr = (reinterpret_cast<const uint8_t *>(data) + (op_window.from - store_window.from));
            WRITE_POOL(op_window.from, ptr, op_window.length());
            update_entry_ms();
        }
    }while(op_window);
}


void AT24CXX::loadBytes(const Address loc, void * data, const Address len){
    auto full_end = loc + len; 
    CHECK_ADDR(full_end);
    READ_POOL(loc, reinterpret_cast<uint8_t *>(data), len);
}