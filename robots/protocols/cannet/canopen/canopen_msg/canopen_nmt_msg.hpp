#pragma once

#include "../canopen_primitive/canopen_nmt_primitive.hpp"


enum class AssertLevel{
    NoCheck,
    Trap,
    Abort,
    Panic,
    Propagate
};

#define FLEX_ASSERT_NONE(expr, ...) \
({\
    if constexpr (assert_level != AssertLevel::NoCheck){\
        const bool expr_ = bool(expr);\
        if(expr_ == false) [[unlikely]]{\
            if constexpr (assert_level == AssertLevel::Trap)\
                __builtin_trap();\
            if constexpr (assert_level == AssertLevel::Abort)\
                __builtin_abort();\
            else if constexpr (assert_level == AssertLevel::Panic)\
                PANIC{__VA_ARGS__};\
            else if constexpr (assert_level == AssertLevel::Propagate)\
                return None;\
        }\
    }\
});\

#define FLEX_RETURN_SOME(expr) \
({\
    if constexpr (assert_level == AssertLevel::Propagate)\
        return Some(expr);\
    else return (expr);\
})\

#define FLEX_OPTION(obj_type) std::conditional_t<assert_level == AssertLevel::Propagate , Option<obj_type>, obj_type>


namespace ymd::canopen::nmt_msg{
using namespace canopen::primitive;


struct [[nodiscard]] NetManage{
    NmtCommand cmd;
    NodeId dest_nodeid;

    [[nodiscard]] std::span<const uint8_t, 2> as_le_bytes() const{
        return std::span<const uint8_t, 2>(reinterpret_cast<const uint8_t*>(this), 2);
    }

    [[nodiscard]] std::span<uint8_t, 2> as_mut_le_bytes(){
        return std::span<uint8_t, 2>(reinterpret_cast<uint8_t*>(this), 2);
    }

    constexpr void fill_bytes(const std::span<uint8_t, 2> bytes) const {
        bytes[0] = std::bit_cast<uint8_t>(cmd);
        bytes[1] = dest_nodeid.to_u7();
    }

    static constexpr auto COBID = CobId::from_bits(0x00);
    [[nodiscard]] constexpr CanMsg to_canmsg(){
        std::array<uint8_t, 2> bytes;
        fill_bytes(bytes);
        return CanMsg::from_bytes(COBID.to_stdid(), bytes);
    }
};

struct [[nodiscard]] Sync{
    // 同步功能用于让总线上所有节点同步，主要用于让节点同步 PDO 消息。其 COB-ID 固定为
    // 0x80，数据域为空。
    static constexpr auto COBID = CobId::from_bits(0x80);
    [[nodiscard]] static constexpr CanMsg to_canmsg(){
        return CanMsg::from_empty(COBID.to_stdid());
    }
};

struct [[nodiscard]] BootUp{ 
    using Self = BootUp;
    NodeId station_nodeid;
};


struct [[nodiscard]] Heartbeat{ 

    NodeId station_nodeid;
    NodeState station_state;
};

}

namespace ymd::canopen::msg_serde{
using namespace primitive;

template<typename T>
struct MsgSerde;

template<>
struct MsgSerde<nmt_msg::Heartbeat>{
    using Self = nmt_msg::Heartbeat;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self & self){
        const auto canid = hal::CanStdId(0x700 | self.station_nodeid.to_u7());
        const std::array<uint8_t, 1> bytes = {static_cast<uint8_t>(self.station_state)};
        return CanMsg::from_bytes(canid, std::move(bytes));
    }

    template<AssertLevel assert_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
    -> FLEX_OPTION(Self){
        FLEX_ASSERT_NONE(msg.is_standard())
        FLEX_ASSERT_NONE(msg.length() == 1)

        const auto canid_u32 = msg.id_as_u32();

        FLEX_ASSERT_NONE((canid_u32 & 0b111'1000'0000) == 0x700)
        const auto self = Self{
            .station_nodeid = NodeId::from_u7(canid_u32 & 0x7f),
            .station_state = std::bit_cast<NodeState>(msg[0])
        };
        FLEX_RETURN_SOME(self);
    }
};


template<>
struct MsgSerde<nmt_msg::BootUp>{
    using Self = nmt_msg::BootUp;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self & self){
        const auto canid = hal::CanStdId(0x700 | self.station_nodeid.to_u7());
        const std::array<uint8_t, 1> bytes = {0};
        return CanMsg::from_bytes(canid, std::move(bytes));
    }

    template<AssertLevel assert_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
    -> FLEX_OPTION(Self){
        FLEX_ASSERT_NONE(msg.is_standard())
        FLEX_ASSERT_NONE(msg.length() == 1)

        const auto canid_u32 = msg.id_as_u32();

        FLEX_ASSERT_NONE((canid_u32 & 0b111'1000'0000) == 0x700)
        const auto self = Self{
            .station_nodeid = NodeId::from_u7(canid_u32 & 0x7f),
        };
        FLEX_RETURN_SOME(self);
    }
};

template<typename T>
static constexpr auto to_canmsg(const T& self){
    return MsgSerde<T>::to_canmsg(self);
}

template<typename T, AssertLevel assert_level = AssertLevel::Propagate>
static constexpr auto from_canmsg(const CanMsg & msg){
    return MsgSerde<T>::template from_canmsg<assert_level>(msg);
}

}