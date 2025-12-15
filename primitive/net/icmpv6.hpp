#pragma once


#include "core/stream/ostream.hpp"

#include <cstdint>

// https://docs.rs/smoltcp/0.12.0/src/smoltcp/wire/icmpv6.rs.html
namespace ymd::net::icmp{


namespace v6{

struct [[nodiscard]] Message{ 
    enum class [[nodiscard]] Kind:uint8_t{
        /// Destination Unreachable.
        DstUnreachable  = 0x01,
        /// Packet Too Big.
        PktTooBig       = 0x02,
        /// Time Exceeded.
        TimeExceeded    = 0x03,
        /// Parameter Problem.
        ParamProblem    = 0x04,
        /// Echo Request
        EchoRequest     = 0x80,
        /// Echo Reply
        EchoReply       = 0x81,
        /// Multicast Listener Query
        MldQuery        = 0x82,
        /// Router Solicitation
        RouterSolicit   = 0x85,
        /// Router Advertisement
        RouterAdvert    = 0x86,
        /// Neighbor Solicitation
        NeighborSolicit = 0x87,
        /// Neighbor Advertisement
        NeighborAdvert  = 0x88,
        /// Redirect
        Redirect        = 0x89,
        /// Multicast Listener Report
        MldReport       = 0x8f,
        /// RPL Control Message
        RplControl      = 0x9b,
    };

    constexpr Message(const Kind kind): kind_(kind){;} 

    [[nodiscard]] constexpr Kind kind() const noexcept { return kind_; }

    /// Per RFC 4443 § 2.1 ICMPv6 message types with the highest order
    /// bit set are informational messages while message types without
    /// the highest order bit set are error messages.
    ///
    /// RFC 4443 § 2.1: https://tools.ietf.org/html/rfc4443#section-2.1
    [[nodiscard]] constexpr bool is_error() const {
        return (std::bit_cast<uint8_t>(kind_) & 0x80) != 0x80;
    }

    /// Return a boolean value indicating if the given message type
    /// is an NDISC message type.
    ///
    /// NDISC: https://tools.ietf.org/html/rfc4861
    [[nodiscard]] constexpr bool is_ndisc() const {
        switch (kind_) {
            case Kind::RouterSolicit:
            case Kind::RouterAdvert:
            case Kind::NeighborSolicit:
            case Kind::NeighborAdvert:
            case Kind::Redirect:
                return true;
            default:
                return false;
        }
    }

    /// Return a boolean value indicating if the given message type
    /// is an MLD message type.
    ///
    /// MLD: https://tools.ietf.org/html/rfc3810
    [[nodiscard]] constexpr bool is_mld() const {
        switch (kind_) {
            case Kind::MldQuery:
            case Kind::MldReport:
                return true;
            default:
                return false;
        }
    }

private:
    Kind kind_;

    friend OutputStream& operator<<(OutputStream& os, const Kind& kind) {
        switch(kind) {
            case Kind::DstUnreachable: return os << "DstUnreachable";
            case Kind::PktTooBig: return os << "PktTooBig";
            case Kind::TimeExceeded: return os << "TimeExceeded";
            case Kind::ParamProblem: return os << "ParamProblem";
            case Kind::EchoRequest: return os << "EchoRequest";
            case Kind::EchoReply: return os << "EchoReply";
            case Kind::MldQuery: return os << "MldQuery";
            case Kind::RouterSolicit: return os << "RouterSolicit";
            case Kind::RouterAdvert: return os << "RouterAdvert";
            case Kind::NeighborSolicit: return os << "NeighborSolicit";
            case Kind::NeighborAdvert: return os << "NeighborAdvert";
            case Kind::Redirect: return os << "Redirect";
            case Kind::MldReport: return os << "MldReport";
            case Kind::RplControl: return os << "RplControl";
        }
        return os << "Unknown" << static_cast<uint8_t>(kind);
    }

    friend OutputStream& operator<<(OutputStream& os, const Message& self) {
        return os << self.kind_;
    }
};

enum class [[nodiscard]] DstUnreachable:uint8_t {
    /// No Route to destination.
    NoRoute         = 0,
    /// Communication with destination administratively prohibited.
    AdminProhibit   = 1,
    /// Beyond scope of source address.
    BeyondScope     = 2,
    /// Address unreachable.
    AddrUnreachable = 3,
    /// Port unreachable.
    PortUnreachable = 4,
    /// Source address failed ingress/egress policy.
    FailedPolicy    = 5,
    /// Reject route to destination.
    RejectRoute     = 6
};

enum class [[nodiscard]] ParamProblem:uint8_t {
    /// Erroneous header field encountered.
    ErroneousHdrField  = 0,
    /// Unrecognized Next Header type encountered.
    UnrecognizedNxtHdr = 1,
    /// Unrecognized IPv6 option encountered.
    UnrecognizedOption = 2
};

inline OutputStream& operator<<(OutputStream& os, const DstUnreachable& self) {
    switch(self) {
        case DstUnreachable::NoRoute: return os << "NoRoute";
        case DstUnreachable::AdminProhibit: return os << "AdminProhibit";
        case DstUnreachable::BeyondScope: return os << "BeyondScope";
        case DstUnreachable::AddrUnreachable: return os << "AddrUnreachable";
        case DstUnreachable::PortUnreachable: return os << "PortUnreachable";
        case DstUnreachable::FailedPolicy: return os << "FailedPolicy";
        case DstUnreachable::RejectRoute: return os << "RejectRoute";
    }
    return os << "Unknown" << static_cast<uint8_t>(self);
}

inline OutputStream& operator<<(OutputStream& os, const ParamProblem& self) {
    switch(self) {
        case ParamProblem::ErroneousHdrField: return os << "ErroneousHdrField";
        case ParamProblem::UnrecognizedNxtHdr: return os << "UnrecognizedNxtHdr";
        case ParamProblem::UnrecognizedOption: return os << "UnrecognizedOption";
    }
    return os << "Unknown" << static_cast<uint8_t>(self);
}

template<typename T>
struct [[nodiscard]] Packet{

};

}

}