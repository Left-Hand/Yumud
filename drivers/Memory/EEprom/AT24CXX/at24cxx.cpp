#include "at24cxx.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

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

    
hal::HalResult AT24CXX::write_pool(const size_t addr, const uint8_t * data, const size_t len){
    AT24CXX_DEBUG("write", len, "bytes to", addr);
    // DEBUGGER.print_arr(data, len);
    if (is_small_chip()){
        return i2c_drv_.write_burst(uint8_t(addr), std::span(data, len));
    }else{
        return i2c_drv_.write_burst((uint16_t)addr, std::span(data, len));
    }
}

hal::HalResult AT24CXX::read_pool(const size_t addr, uint8_t * data, const size_t len){
    AT24CXX_DEBUG("read", len, "bytes to", addr);
    if (is_small_chip()){
        return i2c_drv_.read_burst(uint8_t(addr), std::span(data, len));
    }else{
        return i2c_drv_.read_burst((uint16_t)addr, std::span(data, len));
    }
}


void AT24CXX::wait_for_free(){
    const auto delays = [&]{
        if(last_entry_ms == 0ms){
            update_entry_ms();
            return min_duration_ms;
        }else{
            return min_duration_ms;
        }
    }();

    AT24CXX_DEBUG("wait for", delays);
    clock::delay(Milliseconds(delays));
    // clock::delay(400ms);
}

void AT24CXX::store_bytes(const Address loc, const void * data, const Address len){
    auto full_end = loc + len; 
    CHECK_ADDR(full_end);

    
    AddressView store_window = AddressView{loc,loc + len};
    AddressView op_window = {0,0};

    AT24CXX_DEBUG("multi store entry", store_window);

    do{
        op_window = store_window.grid_forward(op_window, m_pagesize);
        if(op_window.length() != 0){
            wait_for_free();
            const uint8_t * ptr = (reinterpret_cast<const uint8_t *>(data) + (op_window.from - store_window.from));
            write_pool(op_window.from, ptr, op_window.length());
            update_entry_ms();
        }
    }while(op_window.length());
}


void AT24CXX::load_bytes(const Address loc, void * data, const Address len){
    auto full_end = loc + len; 
    CHECK_ADDR(full_end);
    read_pool(loc, reinterpret_cast<uint8_t *>(data), len);
}