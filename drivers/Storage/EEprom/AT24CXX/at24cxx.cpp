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
    // DEBUG_PRINTLN(addr, pbuf);
    // return Ok();
    if (is_small_chip()){
        if(const auto res = i2c_drv_.write_burst(uint8_t(addr.to_u32()), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else{
        if(const auto res = i2c_drv_.write_burst(uint16_t(addr.to_u32()), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
}

IResult<> AT24CXX::read_burst(const Address addr, const std::span<uint8_t> pbuf){
    // DEBUG_PRINTLN(addr, pbuf);
    // return Ok();
    if (is_small_chip()){
        if(const auto res = i2c_drv_.read_burst(uint8_t(addr.to_u32()), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }else{
        if(const auto res = i2c_drv_.read_burst(uint16_t(addr.to_u32()), pbuf);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    }
}


#if 0
namespace static_test{
    constexpr void test1(){
        constexpr auto it = RangeGridIter({0, 13}, 8);
        constexpr auto _0 = it.begin();
        constexpr auto _1 = it.next(_0).unwrap();
        static_assert(_1 == Range2u{8, 13});
        // constexpr auto _2 = it.next(_1).unwrap();

        constexpr auto e_0 = it.end();  
        constexpr auto e_1 = it.prev(e_0).unwrap();  
        static_assert(e_1 == Range2u{0, 8});
    }

    constexpr void test2(){
        constexpr auto it = RangeGridIter({2, 9}, 8);
        constexpr auto _0 = it.begin();
        constexpr auto _1 = it.next(_0).unwrap();
        static_assert(_1 == Range2u{8, 9});

        constexpr auto e_0 = it.end();  
        constexpr auto e_1 = it.prev(e_0).unwrap();  
        static_assert(e_1 == Range2u{2, 8});
    }

    constexpr void test3(){
        constexpr auto it = RangeGridIter({2, 3}, 8);
        constexpr auto _0 = it.begin();
        static_assert(it.next(_0).is_none());

        constexpr auto e_0 = it.end();  
        static_assert(it.prev(e_0).is_none());  
    }
    constexpr void test4(){
        constexpr auto it = RangeGridIter({257, 258}, 8);
        constexpr auto _0 = it.begin();
        static_assert(it.next(_0).is_none());

        constexpr auto e_0 = it.end();  
        static_assert(it.prev(e_0).is_none());  
    }
}
#endif


IResult<> AT24CXX::store_bytes_inblock_impl(const Address loc, const std::span<const uint8_t> pbuf){
    return write_burst(loc, pbuf);
}
IResult<> AT24CXX::load_bytes_inblock_impl(const Address loc, const std::span<uint8_t> pbuf){
    return read_burst(loc, pbuf);
}

IResult<> AT24CXX::init(){
    if(const auto res = validate();
        res.is_err()) return res;
    return Ok();
};

IResult<> AT24CXX::validate(){
    return Ok();
};
