//这个驱动尚未完成
//这个驱动尚未测试

//LD19是一款2D激光雷达 半径12M

// https://ws.dfrobot.com.cn/Fr1DgkvHXX4PYVgKpZgQIsL_lkEn

#pragma once

#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"


namespace ymd::drivers::ld19{


static constexpr size_t DEFAULT_UART_BAUD = 230400;

static constexpr uint8_t HEADER_TOKEN = 0x54;
static constexpr uint8_t VERLEN_TOKEN = 0x2C;
static constexpr size_t POINTS_PER_FRAME = 12;

struct [[nodiscard]] LidarPoint{

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
        //  << os.splitter()
            // << self.intensity << ;
    }
};

struct [[nodiscard]] LidarSpinSpeedCode{
public:
    static constexpr LidarSpinSpeedCode from_bits(const uint16_t bits){
        return LidarSpinSpeedCode{bits};
    }

    [[nodiscard]] constexpr uq16 to_turns_per_secs() const{
        constexpr uq16 RATIO = uq16(1.0 / 360);
        return RATIO * bits;
    }

    uint16_t bits;
};

struct [[nodiscard]] LidarAngleCode{
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

struct [[nodiscard]] TimeStamp{
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
    LidarSpinSpeedCode spin_speed;
    LidarAngleCode start_angle;


    struct [[nodiscard]] LidarPoints{
        std::array<uint8_t, 3 * POINTS_PER_FRAME> bytes;

        //3字节对齐 必须值语义返回
        [[nodiscard]] constexpr LidarPoint operator[](size_t index) const{
            return LidarPoint::from_bytes(std::span<const uint8_t, 3>(bytes.data() + index * 3, 3));
        }
    };
    LidarPoints points;
    LidarAngleCode end_angle;
    TimeStamp timestamp;
    #pragma pack(pop)
};



static constexpr size_t FRAME_SIZE = 44;


namespace events{
    struct [[nodiscard]] DataReady{
        const LidarSectorPacket & packet;
    };
    struct [[nodiscard]] InvalidCrc{
        uint8_t expected;
        uint8_t actual;

        friend OutputStream & operator<<(OutputStream & os, const InvalidCrc & self){
            return os << "expected: " << self.actual << os.splitter() 
                << " actual: " << self.expected;
        } 
    };
};

struct Event:public Sumtype<events::DataReady, events::InvalidCrc> {
    using DataReady = events::DataReady;
    using InvalidCrc = events::InvalidCrc;
};

using Callback = std::function<void(Event)>;


class LD19_ParserSink final{
public:
    explicit LD19_ParserSink(Callback callback):
        callback_(callback)
    {
        reset();
    }

    void push_byte(const uint8_t byte);

    void push_bytes(const std::span<const uint8_t> bytes);

    void flush();

    void reset();
private:

    Callback callback_;
    union{
        LidarSectorPacket frame_;
        alignas(4) std::array<uint8_t, 48> bytes_;
    };
    size_t bytes_count_ = 0;

    enum class State:uint8_t{
        WaitingHeader,
        WaitingVerlen,
        Remaining,
        Emitting
    };

    State state_ = State::WaitingHeader;

};

}