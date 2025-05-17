#include "ICM45686.hpp"

using namespace ymd;
using ICM45686 = ymd::drivers::ICM45686;
using Error = ICM45686::Error;

// #define ICM45686_DEBUG_EN

#ifdef ICM45686_DEBUG_EN
#define ICM45686_TODO(...) TODO()
#define ICM45686_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define ICM45686_PANIC(...) PANIC{__VA_ARGS__}
#define ICM45686_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define ICM45686_DEBUG(...)
#define ICM45686_TODO(...) PANIC_NSRC()
#define ICM45686_PANIC(...)  PANIC_NSRC()
#define ICM45686_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif



template<typename Fn, typename Fn_Dur>
__inline Result<void, Error> retry(const size_t times, Fn && fn, Fn_Dur && fn_dur){
    if constexpr(!std::is_null_pointer_v<Fn_Dur>) std::forward<Fn_Dur>(fn_dur)();
    Result<void, Error> res = std::forward<Fn>(fn)();
    if(res.is_ok()) return Ok();
    if(!times) return res;
    else return retry(times - 1, std::forward<Fn>(fn), std::forward<Fn_Dur>(fn_dur));
}


template<typename Fn>
__inline Result<void, Error> retry(const size_t times, Fn && fn){
    return retry(times, std::forward<Fn>(fn), nullptr);
}

template<typename Fn>
__inline Result<void, Error> wait(const size_t timeout, Fn && fn){
    return retry(timeout, std::forward<Fn>(fn), [](){clock::delay(1ms);});
}


Result<void, Error> ICM45686::validate(){
    uint8_t Product_ID = 0x00;

    if(const auto res = read_reg(uint8_t(ICM45686::REGISTER::WHO_AM_I), Product_ID); res.is_err()){
        return Err(res.unwrap_err());
    }else{
        if(Product_ID != 0xE9) return Err(Error(Error::WrongWhoAmI));
        return Ok();
    }

    return Ok();
}

// [[nodiscard]] Result<void, Error> set_mode(Mode mode){
//     return 
// }