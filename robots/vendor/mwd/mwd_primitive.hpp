#pragma once

#include "core/math/fixed/fixed.hpp"
#include "core/math/float/fp32.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "core/utils/enum/strong_type_gradation.hpp"
#include "core/string/view/string_view.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "primitive/arithmetic/percentage.hpp"
#include "robots/vendor/mit/mit_primitive.hpp"



namespace ymd::robots::mwd{

namespace primitive{

enum class MotorType{
    K,
    TS,
    L
};

//反序列化错误
enum class [[nodiscard]] DeMsgError:uint8_t{
    
};

enum class [[nodiscard]] CommandKind : uint8_t {
    Nop = 0x00,// 无操作
    SetZeroToRom = 0x19,// 设置当前位置为电机零点（写入ROM）
    MotorStop = 0x80,// 电机关闭
    MotorEStop = 0x81,// 电机紧急停止
    BrakeControl = 0x8C,// 抱闸器控制/读取
    MotorRun = 0x88,// 电机运行
    OpenLoopControl = 0xA0,// 开环控制（L电机）
    TorqueControl = 0xA1,// 转矩闭环控制（K/ZH/TS电机）
    SpeedControl = 0xA2,// 速度闭环控制
    PositionMultiLoop1 = 0xA3,// 多圈位置闭环控制1
    PositionMultiLoop2 = 0xA4,// 多圈位置闭环控制2（带限速）
    PositionSingleLoop1 = 0xA5,// 单圈位置闭环控制1
    PositionSingleLoop2 = 0xA6,// 单圈位置闭环控制2（带方向+限速）
    PositionIncrement1 = 0xA7,// 增量位置闭环控制1
    PositionIncrement2 = 0xA8,// 增量位置闭环控制2（带限速）
    GetStatus1 = 0x9A,// 读取电机状态1 + 错误标志
    ClearError = 0x9B,// 清除电机错误标志
    GetStatus2 = 0x9C,// 读取电机状态2（温度/转矩/速度/位置）
    GetStatus3 = 0x9D,// 读取电机状态3（温度/三相电流）
    GetEncoder = 0x90,// 读取编码器数据
    GetAngleMulti = 0x92,// 读取多圈角度
    ClearTurnCount = 0x93,// 清除电机圈数信息
    GetAngleSingle = 0x94,// 读取单圈角度
    SetAngleManual = 0x95,// 设置当前位置为任意角度（RAM）
    GetParam = 0xC0,// 读取控制参数
    SetParamRam = 0xC1,// 写入控制参数到RAM
};

struct [[nodiscard]] ErrorState{
    using Self = ErrorState;
    using Bits = uint8_t;

    uint8_t under_voltage : 1;
    uint8_t over_voltage : 1;
    uint8_t driver_over_temperature : 1;
    uint8_t motor_over_temperature : 1;
    uint8_t motor_over_current : 1;
    uint8_t motor_short_circuit : 1;
    uint8_t motor_stall : 1;
    uint8_t signal_timeout : 1;

    static constexpr Self from_bits(const Bits bits){
        return std::bit_cast<Self>(bits);
    }

    constexpr Bits to_bits() const{
        return std::bit_cast<Bits>(*this);
    }
};


struct [[nodiscard]] MotorTemperatureCode final{
    int8_t bits;

    constexpr int8_t to_celeus() const {
        return bits;
    }
};


static constexpr iq16 div100(const int16_t bits){
    static constexpr uint64_t FACTOR = static_cast<uint64_t>((1ull << 48) * 0.01);
    return iq16::from_bits(static_cast<int32_t>((bits * FACTOR) >> 32));
}

struct [[nodiscard]] VoltageCode final{
    using Self = VoltageCode;
    using Bits = int16_t;
    int16_t bits;

    static constexpr uint32_t RATIO = 100;
    static constexpr double LSB_VOLTS = 1.0 / RATIO;
    static constexpr double MAX_VOLTS = LSB_VOLTS * std::numeric_limits<Bits>::max();
    static constexpr double MIN_VOLTS = LSB_VOLTS * std::numeric_limits<Bits>::min();

    constexpr iq16 to_volts() const {
        return div100(bits);
    }

    static constexpr Result<VoltageCode,std::strong_ordering> from_volts(const iq16 volts){
        if(volts > iq16(MAX_VOLTS)) return Err(std::strong_ordering::greater);
        if(volts < iq16(MIN_VOLTS)) return Err(std::strong_ordering::less);

        const int32_t ret_bits = math::round_cast<int32_t>(volts * RATIO);
        if(ret_bits > std::numeric_limits<Bits>::max()) __builtin_unreachable();
        if(ret_bits < std::numeric_limits<Bits>::min()) __builtin_unreachable();
        return Ok(Self{.bits = static_cast<int16_t>(ret_bits)});
    }
};

static_assert(std::abs((double)VoltageCode{.bits = 10000}.to_volts() - 100) < 1E-4);
static_assert(std::abs((double)VoltageCode{.bits = -10000}.to_volts() - -100) < 1E-4);


struct [[nodiscard]] CurrentCode final{
    using Self = CurrentCode;
    using Bits = int16_t;
    int16_t bits;

    static constexpr uint32_t RATIO = 100;
    static constexpr double LSB_AMPS = 1.0 / RATIO;
    static constexpr double MAX_AMPS = LSB_AMPS * std::numeric_limits<Bits>::max();
    static constexpr double MIN_AMPS = LSB_AMPS * std::numeric_limits<Bits>::min();

    constexpr iq16 to_volts() const {
        return div100(bits);
    }

    static constexpr Result<CurrentCode,std::strong_ordering> from_volts(const iq16 amps){
        if(amps > iq16(MAX_AMPS)) return Err(std::strong_ordering::greater);
        if(amps < iq16(MIN_AMPS)) return Err(std::strong_ordering::less);

        const int32_t ret_bits = math::round_cast<int32_t>(amps * RATIO);
        if(ret_bits > std::numeric_limits<Bits>::max()) __builtin_unreachable();
        if(ret_bits < std::numeric_limits<Bits>::min()) __builtin_unreachable();
        return Ok(Self{.bits = static_cast<Bits>(ret_bits)});
    }
};

//TODO 进行正确计算并添加测试
struct [[nodiscard]] TorqueCode final{
    // K、TS电机的转矩电流值iq或L电机的输出功率值power，int16_t类型。
    // TS电机iq分辨率为(66/4096 A)/LSB；
    // K电机iq分辨率为(33/4096 A)/LSB。
    // L电机power范围-1000~1000

    using Self = TorqueCode;
    using Bits = int16_t;
    int16_t bits;

    static constexpr uint32_t RATIO = 100;
    static constexpr double LSB_AMPS = 1.0 / RATIO;
    static constexpr double MAX_AMPS = LSB_AMPS * std::numeric_limits<Bits>::max();
    static constexpr double MIN_AMPS = LSB_AMPS * std::numeric_limits<Bits>::min();

    constexpr iq16 to_volts() const {
        return div100(bits);
    }

    static constexpr Result<TorqueCode,std::strong_ordering> from_volts(const iq16 amps){
        if(amps > iq16(MAX_AMPS)) return Err(std::strong_ordering::greater);
        if(amps < iq16(MIN_AMPS)) return Err(std::strong_ordering::less);

        const int32_t ret_bits = math::round_cast<int32_t>(amps * RATIO);
        if(ret_bits > std::numeric_limits<Bits>::max()) __builtin_unreachable();
        if(ret_bits < std::numeric_limits<Bits>::min()) __builtin_unreachable();
        return Ok(Self{.bits = static_cast<Bits>(ret_bits)});
    }
};


//TODO 进行正确计算并添加测试
struct [[nodiscard]] PhaseCurrentCode final{
    // TS电机相电流分辨率(66/4096 A)/LSB；K电机(33/4096 A)/LSB
    using Self = PhaseCurrentCode;
    using Bits = int16_t;
    int16_t bits;

    static constexpr uint32_t RATIO = 100;
    static constexpr double LSB_AMPS = 1.0 / RATIO;
    static constexpr double MAX_AMPS = LSB_AMPS * std::numeric_limits<Bits>::max();
    static constexpr double MIN_AMPS = LSB_AMPS * std::numeric_limits<Bits>::min();

    constexpr iq16 to_amps() const {
        return div100(bits);
    }

    static constexpr Result<PhaseCurrentCode,std::strong_ordering> from_volts(const iq16 amps){
        if(amps > iq16(MAX_AMPS)) return Err(std::strong_ordering::greater);
        if(amps < iq16(MIN_AMPS)) return Err(std::strong_ordering::less);

        const int32_t ret_bits = math::round_cast<int32_t>(amps * RATIO);
        if(ret_bits > std::numeric_limits<Bits>::max()) __builtin_unreachable();
        if(ret_bits < std::numeric_limits<Bits>::min()) __builtin_unreachable();
        return Ok(Self{.bits = static_cast<Bits>(ret_bits)});
    }
};

struct [[nodiscard]] MotorState{
    uint8_t bits;

    template <typename Self>
    [[nodiscard]] constexpr auto over_voltage(this Self && self) {
        return make_bitfield_proxy<4, 1, bool>(self.bits);}
};


struct EncoderSpeedCode{
    using Self = EncoderSpeedCode;
    using Bits = int16_t;

    //per lsb is 1 degree/s
    int16_t bits;

    static constexpr double MAX_TPS = std::numeric_limits<Bits>::max() / 360.0;
    static constexpr double MIN_TPS = std::numeric_limits<Bits>::min() / 360.0;

    constexpr Result<Self, std::strong_ordering> from_tps(const iq16 tps){
        if(tps > iq16(MAX_TPS)) return Err(std::strong_ordering::greater);
        if(tps < iq16(MIN_TPS)) return Err(std::strong_ordering::less);
        const int32_t ret_bits = math::round_cast<int32_t>(tps * 360);
        if(ret_bits > std::numeric_limits<Bits>::max()) __builtin_unreachable();
        if(ret_bits < std::numeric_limits<Bits>::min()) __builtin_unreachable();
        return Ok(Self{.bits = static_cast<Bits>(ret_bits)});
    }
    
    constexpr int16_t to_dps() const {
        return bits;
    }
};

struct EncoderPositionCode{
    using Self = EncoderPositionCode;

    uint16_t bits;

    static constexpr Self from_angle_16b(const Angular<uq32> angle){
        const auto bits = static_cast<uint16_t>(angle.to_turns().to_bits() >> 16); 
        return Self{.bits = bits};
    }

    static constexpr Self from_angle_14b(const Angular<uq32> angle){
        const auto bits = static_cast<uint16_t>(angle.to_turns().to_bits() >> 18); 
        return Self{.bits = bits};
    }
};  


struct PackedStatus1{
    using Self = PackedStatus1;
    static constexpr size_t LENGTH = 7;

    MotorTemperatureCode temperature_code;
    VoltageCode busbar_voltage_code;
    CurrentCode busbar_current_code;
    MotorState motor_state;
    ErrorState error_state;

    static constexpr Result<Self, DeMsgError> 
    try_from_bytes(std::span<const uint8_t, LENGTH> bytes){
        return Ok(Self{
            .temperature_code =         le_bytes_ctor_bits(bytes.subspan<0, 1>()),
            .busbar_voltage_code =      le_bytes_ctor_bits(bytes.subspan<1, 2>()),
            .busbar_current_code =      le_bytes_ctor_bits(bytes.subspan<3, 2>()),
            .motor_state =              le_bytes_ctor_bits(bytes.subspan<5, 1>()),
            .error_state =              le_bytes_ctor_bits(bytes.subspan<6, 1>())
        });
    }
};

struct PackedStatus2{

    using Self = PackedStatus2;
    static constexpr size_t LENGTH = 7;

    MotorTemperatureCode temperature_code;
    TorqueCode torque_code;
    EncoderSpeedCode encoder_speed_code;
    EncoderPositionCode encoder_position_code;

    static constexpr Result<Self, DeMsgError> 
    try_from_bytes(std::span<const uint8_t, LENGTH> bytes){
        return Ok(Self{
            .temperature_code =         le_bytes_ctor_bits(bytes.subspan<0, 1>()),
            .torque_code =              le_bytes_ctor_bits(bytes.subspan<1, 2>()),
            .encoder_speed_code =       le_bytes_ctor_bits(bytes.subspan<3, 2>()),
            .encoder_position_code =    le_bytes_ctor_bits(bytes.subspan<5, 2>()),
        });
    }
};

struct PackedStatus3{

    using Self = PackedStatus3;
    static constexpr size_t LENGTH = 7;

    MotorTemperatureCode temperature_code;
    std::array<PhaseCurrentCode, 3> current_codes;

    static constexpr Result<Self, DeMsgError> 
    try_from_bytes(std::span<const uint8_t, LENGTH> bytes){
        return Ok(Self{
            .temperature_code = le_bytes_ctor_bits(bytes.subspan<0, 1>()),
            .current_codes = {
                le_bytes_ctor_bits(bytes.subspan<1, 2>()),
                le_bytes_ctor_bits(bytes.subspan<3, 2>()),
                le_bytes_ctor_bits(bytes.subspan<5, 2>())
            }
        });
    }

    template<typename Receiver>
    constexpr Result<void, typename Receiver::Error>
    serialize(Receiver & receiver) const {
        
    }
};

}

// using namespace primitive;

}