#pragma once

#include "../LT8960L.hpp"
#include "core/debug/debug.hpp"
#include "core/buffer/ringbuf/Fifo_t.hpp"

#include "hal/gpio/gpio.hpp"


#define LT8960L_DEBUG_EN

#ifdef LT8960L_DEBUG_EN
#define LT8960L_TODO(...) TODO()
#define LT8960L_DEBUG(...) DEBUG_PRINTS(__VA_ARGS__);
#define LT8960L_PANIC(...) PANIC{__VA_ARGS__}
#define LT8960L_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define LT8960L_DEBUG(...)
#define LT8960L_TODO(...) PANIC_NSRC()
#define LT8960L_PANIC(...)  PANIC_NSRC()
#define LT8960L_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

namespace ymd::drivers{

using Error = drivers::LT8960L::Error;

// scexpr size_t packet_len = 64;
static constexpr size_t LT8960L_PACKET_SIZE = 12;
static constexpr size_t LT8960L_FIFO_SIZE = 16;

class Tx{
    Fifo_t<uint8_t, LT8960L_FIFO_SIZE> fifo_;

    size_t write(std::span<const uint8_t> pbuf){
        fifo_.push(pbuf);
        return pbuf.size();
    }

    size_t pending() const {
        return fifo_.available();
    }
};


class Rx{
    Fifo_t<uint8_t, LT8960L_FIFO_SIZE> fifo_;

    size_t read(std::span<uint8_t> pbuf){
        fifo_.pop(pbuf);
        return pbuf.size();
    }

    size_t awailable() const {
        return fifo_.available();
    }
};


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

}