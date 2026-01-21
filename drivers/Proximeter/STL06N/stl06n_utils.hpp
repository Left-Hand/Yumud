#pragma once

#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"


namespace ymd::drivers::stl06n{


static constexpr std::array<uint8_t, 256> CRC8_TABLE= {
0x00,0x4d,0x9a,0xd7,0x79,0x34,0xe3,
0xae,0xf2,0xbf,0x68,0x25,0x8b,0xc6,0x11,0x5c,0xa9,0xe4,0x33,
0x7e,0xd0,0x9d,0x4a,0x07,0x5b,0x16,0xc1,0x8c,0x22,0x6f,0xb8,
0xf5,0x1f,0x52,0x85,0xc8,0x66,0x2b,0xfc,0xb1,0xed,0xa0,0x77,
0x3a,0x94,0xd9,0x0e,0x43,0xb6,0xfb,0x2c,0x61,0xcf,0x82,0x55,
0x18,0x44,0x09,0xde,0x93,0x3d,0x70,0xa7,0xea,0x3e,0x73,0xa4,
0xe9,0x47,0x0a,0xdd,0x90,0xcc,0x81,0x56,0x1b,0xb5,0xf8,0x2f,
0x62,0x97,0xda,0x0d,0x40,0xee,0xa3,0x74,0x39,0x65,0x28,0xff,
0xb2,0x1c,0x51,0x86,0xcb,0x21,0x6c,0xbb,0xf6,0x58,0x15,0xc2,
0x8f,0xd3,0x9e,0x49,0x04,0xaa,0xe7,0x30,0x7d,0x88,0xc5,0x12,
0x5f,0xf1,0xbc,0x6b,0x26,0x7a,0x37,0xe0,0xad,0x03,0x4e,0x99,
0xd4,0x7c,0x31,0xe6,0xab,0x05,0x48,0x9f,0xd2,0x8e,0xc3,0x14,
0x59,0xf7,0xba,0x6d,0x20,0xd5,0x98,0x4f,0x02,0xac,0xe1,0x36,
0x7b,0x27,0x6a,0xbd,0xf0,0x5e,0x13,0xc4,0x89,0x63,0x2e,0xf9,
0xb4,0x1a,0x57,0x80,0xcd,0x91,0xdc,0x0b,0x46,0xe8,0xa5,0x72,
0x3f,0xca,0x87,0x50,0x1d,0xb3,0xfe,0x29,0x64,0x38,0x75,0xa2,
0xef,0x41,0x0c,0xdb,0x96,0x42,0x0f,0xd8,0x95,0x3b,0x76,0xa1,
0xec,0xb0,0xfd,0x2a,0x67,0xc9,0x84,0x53,0x1e,0xeb,0xa6,0x71,
0x3c,0x92,0xdf,0x08,0x45,0x19,0x54,0x83,0xce,0x60,0x2d,0xfa,
0xb7,0x5d,0x10,0xc7,0x8a,0x24,0x69,0xbe,0xf3,0xaf,0xe2,0x35,
0x78,0xd6,0x9b,0x4c,0x01,0xf4,0xb9,0x6e,0x23,0x8d,0xc0,0x17,
0x5a,0x06,0x4b,0x9c,0xd1,0x7f,0x32,0xe5,0xa8
};


class [[nodiscard]] Crc8Calculator {
private:
    uint8_t crc_;

public:
    constexpr explicit Crc8Calculator(uint8_t initial_crc = 0) : crc_(initial_crc) {}

    [[nodiscard]] constexpr Crc8Calculator push_byte(uint8_t byte) const {
        uint8_t crc = crc_;
        crc = CRC8_TABLE[(crc ^ byte) & 0xff];
        return Crc8Calculator{crc};
    }

    [[nodiscard]] constexpr Crc8Calculator push_bytes(std::span<const uint8_t> bytes) const {
        uint8_t crc = crc_;
        for (const auto byte : bytes) {
            crc = CRC8_TABLE[(crc ^ byte) & 0xff];
        }
        return Crc8Calculator{crc};
    }

    [[nodiscard]] constexpr uint8_t get() const {
        return crc_;
    }
};

static constexpr size_t DEFAULT_UART_BAUD = 230400;

static constexpr uint8_t HEADER_TOKEN = 0x54;
static constexpr uint8_t SECTOR_COMMAND = 0x2C;
static constexpr size_t POINTS_PER_FRAME = 12;

static constexpr size_t SECTOR_PAYLOAD_LENGTH = 2 + 2 + 12 * 3 + 2 + 2 ;

struct alignas(4) [[nodiscard]] LidarPoint final{

    uint16_t distance_mm;
    uint8_t intensity;

    static constexpr LidarPoint from_bytes(const std::span<const uint8_t, 3> bytes){
        return LidarPoint{
            .distance_mm = static_cast<uint16_t>((bytes[1] << 8) | bytes[0]),
            .intensity = bytes[2]
        };
    }

    [[nodiscard]] constexpr uq16 distance_meters() const {
        return distance_mm * 0.001_uq16;
    }

    friend OutputStream & operator<<(OutputStream & os, const LidarPoint & self){
        return os << self.distance_meters() << "m";
    }
};

struct alignas(1) [[nodiscard]] Command final{
    using Self = Command;

    enum class [[nodiscard]] Kind:uint8_t{
        Sector = 0x2c,
        Start = 0xA0,
        Stop = 0xA1,
        SetSpeed = 0xa2,
        GetSpeed = 0xa3
    };

    using enum Kind;

    constexpr Command(Kind kind):kind_(kind){}

    [[nodiscard]] static constexpr Option<Self> try_from_u8(const uint8_t bits){
        switch(bits){
            case 0x2c:
            case 0xa0:
            case 0xa1:
            case 0xa2:
            case 0xa3:{
                return Some(Self(static_cast<Kind>(bits)));
            }
        }
        return None;
    }
    [[nodiscard]] constexpr uint8_t to_u8() const{
        return static_cast<uint8_t>(kind_);
    }

    [[nodiscard]] constexpr size_t payload_length() const {
        switch(kind_){
            case Kind::Sector: return SECTOR_PAYLOAD_LENGTH;
            default: return 1 + 4 + 1;
        }
        __builtin_unreachable();
    }

    [[nodiscard]] constexpr Kind kind() const{
        return kind_;
    }
private:
    Kind kind_;

    friend OutputStream & operator << (OutputStream & os, const Self self){
        switch(self.kind()){
            case Kind::Sector: return os << "Sector";
            case Kind::Start: return os << "Start";
            case Kind::Stop: return os << "Stop";
            case Kind::SetSpeed: return os << "SetSpeed";
            case Kind::GetSpeed: return os << "GetSpeed";
        }
        __builtin_unreachable();
    }
};

struct alignas(2) [[nodiscard]] LidarSpinSpeedCode final{
public:
    using Self = LidarSpinSpeedCode;
    static constexpr Self from_bits(const uint16_t bits){
        return Self{bits};
    }

    static constexpr Self from_bytes(const uint8_t b0, const uint8_t b1){
        return Self{
            static_cast<uint16_t>(static_cast<uint16_t>(b0) | (static_cast<uint16_t>(b1) << 8))
        };
    }

    static constexpr Self from_tps(const uq16 tps){
        return Self::from_bits(static_cast<uint16_t>(tps * 360));
    }

    [[nodiscard]] constexpr uq16 to_tps() const{
        constexpr uq16 RATIO = uq16(1.0 / 360);
        return RATIO * bits;
    }

    uint16_t bits;
};

struct alignas(2) [[nodiscard]] LidarAngleCode final{
public:
    static constexpr LidarAngleCode from_bits(const uint16_t bits){
        return LidarAngleCode{bits};
    }

    [[nodiscard]] constexpr uq16 to_turns() const{
        constexpr auto RATIO = iq24(1.0 / 360 * 0.01);
        return RATIO * bits;
    }

    uint16_t bits;
};


struct alignas(2) [[nodiscard]] TimeStamp final{
    uint16_t bits;

    static constexpr TimeStamp from_bits(const uint16_t bits){
        return TimeStamp{bits};
    }

    [[nodiscard]] constexpr std::chrono::duration<uint16_t, std::milli>
    to_ms() const {
        return std::chrono::duration<uint16_t, std::milli>(bits);
    }
};

struct [[nodiscard]] LidarSectorPacket final{
    using Self = LidarSectorPacket;

    #pragma pack(push, 1)
    LidarSpinSpeedCode spin_speed;//[0:2]
    LidarAngleCode start_angle;//[2:4]


    struct [[nodiscard]] Points{
        std::array<uint8_t, 3 * POINTS_PER_FRAME> bytes;

        //3字节对齐 必须值语义返回
        [[nodiscard]] constexpr LidarPoint operator[](size_t index) const{
            return LidarPoint::from_bytes(std::span<const uint8_t, 3>(bytes.data() + index * 3, 3));
        }
    };
    Points points;//[4:40]
    LidarAngleCode stop_angle;//[40:42]
    TimeStamp timestamp;//[42:44]
    uint8_t crc8;//[44:45]
    #pragma pack(pop)

    static constexpr size_t PAYLOAD_LEN = 44;
    [[nodiscard]] uint8_t calc_crc() const {
        const auto payload_bytes = std::span<const uint8_t, PAYLOAD_LEN>(
            reinterpret_cast<const uint8_t *>(this),
            PAYLOAD_LEN
        );

        Crc8Calculator calc = Crc8Calculator();
        return calc.push_bytes(payload_bytes).get();
    }
};

static_assert(sizeof(LidarSectorPacket) == 46);

struct [[nodiscard]] ReqArg final{
    using Self = ReqArg;

    uint32_t bits;

    static constexpr Self zero(){
        return Self{0};
    }
};

struct req_msgs{
struct [[nodiscard]] Start final{
    static constexpr Command COMMAND = Command::Start;

    constexpr ReqArg to_req_arg() const {
        return ReqArg::zero();
    }
};

struct [[nodiscard]] Stop final{
    static constexpr Command COMMAND = Command::Stop;
    constexpr ReqArg to_req_arg() const {
        return ReqArg::zero();
    }
};

struct [[nodiscard]] SetSpeed final{
    static constexpr Command COMMAND = Command::SetSpeed;

    LidarSpinSpeedCode speed;
    constexpr ReqArg to_req_arg() const {
        return ReqArg{static_cast<uint32_t>(speed.bits)};
    }
};

struct [[nodiscard]] GetSpeed final{
    static constexpr Command COMMAND = Command::GetSpeed;

    constexpr ReqArg to_req_arg() const {
        return ReqArg::zero();
    }
};
};

struct resp_msgs{
struct [[nodiscard]] Start final{
    static constexpr Command COMMAND = Command::Start;

};

struct [[nodiscard]] Stop final{
    static constexpr Command COMMAND = Command::Stop;
};

struct [[nodiscard]] SetSpeed final{
    static constexpr Command COMMAND = Command::SetSpeed;
};

struct [[nodiscard]] GetSpeed final{
    static constexpr Command COMMAND = Command::GetSpeed;
    LidarSpinSpeedCode speed;
};
};

namespace events{
struct [[nodiscard]] DataReady final{
    const LidarSectorPacket & sector;
};

struct [[nodiscard]] InvalidCrc final{
    using Self = InvalidCrc;
    Command command;
    uint8_t expected;
    uint8_t actual;

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os << os.field("command")(self.command) << os.splitter()
            << os.field("expected")(self.expected) << os.splitter()
            << os.field("actual")(self.actual);
    }
};

using Start = resp_msgs::Start;
using Stop = resp_msgs::Stop;
using SetSpeed = resp_msgs::SetSpeed;
using GetSpeed = resp_msgs::GetSpeed;
};

struct Event:public Sumtype<
    events::DataReady,
    events::InvalidCrc,
    events::Start,
    events::Stop,
    events::SetSpeed,
    events::GetSpeed
    > {
    using DataReady = events::DataReady;
    using InvalidCrc = events::InvalidCrc;


    using Start = events::Start;
    using Stop = events::Stop;
    using SetSpeed = events::SetSpeed;
    using GetSpeed = events::GetSpeed;
};
}
