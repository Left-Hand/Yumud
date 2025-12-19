#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "core/math/realmath.hpp"
#include "core/container/heapless_vector.hpp"
#include "core/tmp/reflect/enum.hpp"

// https://www.waveshare.net/wiki/DDSM400

namespace ymd::robots::waveshare::ddsm400{
static constexpr size_t DEFAULT_BAUDRATE = 38400;

// head command payload crc
static constexpr size_t MAX_PAYLOAD_BYTES = 7;
static constexpr size_t MAX_PACKET_BYTES = 10;


struct [[nodiscard]] MotorId{
    using Self = MotorId;
    uint8_t count;

    static constexpr MotorId from_u8(uint8_t bits) {
        return MotorId{bits};
    }
    [[nodiscard]] constexpr uint8_t to_u8() const {
        return count;
    }
};

// 模式值：
// 0x00：设定为开环
// 0x01：设定为电流环
// 0x02：设定为速度环
// 0x03：设定为位置环
// 0x08：电机使能
// 0x09：电机失能
enum class LoopMode:uint8_t{
    OpenLoop = 0x00,
    CurrentLoop = 0x01,
    SpeedLoop = 0x02,
    PositionLoop = 0x03,
    Activate = 0x08,
    Deactivate = 0x09,
};  

enum class [[nodiscard]] ReqCommand:uint8_t{
    SetTarget = 0x64,
    GetJourney = 0x74,
    SetLoopMode = 0xa0,
    SetMotorId = 0x55,
    GetLoopMode = 0x75,
};

enum class [[nodiscard]] RespCommand:uint8_t{ 
    Feedback = 0x65,
    Feedback2 = 0x75,
    SetLoopMode = 0xa1,
    GetLoopMode = 0x76,
};


#pragma pack(push, 1)
struct FlatPacket{
    MotorId motor_id;
    union{
        ReqCommand req_command;
        RespCommand resp_command;
    };
    uint8_t payload[MAX_PAYLOAD_BYTES];
    uint8_t crc;
};
#pragma pack(pop)

static_assert(sizeof(FlatPacket) == MAX_PACKET_BYTES);

static constexpr uint8_t calc_crc8(){
    //TODO
    return 0;
}

// 电流环模式下：-32767~32767 对应 -4A~4A，数据类型有符号 16 位；
// 速度环模式下：-3800~3800 对应 -380rpm~380rpm，单位 0.1rpm，数据类型有符号 16 位；
// 位置环模式下：0~32767 对应 0°~360°，数据类型无符号 16 位；
struct [[nodiscard]] SpeedCode final{
    int16_t bits;
};

struct [[nodiscard]] CurrentCode final{
    int16_t bits;
};

struct [[nodiscard]] SetPointCode final{
    uint16_t bits;
};

struct [[nodiscard]] AccelerationTimeCode final{ 
    uint8_t bits;
};

struct [[nodiscard]] FaultCode final{
    uint8_t hall:1;
    uint8_t over_current:1;
    uint8_t __resv__:1;
    uint8_t stall:1;
    uint8_t over_temperature:1;
    uint8_t disconnected:1;
    uint8_t voltage:1;
    uint8_t __resv2__:1;
};

struct [[nodiscard]] LapAngleCode final{
    uint16_t bits;
};

struct TempratureCode final{ 
    uint8_t bits;
};

enum class DeMsgError:uint8_t{
    Unnamed
};

static_assert(sizeof(FaultCode) == 1);
}


