#pragma once

#include <cstdint>
#include <span>

namespace ymd::usb{

struct [[nodiscard]] EndpointAddress{
public:
    enum class [[nodiscard]] Kind: uint8_t {
        // EP1_OUT = 0x01,
        EP1_OUT = 0x01,
        EP2_OUT = 0x02,
        EP3_OUT = 0x03,
        EP4_OUT = 0x04,

        EP1_IN = 0x81,
        EP2_IN = 0x82,
        EP3_IN = 0x83,
        EP4_IN = 0x84
    };

    using InvervalMs = uint8_t;

    const uint8_t idx_;
public:
    constexpr EndpointAddress(const uint8_t idx):
        idx_(idx){}
    EndpointAddress(const EndpointAddress & other) = delete;
    EndpointAddress(const EndpointAddress && other) = delete;
    constexpr Kind oaddr() const{return Kind(idx_ & 0x7f);}
    constexpr Kind iaddr() const{return Kind(idx_ | 0x80);}
    constexpr uint8_t idx() const{return idx_;}
};


    // __attribute__((aligned(4))) uint8_t buf_[64 + 64 + 64];

// using EndpointAddress = EndpointAddress::Kind;
// using EndpointInvervalMs = EndpointAddress::InvervalMs;
}