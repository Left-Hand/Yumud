#include "at24cxx.hpp"

#ifdef AT24CXX_DEBUG
#undef AT24CXX_DEBUG
#define AT24CXX_DEBUG(...) DEBUG_PRINT(SpecToken::Space, ##__VA_ARGS__, "\t|", __PRETTY_FUNCTION__);
#else
#define AT24CXX_DEBUG(...)
#endif

#define ACCESS_STRICT_PROTECT

#define WRITE_POOL(addr, ptr, len)\
if(is_small_chip())\
    bus_drv.writePool((uint8_t)addr, ptr, len);\
else\
    bus_drv.writePool((uint16_t)addr, ptr, len);\

#define READ_POOL(addr, ptr, len)\
if(is_small_chip())\
    bus_drv.readPool((uint8_t)addr, ptr, len);\
else\
    bus_drv.readPool((uint16_t)addr, ptr, len);\

#define WRITE_REG(loc, data)\
if(is_small_chip()) bus_drv.writeReg((uint8_t)loc, data);\
else bus_drv.writeReg((uint16_t)loc, data);\

#define READ_REG(loc, data)\
if(is_small_chip()) bus_drv.readReg((uint8_t)loc, data);\
else bus_drv.readReg((uint16_t)loc, data);\

#ifdef ACCESS_STRICT_PROTECT
#define CHECK_ADDR(loc)\
ASSERT_WITH_DOWN((loc < m_size), "invalid addr")
#else

#define CHECK_ADDR(loc)\
if(loc >= m_size){\
    AT24CXX_DEBUG("invalid addr");\
    return;\
}

#endif


void AT24CXX::_store(const uint8_t data, const Address loc){
    CHECK_ADDR(loc);
    AT24CXX_DEBUG("store", data, "at", loc);
    WRITE_REG(loc, data);
}
void AT24CXX::_load(uint8_t & data, const Address loc){
    CHECK_ADDR(loc);
    AT24CXX_DEBUG("load", data, "at", loc);
    READ_REG(loc, data);
}

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

void AT24CXX::_store(const void * data, const Address data_size, const Address loc){
    auto full_end = loc + data_size; 
    CHECK_ADDR(full_end);

    AddressWindow store_window = AddressWindow{loc,loc + data_size};
    AddressWindow op_window = {0,0};
    AT24CXX_DEBUG("multi store entry", store_window);
    do{
        op_window = store_window.grid_forward(op_window, m_pagesize);
        if(op_window){
            uint8_t * ptr = ((uint8_t *)data + (op_window.start - store_window.start));
            wait_for_free();
            WRITE_POOL(op_window.start, ptr, op_window.length());
            update_entry_ms();
        }
    }while(op_window);
}


void AT24CXX::_load(void * data, const Address data_size, const Address loc){
    auto full_end = loc + data_size; 
    CHECK_ADDR(full_end);
    READ_POOL(loc, (uint8_t *)data, data_size);
}