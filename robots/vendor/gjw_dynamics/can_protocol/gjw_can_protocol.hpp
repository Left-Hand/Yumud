#pragma once

#include "core/math/realmath.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "primitive/can/bxcan_frame.hpp"

namespace ymd::robots::gjw::can_protocol{

struct [[nodiscard]] NodeId final{
    uint8_t bits;
};


// 指令码	功能解释	说明
// 0x01	    单帧读指令	驱动器内存表读指令
// 0x02	    单帧写指令	驱动器内存表写指令
// 0x03	    单圈控制指令	
// 0x04	    多圈控制指令	
// 0x05	    主动上报指令	心跳指令/周期性反馈指令
// 0x06	    单圈同步控制指令	多驱动器同步控制
// 0x07	    多圈同步控制指令	多驱动器同步控制
// 0x08	    同步执行指令	多驱动器同步控制
// 0x09	    版本查询指令	
// 0x0A	    警报/状态指令

enum class [[nodiscard]] FuncCode : uint8_t {
    // 基础读写指令
    ReadSingle          = 0x01,  // 单帧读指令
    WriteSingle         = 0x02,  // 单帧写指令
    LapTurn             = 0x03,  // 单圈控制指令
    MultiTurns          = 0x04,  // 多圈控制指令

    // 主动上报与同步控制指令
    ActiveReport        = 0x05,  // 主动上报指令（心跳/周期性反馈）
    LapTurnSync         = 0x06,  // 单圈同步控制指令
    MultiTurnsSync      = 0x07,  // 多圈同步控制指令
    SyncExecute         = 0x08,  // 同步执行指令（触发同步动作）

    // 工具类指令
    VersionQuery        = 0x09,  // 版本查询指令
    AlertStatus         = 0x0A   // 警报/状态指令（状态上报/清除）
};

struct [[nodiscard]] FrameId final{
    struct [[nodiscard]] Parts final{
        NodeId node_id;
        bool is_response;
        FuncCode func_code;
    };

    constexpr FrameId(const hal::CanStdId stdid) noexcept{
        this->bits_ = stdid.to_bits();
    }

    static constexpr FrameId from_stdid(const hal::CanStdId stdid) noexcept{
        return FrameId(stdid);
    }

    static constexpr FrameId from_parts(const Parts parts) noexcept{ 
        uint16_t bits = 0;
        bits |= std::bit_cast<uint8_t>(parts.node_id) & 0x3f;
        bits |= (parts.is_response << 6);
        bits |= (std::bit_cast<uint8_t>(parts.func_code) << 7);
        return FrameId(hal::CanStdId::from_bits(bits));
    }

    
    constexpr bool is_response() const noexcept{
        return (this->bits_ & (1u << 7)) != 0;
    }

    constexpr bool is_request() const noexcept{
        return !this->is_response();
    }

    constexpr NodeId node_id() const noexcept{
        return NodeId(this->bits_ & 0x3f);
    }

    constexpr FuncCode func_code() const noexcept{
        return FuncCode(this->bits_ >> 7);
    }

    constexpr hal::CanStdId to_stdid() const noexcept{
        return hal::CanStdId::from_bits(this->bits_);
    }

    constexpr Parts to_parts() const noexcept{ 
        return Parts{
            .node_id = this->node_id(), 
            .is_response = this->is_response(),
            .func_code = this->func_code()
        };
    }
private:    
    uint16_t bits_;
    // uint16_t node_id_bits_:6;
    // uint16_t is_resp_not_req_:1;
    // uint16_t command_bits_:5;
};

static_assert(sizeof(FrameId) == sizeof(uint16_t));


}