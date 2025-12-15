#pragma once


#include "core/stream/ostream.hpp"

#include <cstdint>

// https://docs.rs/smoltcp/0.12.0/src/smoltcp/wire/icmpv6.rs.html
namespace ymd::net::icmp{


namespace v4{
enum class [[nodiscard]] Message:uint8_t{
    /// Echo reply
    EchoReply      =  0,
    /// Destination unreachable
    DstUnreachable =  3,
    /// Message redirect
    Redirect       =  5,
    /// Echo request
    EchoRequest    =  8,
    /// Router advertisement
    RouterAdvert   =  9,
    /// Router solicitation
    RouterSolicit  = 10,
    /// Time exceeded
    TimeExceeded   = 11,
    /// Parameter problem
    ParamProblem   = 12,
    /// Timestamp
    Timestamp      = 13,
    /// Timestamp reply
    TimestampReply = 14
};

inline OutputStream& operator<<(OutputStream& os, const Message & self)
{
    switch (self)
    {
        case Message::EchoReply: return os << "EchoReply";
        case Message::EchoRequest: return os << "EchoRequest";
        case Message::DstUnreachable: return os << "DstUnreachable";
        case Message::Redirect: return os << "Redirect";
        case Message::RouterAdvert: return os << "RouterAdvert";
        case Message::RouterSolicit: return os << "RouterSolicit";
        case Message::TimeExceeded: return os << "TimeExceeded";
        case Message::ParamProblem: return os << "ParamProblem";
        case Message::Timestamp: return os << "Timestamp";
        case Message::TimestampReply: return os << "TimestampReply";
    }
    return os << "Unknown" << std::bit_cast<uint8_t>(self);
}

enum class [[nodiscard]] DstUnreachable:uint8_t{
    /// Destination network unreachable
    NetUnreachable   =  0,
    /// Destination host unreachable
    HostUnreachable  =  1,
    /// Destination protocol unreachable
    ProtoUnreachable =  2,
    /// Destination port unreachable
    PortUnreachable  =  3,
    /// Fragmentation required, and DF flag set
    FragRequired     =  4,
    /// Source route failed
    SrcRouteFailed   =  5,
    /// Destination network unknown
    DstNetUnknown    =  6,
    /// Destination host unknown
    DstHostUnknown   =  7,
    /// Source host isolated
    SrcHostIsolated  =  8,
    /// Network administratively prohibited
    NetProhibited    =  9,
    /// Host administratively prohibited
    HostProhibited   = 10,
    /// Network unreachable for ToS
    NetUnreachToS    = 11,
    /// Host unreachable for ToS
    HostUnreachToS   = 12,
    /// Communication administratively prohibited
    CommProhibited   = 13,
    /// Host precedence violation
    HostPrecedViol   = 14,
    /// Precedence cutoff in effect
    PrecedCutoff     = 15
};

inline OutputStream & operator<<(OutputStream & os, const DstUnreachable & self){
    switch(self){
        case DstUnreachable::ProtoUnreachable: return os << "ProtoUnreachable";
        case DstUnreachable:: DstNetUnknown: return os << "DstNetUnknown";
        case DstUnreachable::DstHostUnknown: return os << "DstHostUnknown";
        case DstUnreachable::NetUnreachable: return os << "NetUnreachable";
        case DstUnreachable::NetUnreachToS: return os << "NetUnreachToS";
        case DstUnreachable::HostUnreachable: return os << "HostUnreachable";
        case DstUnreachable::HostUnreachToS: return os << "HostUnreachToS";
        case DstUnreachable::PortUnreachable: return os << "PortUnreachable";
        case DstUnreachable::FragRequired: return os << "FragRequired";
        case DstUnreachable::SrcRouteFailed: return os << "SrcRouteFailed";
        case DstUnreachable::NetProhibited: return os << "NetProhibited";
        case DstUnreachable::HostProhibited: return os << "HostProhibited";
        case DstUnreachable::SrcHostIsolated: return os << "SrcHostIsolated";
        case DstUnreachable::CommProhibited: return os << "CommProhibited";
        case DstUnreachable::HostPrecedViol: return os << "HostPrecedViol";
        case DstUnreachable::PrecedCutoff: return os << "PrecedCutoff";
    }
    return os << "Unknown" << std::bit_cast<uint8_t>(self);
}

enum class [[nodiscard]] Redirect:uint8_t{
    /// Redirect Datagram for the Network
    Net     = 0,
    /// Redirect Datagram for the Host
    Host    = 1,
    /// Redirect Datagram for the ToS & network
    NetToS  = 2,
    /// Redirect Datagram for the ToS & host
    HostToS = 3
};

inline OutputStream& operator<<(OutputStream& os, const Redirect& self)
{
    switch (self)
    {
        case Redirect::Net: return os << "Net";
        case Redirect::Host: return os << "Host";
        case Redirect::NetToS: return os << "NetToS";
        case Redirect::HostToS: return os << "HostToS";
    }
    return os << "Unknown" << std::bit_cast<uint8_t>(self);
}


enum class [[nodiscard]] ParamProblem:uint8_t{
    /// Pointer indicates the error
    AtPointer     = 0,
    /// Missing a required option
    MissingOption = 1,
    /// Bad length
    BadLength     = 2
};



inline OutputStream& operator<<(OutputStream& os, const ParamProblem& self)
{
    switch (self)
    {
        case ParamProblem::AtPointer: return os << "AtPointer";
        case ParamProblem::MissingOption: return os << "MissingOption";
        case ParamProblem::BadLength: return os << "BadLength";
    }
    return os << "Unknown" << std::bit_cast<uint8_t>(self);
}
}

}