//这个驱动尚未完成
//这个驱动尚未测试

//LD19是一款2D激光雷达 半径12M

// https://ws.dfrobot.com.cn/Fr1DgkvHXX4PYVgKpZgQIsL_lkEn

#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"
#include "algebra/vectors/polar.hpp"


namespace ymd::drivers::m10{
//固定波特率
static constexpr size_t DEFAULT_UART_BAUD = 460800;

static constexpr uint8_t HEADER1_TOKEN = 0xa5;
static constexpr uint8_t HEADER2_TOKEN = 0x5a;

//每个扇区有42点
static constexpr size_t POINTS_PER_SECTOR = 42;

//每圈有360/15 = 24个扇区
static constexpr size_t SECTORS_PER_LAP = 360 / 15;

//每圈有42*24 = 1008点
static constexpr size_t POINTS_PER_LAP = POINTS_PER_SECTOR * SECTORS_PER_LAP;

// speed: 转速参数，一共两个字节,高位在前，低位在后，
// 表示雷达从一个齿转到下一个 齿所需要的时间计数值，它和转速的计算公式为：转速=2500000/speed。
// 例如：0x10 0x68 即十进制 4200 则转速为每分钟 595.239 转，也就是 10HZ；
struct [[nodiscard]] LidarSpinSpeedCode{
public:
    static constexpr LidarSpinSpeedCode from_bits(const uint16_t bits){
        return {bits};
    }

    [[nodiscard]] constexpr uq16 to_tps() const{
        const auto rpm = (2500000u / __bswap16(bits));
        return rpm * uq16(1.0 / 60);
    }
    uint16_t bits;
};

// 角度参数，一共两个字节,高位在前，低位在后，为从 0 度（360 度） 开始每 15 度增加的角度信息。
// 例如：0x8C 0xA0 即十进制 36000 表示角度为 360 度，也就 是 0 度；
struct [[nodiscard]] LidarAngleCode{
public:
    static constexpr LidarAngleCode from_bits(const uint16_t bits){
        return {bits};
    }

    [[nodiscard]] constexpr uq24 to_turns() const{
        constexpr auto LSB_VALUE = uq24(1.0 / 360 * 0.01);
        return LSB_VALUE * __bswap16(bits);
    }

    uint16_t bits;
};

//  Distance: 距离参数，一共两个字节,高位在前，低位在后，表示 15 度差分 42 个点之 后对应角度的距离值，单位是毫米。
// 例如：0x13 0x88 即十进制 5000 那么该角度对应的距 离值就是 5 米。
struct [[nodiscard]] LidarDistanceCode{
    using Self = LidarDistanceCode;

    static constexpr LidarDistanceCode from_bits(const uint16_t bits){
        return {bits};
    }

    [[nodiscard]] constexpr uq24 to_meters() const{
        constexpr auto LSB_VALUE = uq24(0.001);
        return LSB_VALUE * __bswap16(bits);
    }

    uint16_t bits;

    friend OutputStream & operator << (OutputStream & os, const Self & self){
        return os << self.to_meters();
    }
};

//每个帧返回15度内的42个点 总共42 * 24 = 1008个点
struct [[nodiscard]] LidarSector final{
    using Self = LidarSector;

    LidarAngleCode start_angle;
    LidarSpinSpeedCode spin_speed;
    std::array<LidarDistanceCode, POINTS_PER_SECTOR> distances;

    struct [[nodiscard]] Iterator{
        static constexpr uq24 DELTA_TURNS = uq24(15.0 / 360 / POINTS_PER_SECTOR);
        constexpr Iterator(const LidarSector & sector):sector_(sector){
            current_turns_ = sector_.start_angle.to_turns();
        }

        [[nodiscard]] constexpr bool has_next() const{
            return index_ < sector_.distances.size();
        }

        constexpr Polar<uq24> next(){
            const auto ret = Polar<uq24>{
                .amplitude = sector_.distances[index_].to_meters(),
                .phase = Angular<uq24>::from_turns(current_turns_)
            };
            index_ += 1;
            current_turns_ += DELTA_TURNS;
            return ret;
        };
    private:

        const LidarSector & sector_;
        size_t index_ = 0;
        uq24 current_turns_ = 0;
    };

    constexpr auto iter() const{
        return Iterator{*this};
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self){ 
        return os << os.field("start_angle(n)")(self.start_angle.to_turns()) << os.splitter()
            << os.field("spin_speed(n/s)")(self.spin_speed.to_tps());
    };
};

static constexpr size_t NUM_SECTOR_BYTES = sizeof(LidarSector);

namespace events{
struct [[nodiscard]] DataReady{
    using Self = DataReady;

    const LidarSector & sector;

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os << os.field("sector")(self.sector);
    }
};
struct [[nodiscard]] InvalidCrc{
    using Self = InvalidCrc;
    uint8_t expected;
    uint8_t actual;

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os << os.field("expected")(self.expected) << os.splitter()
            << os.field("actual")(self.actual);
    }
};
};

struct [[nodiscard]] Event:public Sumtype<
    events::DataReady,
    events::InvalidCrc
>{
    using DataReady = events::DataReady;
    using InvalidCrc = events::InvalidCrc;
};

using Callback = std::function<void(Event)>;


class M10_ParseReceiver final{
public:
    explicit M10_ParseReceiver(Callback callback):
        callback_(callback)
    {
        reset();
    }

    void push_byte(const uint8_t byte);

    void push_bytes(const std::span<const uint8_t> bytes){
        for(const auto byte : bytes){
            push_byte(byte);
        }
    }

    void flush();

    void reset(){
        state_ = FsmState::WaitingHeader1;
        bytes_count_ = 0;
    }
private:
    union{
        LidarSector sector_;
        alignas(4) std::array<uint8_t, sizeof(LidarSector)> bytes_;
    };

    Callback callback_;

    size_t bytes_count_ = 0;

    enum class FsmState:uint8_t{
        WaitingHeader1,
        WaitingHeader2,
        Remaining
    };

    volatile FsmState state_ = FsmState::WaitingHeader1;

};

}