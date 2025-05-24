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


    
hal::HalResult AT24CXX::write_burst(const Address addr, const std::span<const uint8_t> pdata){
    if (is_small_chip()){
        return i2c_drv_.write_burst(uint8_t(addr.as_u32()), pdata);
    }else{
        return i2c_drv_.write_burst(uint16_t(addr.as_u32()), pdata);
    }
}

hal::HalResult AT24CXX::read_burst(const Address addr, const std::span<uint8_t> pdata){
    if (is_small_chip()){
        return i2c_drv_.read_burst(uint8_t(addr.as_u32()), pdata);
    }else{
        return i2c_drv_.read_burst(uint16_t(addr.as_u32()), pdata);
    }
}


template<typename FnBefore, typename FnExecute, typename FnAfter>
auto internate_grid(
    const uint32_t address, 
    const uint32_t grid,
    const std::span<const uint8_t> pdata,
    const FnBefore&& fn_before, 
    FnExecute&& fn_execute, 
    FnAfter&& fn_after
){
    Range2<uint32_t> store_window = {address,address + grid};
    Range2<uint32_t> op_window = {0,0};
    do{
        op_window = store_window.grid_forward(op_window, grid);
        if(op_window.length() != 0){
            std::forward<FnBefore>(fn_before)();
            const uint8_t * ptr = (pdata.data() + (op_window.from - store_window.from));
            std::forward<FnExecute>(fn_execute)(op_window.from, std::span<const uint8_t>(ptr, op_window.length()));
            std::forward<FnAfter>(fn_after)();
        }
    }while(op_window.length());
}

// void AT24CXX::store_bytes_impl(const Address loc, const std::span<const uint8_t> pdata){
//     // const auto full_end = loc + len; 
//     // CHECK_ADDR(full_end);

//     internate_grid(
//         loc.as_u32(),
//         pagesize_,
//         pdata,
//         [this]{blocking_until_free();},
//         [this](const uint32_t address, const std::span<const uint8_t> pdata){write_burst(Address(address), pdata);},
//         [this]{state_ = Operation::Store;},
//     );
// }


void AT24CXX::load_bytes_impl(const Address loc, const std::span<uint8_t> pdata){
    // auto full_end = loc + ; 
    read_burst(loc, pdata);
}