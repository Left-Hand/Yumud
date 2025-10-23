#pragma once

#include "core/stream/ostream.hpp"

namespace ymd{
struct [[nodiscard]] MacAddress final{
public:
    using Self = MacAddress;
    constexpr MacAddress(const std::span<const uint8_t, 6> &pbuf){
        std::copy(pbuf.begin(), pbuf.end(), buf.begin());
    }

    [[nodiscard]] constexpr uint8_t operator [](const size_t index) const{return buf[index];}

    [[nodiscard]] constexpr uint8_t & operator [](const size_t index) {return buf[index];}

    [[nodiscard]] constexpr std::span<const uint8_t, 6> as_bytes() const {return std::span(buf);}
private:
    using DataType = std::array<uint8_t, 6>; // Define a type alias for the underlying buffer
    DataType buf; // Buffer to hold the 6 bytes of the MAC address

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