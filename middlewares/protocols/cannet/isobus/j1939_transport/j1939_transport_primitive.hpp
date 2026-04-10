#pragma once

#include <cstdint>
#include "core/utils/sumtype.hpp"
#include "core/utils/Result.hpp"

namespace ymd::j1939::transport{

struct [[nodiscard]] ConnectionManagement final{
    using Self = ConnectionManagement;
    enum class Kind:uint8_t{
        RequestToSend = 0x10,
        ClearToSend = 0x11,
        EndOfMessageAcknowledgment = 0x13,
        BroadcastAnnounceMessage = 0x20,
        Abort = 0xff,
    };

    constexpr ConnectionManagement(Kind kind) : kind_(kind) {}

    [[nodiscard]] constexpr auto kind() const{return kind_;}

    [[nodiscard]] constexpr bool operator ==(const Kind rhs){
        return kind_ == rhs;
    }

    static constexpr Option<Self> try_from(const uint8_t b){
        switch(std::bit_cast<Kind>(b)){
            case Kind::RequestToSend:
            case Kind::ClearToSend:
            case Kind::EndOfMessageAcknowledgment:
            case Kind::BroadcastAnnounceMessage:
            case Kind::Abort:
                return Some(Self{std::bit_cast<Kind>(b)});
        }
        return None;
    }
private:
    Kind kind_;

    DEF_FRIEND_DERIVE_DEBUG(Kind)

    friend OutputStream& operator <<(OutputStream& os, const Self & self){
        return os << self.kind_;
    }
};


}