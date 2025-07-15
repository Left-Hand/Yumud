#pragma once

#include "core/utils/serde.hpp"
#include "command.hpp"

namespace ymd{

using namespace robots::joint_cmds;

DEF_DERIVE_SERIALIZE_AS_TUPLE(SetPosition)
DEF_DERIVE_DEBUG_AS_DISPLAY(SetPosition)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(SetPosition)

template<>
struct reflecter::MemberPtrReflecter<SetPosition> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &SetPosition::position;
    }();
};


DEF_DERIVE_SERIALIZE_AS_TUPLE(SetSpeed)
DEF_DERIVE_DEBUG_AS_DISPLAY(SetSpeed)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(SetSpeed)

template<>
struct reflecter::MemberPtrReflecter<SetSpeed> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &SetSpeed::speed;
    }();
};

DEF_DERIVE_SERIALIZE_AS_TUPLE(SetPositionAndSpeed)
DEF_DERIVE_DEBUG_AS_DISPLAY(SetPositionAndSpeed)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(SetPositionAndSpeed)

template<>
struct reflecter::MemberPtrReflecter<SetPositionAndSpeed> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &SetPositionAndSpeed::position;
        else if constexpr (N == 1) return &SetPositionAndSpeed::speed;
    }();
};



DEF_DERIVE_SERIALIZE_AS_TUPLE(SetTrapzoid)
DEF_DERIVE_DEBUG_AS_DISPLAY(SetTrapzoid)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(SetTrapzoid)

template<>
struct reflecter::MemberPtrReflecter<SetTrapzoid> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &SetTrapzoid::position;
        else if constexpr (N == 1) return &SetTrapzoid::speed;
        else if constexpr (N == 2) return &SetTrapzoid::acceleration;
        else if constexpr (N == 3) return &SetTrapzoid::deceleration;
    }();
};


using namespace robots::machine_cmds;

DEF_DERIVE_SERIALIZE_AS_TUPLE(Spin)
DEF_DERIVE_DEBUG_AS_DISPLAY(Spin)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(Spin)


DEF_DERIVE_SERIALIZE_AS_TUPLE(SetPositionXYZ)
DEF_DERIVE_DEBUG_AS_DISPLAY(SetPositionXYZ)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(SetPositionXYZ)

DEF_DERIVE_SERIALIZE_AS_TUPLE(Replace)
DEF_DERIVE_DEBUG_AS_DISPLAY(Replace)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(Replace)

template<>
struct reflecter::MemberPtrReflecter<SetPositionXYZ> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &SetPositionXYZ::x;
        else if constexpr (N == 1) return &SetPositionXYZ::y;
        else if constexpr (N == 2) return &SetPositionXYZ::z;
    }();
};


DEF_DERIVE_SERIALIZE_AS_TUPLE(NestU8)
DEF_DERIVE_DEBUG_AS_DISPLAY(NestU8)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(NestU8)


DEF_DERIVE_SERIALIZE_AS_TUPLE(SetKpKd)
DEF_DERIVE_DEBUG_AS_DISPLAY(SetKpKd)


template<>
struct reflecter::MemberPtrReflecter<Spin> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &Spin::delta_rotation;
    }();
};


template<>
struct reflecter::MemberPtrReflecter<NestU8> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &NestU8::buf;
    }();
};


template<>
struct reflecter::MemberPtrReflecter<Replace> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &Replace::x1;
        else if constexpr (N == 1) return &Replace::y1;
        else if constexpr (N == 2) return &Replace::x2;
        else if constexpr (N == 3) return &Replace::y2;
    }();
};

}