#pragma once

#include "core/utils/serde.hpp"
#include "command.hpp"

namespace ymd{
using namespace robots;

DEF_DERIVE_SERIALIZE_AS_TUPLE(SetPositionCommand)
DEF_DERIVE_DEBUG_AS_DISPLAY(SetPositionCommand)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(SetPositionCommand)


template<>
struct reflecter::MemberPtrReflecter<SetPositionCommand> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &SetPositionCommand::position;
        else if constexpr (N == 1) return &SetPositionCommand::speed;
    }();
};

DEF_DERIVE_SERIALIZE_AS_TUPLE(SpinCommand)
DEF_DERIVE_DEBUG_AS_DISPLAY(SpinCommand)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(SpinCommand)


DEF_DERIVE_SERIALIZE_AS_TUPLE(SetPositionXYZCommand)
DEF_DERIVE_DEBUG_AS_DISPLAY(SetPositionXYZCommand)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(SetPositionXYZCommand)

DEF_DERIVE_SERIALIZE_AS_TUPLE(ReplaceCommand)
DEF_DERIVE_DEBUG_AS_DISPLAY(ReplaceCommand)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(ReplaceCommand)

template<>
struct reflecter::MemberPtrReflecter<SetPositionXYZCommand> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &SetPositionXYZCommand::x;
        else if constexpr (N == 1) return &SetPositionXYZCommand::y;
        else if constexpr (N == 2) return &SetPositionXYZCommand::z;
    }();
};


DEF_DERIVE_SERIALIZE_AS_TUPLE(NestU8Command)
DEF_DERIVE_DEBUG_AS_DISPLAY(NestU8Command)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(NestU8Command)


DEF_DERIVE_SERIALIZE_AS_TUPLE(SetKpKdCommand)
DEF_DERIVE_DEBUG_AS_DISPLAY(SetKpKdCommand)


template<>
struct reflecter::MemberPtrReflecter<SpinCommand> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &SpinCommand::delta_rotation;
    }();
};


template<>
struct reflecter::MemberPtrReflecter<NestU8Command> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &NestU8Command::buf;
    }();
};


template<>
struct reflecter::MemberPtrReflecter<ReplaceCommand> {
    
    template<size_t N>
    static constexpr auto member_ptr_v = [] {
        if constexpr (N == 0) return &ReplaceCommand::x1;
        else if constexpr (N == 1) return &ReplaceCommand::y1;
        else if constexpr (N == 2) return &ReplaceCommand::x2;
        else if constexpr (N == 3) return &ReplaceCommand::y2;
    }();
};

}