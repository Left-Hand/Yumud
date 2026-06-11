#pragma once

#include "gjw_can_protocol.hpp"
#include "core/container/heapless_vector.hpp"

namespace ymd::robots::gjw::can_protocol{

template<typename Receiver>
static constexpr Result<void, typename Receiver::Error> serialize_be_u16(
    Receiver & receiver, 
    const uint16_t arg
) {
    const std::array<uint8_t, 2> buffer = {
        static_cast<uint8_t>(arg >> 8),
        static_cast<uint8_t>(arg & 0xFF)
    };

    if(const auto res = receiver.push_bytes(buffer); 
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

template<typename Receiver>
constexpr Result<void, typename Receiver::Error> serialize_be_u16x4(
    Receiver & receiver, 
    const uint16_t arg0,
    const uint16_t arg1,
    const uint16_t arg2,
    const uint16_t arg3
) {

    if (const auto res = serialize_be_u16(receiver, arg0);
        res.is_err()) return Err(res.unwrap_err());

    if (const auto res = serialize_be_u16(receiver, arg1);
        res.is_err()) return Err(res.unwrap_err());

    if (const auto res = serialize_be_u16(receiver, arg2);
        res.is_err()) return Err(res.unwrap_err());

    if (const auto res = serialize_be_u16(receiver, arg3);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

namespace req_msgs{



// REQ[0x01] 单帧读指令
// 使用单帧读指令对驱动器内存表寄存器进行读查询，一次最多能读3个寄存器(6字节数据)。
struct [[nodiscard]] ReadSingle final{
    static constexpr FuncCode FUNC_CODE = FuncCode::ReadSingle;
    static constexpr size_t CONSTANT_LENGTH = 3;

    uint16_t reg_addr;
    uint8_t quantity;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver & receiver) const noexcept {
        auto & self = *this;

        {
            const std::array<uint8_t, 3> buffer = {
                static_cast<uint8_t>(self.reg_addr >> 8),
                static_cast<uint8_t>(self.reg_addr & 0xFF),
                static_cast<uint8_t>(quantity),
            };

            if(const auto res = receiver.push_bytes(buffer); 
                res.is_err()) return Err(res.unwrap_err());
        }

        return Ok();
    }
};


// REQ[0x03] 单圈控制指令
struct [[nodiscard]] LapTurn final {
    static constexpr FuncCode FUNC_CODE = FuncCode::LapTurn;
    static constexpr size_t CONSTANT_LENGTH = 8;

    uint16_t target_pos;
    uint16_t acceleration;
    uint16_t speed;
    uint16_t current;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver& receiver) const noexcept {
        return serialize_be_u16x4(receiver, target_pos, acceleration, speed, current);
    }
};

// REQ[0x04] 多圈控制指令
struct [[nodiscard]] MultiTurns final {
    static constexpr FuncCode FUNC_CODE = FuncCode::MultiTurns;
    static constexpr size_t CONSTANT_LENGTH = 8;

    uint32_t target_pos;
    uint16_t acceleration;
    uint16_t speed;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver& receiver) const noexcept {
        return serialize_be_u16x4(
            receiver,
            static_cast<uint16_t>(target_pos >> 16),
            static_cast<uint16_t>(target_pos & 0xFFFF),
            acceleration,
            speed
        );
    }
};

// REQ[0x05] 主动上报指令
// 没有对应的请求 只有响应

// REQ[0x06] 单圈同步控制指令
struct [[nodiscard]] LapTurnSync final {
    static constexpr FuncCode FUNC_CODE = FuncCode::LapTurnSync;
    static constexpr size_t CONSTANT_LENGTH = 8;

    uint16_t target_pos;
    uint16_t acceleration;
    uint16_t speed;
    uint16_t current;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver& receiver) const noexcept {
        return serialize_be_u16x4(receiver, target_pos, acceleration, speed, current);
    }
};

// REQ[0x07] 多圈同步控制指令
struct [[nodiscard]] MultiTurnsSync final {
    static constexpr FuncCode FUNC_CODE = FuncCode::MultiTurnsSync;
    static constexpr size_t CONSTANT_LENGTH = 8;

    uint32_t target_pos;
    uint16_t acceleration;
    uint16_t speed;

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error> serialize_context(Receiver& receiver) const noexcept {
        return serialize_be_u16x4(
            receiver,
            static_cast<uint16_t>(target_pos >> 16),
            static_cast<uint16_t>(target_pos & 0xFFFF),
            acceleration,
            speed
        );
    }
};

// REQ[0x08] 同步执行指令
struct [[nodiscard]] SyncExecute final {
    static constexpr FuncCode FUNC_CODE = FuncCode::SyncExecute;
    static constexpr size_t CONSTANT_LENGTH = 0;

    //没有有效的载荷 不需要序列化方法
};

// REQ[0x09] 版本查询指令
struct [[nodiscard]] VersionQuery final {
    static constexpr FuncCode FUNC_CODE = FuncCode::VersionQuery;
    static constexpr size_t CONSTANT_LENGTH = 0;

    //没有有效的载荷 不需要序列化方法
};

// REQ[0x0A] 警报/状态查询指令
// 没有对应的请求 只有响应
}

namespace resp_msgs{
// REQ[0x01] 单帧读响应
// 响应单帧读指令，返回查询的寄存器值。
struct [[nodiscard]] ReadSingle final{

};

}

}