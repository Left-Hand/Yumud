#pragma once

#include <cstdint>
#include <bit>

// https://github.com/SpektrumRC/SRXL2/blob/master/Docs/SRXL2%20Specification.pdf

namespace ymd::srxl2{


enum class [[nodiscard]] DeviceType:uint8_t{
    None = 0x00,
    RemoteReceiver = 0x1,
    Receiver = 0x2,
};


struct [[nodiscard]] DeviceId final{
    uint8_t index : 4;
    DeviceType type : 4;

    [[nodiscard]] constexpr uint8_t to_u8() const {
        return std::bit_cast<uint8_t>(*this);
    }

    [[nodiscard]] constexpr bool is_boardcast() const{
        return to_u8() == 0xff;
    }

    [[nodiscard]] constexpr bool need_ack() const{
        return to_u8() != 0;
    }
};


static_assert(sizeof(DeviceId) == 1);


enum class [[nodiscard]] PacketId:uint8_t{
    HandShake = 0x21,
    BindInfo = 0x41,
    ParameterConfig = 0x50,
    SignalQuality = 0x55,
    TelemetrySensor = 0x80,
    ControlData = 0xcd
};

enum class [[nodiscard]] Baudrate:uint8_t{
    _115200bps = 0x00,
    _400000bps = 0x01
};

struct [[nodiscard]] Guid final{
    uint32_t bits;
};

struct [[nodiscard]] Uid final{
    uint32_t bits;
};

enum class [[nodiscard]] BindType:uint8_t{
    None = 0x00,
    Dsm2_22ms = 0x01,
    Dsm2_mc24 = 0x02
};

namespace msgs{
// The packet format is as follows: 
// <0xA6><0x21><Length><SrcID><DestID><Priority><BaudRate><Info><UID><CRC> 

struct [[nodiscard]] HandShake final{
    Baudrate buadrate;

};


}
}