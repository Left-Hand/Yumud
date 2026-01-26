#pragma once

#include "can_enum.hpp"

namespace ymd::hal::can{



/// @brief CAN发送事件
struct alignas(4) TransmitEvent{
    enum class [[nodiscard]] Kind:uint8_t{
        Failed,
        Success
    };

    using Self = TransmitEvent;
    using enum Kind;

    Kind kind;
    MailboxIndex mbox_idx;
    uint16_t padding = 0;
};

static_assert(sizeof(TransmitEvent) == 4);

/// @brief CAN接收事件
struct alignas(4) [[nodiscard]] ReceiveEvent{
    enum class [[nodiscard]] Kind:uint8_t{
        Pending,
        Full,
        Overrun,
    };

    using Self = ReceiveEvent;
    using enum Kind;

    Kind kind;
    FifoIndex fifo_idx;
    uint16_t padding = 0;
};

static_assert(sizeof(ReceiveEvent) == 4);

struct alignas(4) [[nodiscard]] StatusFlag final{
    using Self = StatusFlag;

    uint32_t wakeup:1 = 0;
    uint32_t sleep_acknowledge:1 = 0;
    uint32_t error_warning:1 = 0;
    uint32_t error_passive:1 = 0;
    uint32_t bus_off:1 = 0;
    uint32_t last_error_code:1 = 0;
    uint32_t error:1 = 0;
    uint32_t __resv__:1+16 = 0;

    static consteval Self zero(){
        return Self{};
    } 
};

static_assert(sizeof(StatusFlag) == 4);

struct alignas(4) [[nodiscard]] Event final{
    using Self = Event;
    enum class [[nodiscard]] Type:uint32_t{
        Transmit,
        Receive,
        Status
    };

    struct ErasuredArg{
        // void * mut_void_ptr = nullptr;
        // const void * void_ptr = nullptr;
        // uint8_t u32_digit = 0;
        // uint32_t u32_digit = 0;
        uint32_t u32_digit = 0;
    };


    alignas(4) Type type;
    alignas(4) ErasuredArg arg;

    template<typename E>
    __attribute__((always_inline))
    constexpr auto exact_arg() const {
        if constexpr(std::is_same_v<E, TransmitEvent>){
            return std::bit_cast<TransmitEvent>(arg.u32_digit);
        }else if constexpr (std::is_same_v<E, ReceiveEvent>){
            return std::bit_cast<ReceiveEvent>(arg.u32_digit);
        }else if constexpr (std::is_same_v<E, StatusFlag>){
            return std::bit_cast<StatusFlag>(arg.u32_digit);
        }
        __builtin_unreachable();
    }

    template<typename E>
    __attribute__((always_inline))
    static constexpr Self from(const E ev){
        if constexpr(std::is_same_v<E, TransmitEvent>){
            return Self{Type::Transmit, ErasuredArg{.u32_digit = std::bit_cast<uint32_t>(ev)}};
        }else if constexpr (std::is_same_v<E, ReceiveEvent>){
            return Self{Type::Receive, ErasuredArg{.u32_digit = std::bit_cast<uint32_t>(ev)}};
            // return Self{Type::Receive, ErasuredArg{.u32_digit = ev.to_u16()}};
        }else if constexpr (std::is_same_v<E, StatusFlag>){
            return Self{Type::Status, ErasuredArg{.u32_digit = std::bit_cast<uint32_t>(ev)}};
        }
        __builtin_unreachable();
    }
};

}

namespace ymd::hal{

using CanEvent = can::Event;
using CanTransmitEvent = can::TransmitEvent;
using CanReceiveEvent = can::ReceiveEvent;
using CanStatusFlag = can::StatusFlag;
}