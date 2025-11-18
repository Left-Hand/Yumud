#pragma once

#include <cstdint>

namespace ymd{
    class OutputStream;
};

namespace ymd::hal{
class Gpio;

enum class [[nodiscard]] UartParity{
    None    = 0x0000,
    Even    = 0x0400,
    Odd     = 0x0600
};

struct [[nodiscard]] UartEvent{
    enum class [[nodiscard]] Kind:uint8_t{
        TxIdle,
        TxOverrun,
        RxIdle,
        RxOverrun,
        RxFull
    };

    using enum Kind;

    constexpr UartEvent(Kind kind):kind_(kind){}
    constexpr UartEvent(const UartEvent & ) = default;
    constexpr UartEvent(UartEvent && ) = default;
    [[nodiscard]] constexpr bool is_tx() const {
        return kind_ == TxIdle || kind_ == TxOverrun;
    }

    [[nodiscard]] constexpr bool is_rx() const {
        return not is_tx();
    }

    [[nodiscard]] constexpr Kind kind() const {
        return kind_;
    }

    [[nodiscard]] constexpr bool operator ==(const Kind kind){
        return kind_ == kind;
    }
private:
    Kind kind_;

    friend OutputStream & operator << (OutputStream & os, const UartEvent & event);
};
}