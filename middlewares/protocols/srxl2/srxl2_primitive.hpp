#pragma once

#include <cstdint>
#include <bit>

// https://github.com/SpektrumRC/SRXL2/blob/master/Docs/SRXL2%20Specification.pdf

namespace ymd::srxl2{


enum class DeviceType:uint8_t{
    None = 0x00,
    RemoteReceiver = 0x1,
    Receiver = 0x2,
};


struct DeviceId{
    uint8_t index : 4;
    DeviceType type : 4;

    constexpr uint8_t to_u8() const {
        return std::bit_cast<uint8_t>(*this);
    }

    constexpr bool is_boardcast() const{
        return to_u8() == 0xff;
    }

    constexpr bool need_ack() const{
        return to_u8() != 0;
    }
};


static_assert(sizeof(DeviceId) == 1);


enum class PacketId:uint8_t{
    HandShake = 0x21,
    BindInfo = 0x41,
    ParameterConfig = 0x50,
    SignalQuality = 0x55,
    TelemetrySensor = 0x80,
    ControlData = 0xcd
};

enum class Baudrate:uint8_t{
    _115200bps = 0x00,
    _400000bps = 0x01
};

struct Guid{
    uint32_t count;
};

struct Uid{
    uint32_t count;
};

enum class BindType:uint8_t{
    None = 0x00,
    Dsm2_22ms = 0x01,
    Dsm2_mc24 = 0x02
};

namespace msgs{
// The packet format is as follows: 
// <0xA6><0x21><Length><SrcID><DestID><Priority><BaudRate><Info><UID><CRC> 

struct HandShake{
    Baudrate buadrate;

};


}
}