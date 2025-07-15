#pragma once

namespace ymd{
namespace robots::machine_cmds{

struct SetPositionXy{
    bf16 x;
    bf16 y;
};

struct SetPositionXyz{
    bf16 x;
    bf16 y;
    bf16 z;
};


struct SetPolar{
    bf16 radius;
    bf16 phi;
};


struct Replace{
    bf16 x1, y1;
    bf16 x2, y2;
};


struct Spin{
    bf16 delta_rotation;
};

struct Rotate{
    bf16 rotation;
};

struct Abort{
    bf16 z;
};
}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_cmds::SetPositionXy)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_cmds::SetPositionXy)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_cmds::SetPositionXy)

template<>
struct reflecter::MemberPtrReflecter<robots::machine_cmds::SetPositionXy> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::machine_cmds::SetPositionXy::x;
        else if constexpr (N == 1) return &robots::machine_cmds::SetPositionXy::y;
    }();
};

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_cmds::SetPositionXyz)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_cmds::SetPositionXyz)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_cmds::SetPositionXyz)

template<>
struct reflecter::MemberPtrReflecter<robots::machine_cmds::SetPositionXyz> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::machine_cmds::SetPositionXyz::x;
        else if constexpr (N == 1) return &robots::machine_cmds::SetPositionXyz::y;
        else if constexpr (N == 2) return &robots::machine_cmds::SetPositionXyz::z;
    }();
};

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_cmds::Replace)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_cmds::Replace)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_cmds::Replace)

template<>
struct reflecter::MemberPtrReflecter<robots::machine_cmds::Replace> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::machine_cmds::Replace::x1;
        else if constexpr (N == 1) return &robots::machine_cmds::Replace::y1;
        else if constexpr (N == 2) return &robots::machine_cmds::Replace::x2;
        else if constexpr (N == 3) return &robots::machine_cmds::Replace::y2;
    }();
};


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_cmds::Spin)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_cmds::Spin)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_cmds::Spin)

template<>
struct reflecter::MemberPtrReflecter<robots::machine_cmds::Spin> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::machine_cmds::Spin::delta_rotation;
    }();
};




DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_cmds::Rotate)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_cmds::Rotate)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_cmds::Rotate)

template<>
struct reflecter::MemberPtrReflecter<robots::machine_cmds::Rotate> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &robots::machine_cmds::Rotate::rotation;
    }();
};


}