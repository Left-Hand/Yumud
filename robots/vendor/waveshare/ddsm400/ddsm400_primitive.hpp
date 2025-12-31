#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "primitive/arithmetic/angular.hpp"

#include "core/math/realmath.hpp"
#include "core/container/heapless_vector.hpp"
#include "core/tmp/reflect/enum.hpp"

// https://www.waveshare.net/wiki/DDSM400

namespace ymd::robots::waveshare::ddsm400::primitive{
static constexpr size_t DEFAULT_BAUDRATE = 38400;

// head command payload crc
static constexpr size_t NUM_PAYLOAD_BYTES = 7;
static constexpr size_t NUM_PACKET_BYTES = 10;


struct [[nodiscard]] MotorId final{
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
enum class [[nodiscard]] LoopMode:uint8_t{
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

enum class [[nodiscard]] DeMsgError:uint8_t{
    Unnamed,
    CommandNotMatch,
    InvalidCommand,
};


enum class [[nodiscard]] SerMsgError:uint8_t{
    CurrentOverflow,
    CurrentUnderflow,
    SpeedOverflow,
    SpeedUnderflow,
};

static constexpr Option<ReqCommand> try_into_req_command(const uint8_t b){
    // return tmp::enum_match<ReqCommand>(b, [](auto cmd){
    //     return Option<ReqCommand>::some(cmd);
    // });

    switch(std::bit_cast<ReqCommand>(b)){
        case ReqCommand::SetTarget: return Some(ReqCommand::SetTarget);
        case ReqCommand::GetJourney: return Some(ReqCommand::GetJourney);
        case ReqCommand::SetLoopMode: return Some(ReqCommand::SetLoopMode);
        case ReqCommand::SetMotorId: return Some(ReqCommand::SetMotorId);
        case ReqCommand::GetLoopMode: return Some(ReqCommand::GetLoopMode);
    }
    return None;
}


// CRC-8/MAXIM
static constexpr uint8_t calc_crc8(std::span<const uint8_t> bytes){
    // https://www.codeleading.com/article/59474838692/
    // 遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接和本声明

    size_t len = bytes.size();
    const uint8_t *data = bytes.data();

    uint8_t crc, i;
    crc = 0x00;

    while(len--)
    {
        crc ^= *data++;
        for(i = 0;i < 8;i++)
        {
            if(crc & 0x01)
            {
                crc = (crc >> 1) ^ 0x8c;
            }
                else crc >>= 1;
        }
    }
    return crc;
}

#pragma pack(push, 1)
struct FlatPacket{
    MotorId motor_id;
    union{
        ReqCommand req_command;
        RespCommand resp_command;
    };
    uint8_t payload[NUM_PAYLOAD_BYTES];

    constexpr uint8_t calc_crc() const { 
        return calc_crc8(std::span(&motor_id.count, NUM_PACKET_BYTES - 1));
    }
};
#pragma pack(pop)

static_assert(sizeof(FlatPacket) == NUM_PACKET_BYTES - 1);



struct [[nodiscard]] SetPointCode final{
    int16_t bits;
};


// 电流环模式下：-32767~32767 对应 -4A~4A，数据类型有符号 16 位；
struct [[nodiscard]] CurrentCode final{
    using Self = CurrentCode;
    int16_t bits;
    static constexpr Result<Self, SerMsgError> try_from_amps(const iq16 amps){
        if(amps > 4) return Err(SerMsgError::CurrentOverflow);
        if(amps < -4) return Err(SerMsgError::CurrentUnderflow);
        const int16_t bits = static_cast<int16_t>((amps >> 3).to_bits());
        return Ok(Self{.bits = bits});
    }

    constexpr iq16 to_amps() const {
        return iq16::from_bits(int32_t(bits) << 3);
    }

    constexpr operator SetPointCode() const {
        return SetPointCode{.bits = bits};
    }
};

static_assert(sizeof(CurrentCode) == 2);


// 速度环模式下：-3800~3800 对应 -380rpm~380rpm，单位 0.1rpm，数据类型有符号 16 位；

struct [[nodiscard]] SpeedCode final{
    using Self = SpeedCode;
    int16_t bits;

    static constexpr Result<Self, SerMsgError> try_from_rpm(const iq16 rpm){
        if(rpm > 380) return Err(SerMsgError::SpeedOverflow);
        if(rpm < -380) return Err(SerMsgError::SpeedUnderflow);
        return Ok(SpeedCode{.bits = static_cast<int16_t>(rpm * 10)});
    }

    static constexpr Result<Self, SerMsgError> try_from_rps(const iq16 rps){
        return try_from_rpm(rps * 60);
    }

    constexpr iq16 to_rpm() const {
        return iq16::from_bits(bits) / 10;
    }

    constexpr iq16 to_rps() const {
        return iq16::from_bits(bits) / 600;
    }

    constexpr operator SetPointCode() const {
        return SetPointCode{.bits = bits};
    }
};



// 位置环模式下：0~32767 对应 0°~360°，数据类型无符号 16 位；
struct [[nodiscard]] LapAngleCode final{
    using Self = LapAngleCode;
    uint16_t bits;

    static constexpr Self from_angle(const Angular<uq32> angle){
        return LapAngleCode{.bits = static_cast<uint16_t>(angle.to_turns().to_bits() >> 16)};
    }

    constexpr Angular<uq32> to_angle() const {
        return Angular<uq32>::from_turns(uq32::from_bits(static_cast<uint32_t>(bits) << 16));
    }

    constexpr operator SetPointCode() const {
        return SetPointCode{.bits = std::bit_cast<int16_t>(bits)};
    }
};

static_assert(sizeof(LapAngleCode) == 2);


// 加速时间：速度环模式下有效，每1rpm的加速时间，单位为1ms，
// 当设置为1时，每1rpm的加速时间为1ms，当设置为10时，每1rpm的加速时间为 10*1ms=10ms，
// 设置为0时，既默认为1，每1rpm的加速时间为1ms
struct [[nodiscard]] AccelerationTimeCode final{ 
    uint8_t bits;
};

struct [[nodiscard]] TempratureCode final{ 
    uint8_t bits;
};

// 故障码：
// 故障值	BIT7	BIT6	BIT5	BIT4	BIT3	BIT2	BIT1	BIT0
// 内容	保留	过欠压故障	断联故障	过温故障	堵转故障	保留	过流故障	霍尔故障
// 例如故障码为：0x02 即为 0b00000010，表示发生过流故障。
struct [[nodiscard]] FaultFlags final{
    uint8_t hall:1;
    uint8_t over_current:1;
    uint8_t __resv__:1;
    uint8_t stall:1;
    uint8_t over_temperature:1;
    uint8_t disconnected:1;
    uint8_t voltage:1;
    uint8_t __resv2__:1;
};




static_assert(sizeof(FaultFlags) == 1);
}


