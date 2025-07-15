#include "core/utils/serde.hpp"

namespace ymd{


namespace robots::joint_cmds{

struct SetPosition{
    bf16 position;
};


struct SetSpeed{
    bf16 speed;
};


struct SetPositionAndSpeed{
    bf16 position;
    bf16 speed;
};


struct SetTrapzoid{
    bf16 position;
    bf16 speed;
    bf16 acceleration;
    bf16 deceleration;
};


struct SetKpKd{
    bf16 kp;
    bf16 kd;
};
}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetPosition)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::joint_cmds::SetPosition)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetPosition)

template<>
struct reflecter::MemberPtrReflecter<robots::joint_cmds::SetPosition> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::joint_cmds::SetPosition::position;
    }();
};


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetSpeed)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::joint_cmds::SetSpeed)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetSpeed)

template<>
struct reflecter::MemberPtrReflecter<robots::joint_cmds::SetSpeed> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::joint_cmds::SetSpeed::speed;
    }();
};

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetPositionAndSpeed)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::joint_cmds::SetPositionAndSpeed)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetPositionAndSpeed)

template<>
struct reflecter::MemberPtrReflecter<robots::joint_cmds::SetPositionAndSpeed> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::joint_cmds::SetPositionAndSpeed::position;
        else if constexpr (N == 1) return &robots::joint_cmds::SetPositionAndSpeed::speed;
    }();
};



DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetTrapzoid)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::joint_cmds::SetTrapzoid)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetTrapzoid)

template<>
struct reflecter::MemberPtrReflecter<robots::joint_cmds::SetTrapzoid> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::joint_cmds::SetTrapzoid::position;
        else if constexpr (N == 1) return &robots::joint_cmds::SetTrapzoid::speed;
        else if constexpr (N == 2) return &robots::joint_cmds::SetTrapzoid::acceleration;
        else if constexpr (N == 3) return &robots::joint_cmds::SetTrapzoid::deceleration;
    }();
};

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetKpKd)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::joint_cmds::SetKpKd)

template<>
struct reflecter::MemberPtrReflecter<robots::joint_cmds::SetKpKd> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::joint_cmds::SetKpKd::kp;
        else if constexpr (N == 1) return &robots::joint_cmds::SetKpKd::kd;
    }();
};


}