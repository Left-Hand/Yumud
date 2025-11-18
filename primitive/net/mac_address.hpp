#pragma once

#include "core/stream/ostream.hpp"

namespace ymd{
struct [[nodiscard]] MacAddress final{
public:
    using Self = MacAddress;
    constexpr MacAddress(const std::span<const uint8_t, 6> bytes){
        std::copy(bytes.begin(), bytes.end(), bytes_.begin());
    }

    [[nodiscard]] constexpr uint8_t operator [](const size_t index) const{return bytes_[index];}

    [[nodiscard]] constexpr uint8_t & operator [](const size_t index) {return bytes_[index];}

    [[nodiscard]] constexpr std::span<const uint8_t, 6> as_bytes() const {
        return std::span(bytes_);}
private:
    using Storage = std::array<uint8_t, 6>; // Define a type alias for the underlying buffer
    Storage bytes_; // Buffer to hold the 6 bytes of the MAC address

    friend OutputStream & operator <<(OutputStream & os, const Self self){
        return os 
            << self[0] << os.brackets<':'>()
            << self[1] << os.brackets<':'>()
            << self[2] << os.brackets<':'>()
            << self[3] << os.brackets<':'>()
            << self[4] << os.brackets<':'>()
            << self[5];
    }
};
}