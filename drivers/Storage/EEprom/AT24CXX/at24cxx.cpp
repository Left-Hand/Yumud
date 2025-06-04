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

using Error = AT24CXX::Error;
template<typename T = void>
using IResult = Result<T, Error>;

IResult<> AT24CXX::write_burst(const Address addr, const std::span<const uint8_t> pbuf){
    if (is_small_chip()){
        if(const auto res = i2c_drv_.write_burst(uint8_t(addr.as_u32()), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else{
        if(const auto res = i2c_drv_.write_burst(uint16_t(addr.as_u32()), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
}

IResult<> AT24CXX::read_burst(const Address addr, const std::span<uint8_t> pbuf){
    if (is_small_chip()){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr.as_u32()), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else{
        if(const auto res = i2c_drv_.read_burst(uint16_t(addr.as_u32()), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
}


template<typename FnBefore, typename FnExecute, typename FnAfter>
auto internate_grid(
    const uint32_t address, 
    const uint32_t grid,
    const std::span<const uint8_t> pbuf,
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
            const uint8_t * ptr = (pbuf.data() + (op_window.start - store_window.start));
            std::forward<FnExecute>(fn_execute)(op_window.start, std::span<const uint8_t>(ptr, op_window.length()));
            std::forward<FnAfter>(fn_after)();
        }
    }while(op_window.length());
}


// static constexpr Option<Range2u> map_grid_to_next(const Range2u range, const uint gsize){
//     const bool left_is_aligned = range.start % gsize == 0;
// }

// void AT24CXX::store_bytes_impl(const Address loc, const std::span<const uint8_t> pbuf){
//     // const auto full_end = loc + len; 
//     // CHECK_ADDR(full_end);

//     internate_grid(
//         loc.as_u32(),
//         pagesize_,
//         pbuf,
//         [this]{blocking_until_free();},
//         [this](const uint32_t address, const std::span<const uint8_t> pbuf){write_burst(Address(address), pbuf);},
//         [this]{state_ = Operation::Store;},
//     );
// }


IResult<> AT24CXX::load_bytes_impl(const Address loc, const std::span<uint8_t> pbuf){

    if(const auto res = read_burst(loc, pbuf);
        res.is_err()) return res;

    TODO();
    return Ok();
}

IResult<bool> AT24CXX::is_busy(){return Ok(true);}
// IResult<bool> is_available(){return Ok(false);}

IResult<> AT24CXX::init(){
    if(const auto res = validate();
        res.is_err()) return res;
    return Ok();
};

IResult<> AT24CXX::validate(){
    return Ok();
};
