#pragma once


#include <cstdint>
#include <span>
#include "core/utils/Result.hpp"

namespace ymd::drivers::rd03{

struct Status{
    uint16_t bits;

    constexpr bool is_ok() const {return bits == 0;}
    constexpr bool is_err() const {return bits == 1;}
};


template<typename Receiver>
static constexpr Result<void, typename Receiver::Error> 
push_header(Receiver & receiver) noexcept{
    const uint8_t buf[] = {0xFD, 0xFC, 0xFB, 0xFA};
    return receiver.push_bytes(buf);
};


template<typename Receiver>
static constexpr Result<void, typename Receiver::Error> 
push_tail(Receiver & receiver) noexcept{
    const uint8_t buf[] = {0x04, 0x03, 0x02, 0x01};
    return receiver.push_bytes(buf);
};




}