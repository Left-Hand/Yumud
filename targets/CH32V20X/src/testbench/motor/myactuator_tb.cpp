#include "src/testbench/tb.h"

#include "primitive/arithmetic/percentage.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/utils/bytes/bytes_provider.hpp"
#include "core/utils/bits/bits_caster.hpp"
#include "core/utils/bits/from_bits_debinder.hpp"
#include "core/utils/enum/strong_type_gradation.hpp"
#include "core/string/string_view.hpp"
#include "core/math/float/fp32.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "types/transforms/Basis.hpp"

#include "robots/vendor/DJI/M3508/m3508.hpp"
#include "robots/vendor/DJI/DR16/DR16.hpp"


namespace ymd::drivers::myact { 

// p_des:-12.5到 12.5, 单位rad;
// 数据类型为uint16_t, 取值范围为0~65535, 其中0代表-12.5,65535代表 12.5,
//  0~65535中间的所有数值，按比例映射 至-12.5~12.5。
DEF_U16_STRONG_TYPE_GRADATION(MitPositionCode_u16,  from_radians,    
    iq16,   -12.5,  12.5,   25.0/65535)

// v_des:-45到 45, 单位rad/s;
// 数据类型为12位无符号整数，取值范围为0~4095,其中0代表-45,4095代表45,
//  0~4095 中间的所有数值，按比例映射至-45~45。
DEF_U16_STRONG_TYPE_GRADATION(MitSpeedCode_u12,     from_radians,    
    iq16,   -45,    45,     90.0/4095)

// kp: 0到 500;
// 数据类型为12位无符号整数，取值范围为0~4095,其中0代表0,4095代表500,
//  0~4095中间的所有数值，按比例映射至0~500。
DEF_U16_STRONG_TYPE_GRADATION(MitKpCode_u12,        from_val,       
    uq16,   0,      500,    500.0/4095)

// kd: 0到 5;
// 数据类型为12位无符号整数，取值范围为0~4095,其中0代表0, 4095代表5,
//  0~4095中间的所有数值，按比例映射至0~5。
DEF_U16_STRONG_TYPE_GRADATION(MitKdCode_u12,        from_val,       
    uq16,   0,      5,      5.0/4095)

// t_f:-24到 24, 单位N-m;
// 数据类型为12位无符号整数，取值范围为0~4095,其中0代表-24,4095代表24,
//  0~4095中间的所有数值，按比例映射至-24~24。
DEF_U16_STRONG_TYPE_GRADATION(MitTorqueCode_u12,    from_nm,        
    iq16,   -24,      24,     24.0/4095)

struct [[nodiscard]] SpeedCode_i16{
    int16_t bits;

    constexpr iq8 to_dps() const {
        return bits;
    }
};

struct [[nodiscard]] SpeedLimitCode_u16{
    uint16_t bits;

    static constexpr SpeedLimitCode_u16 from_dps(const uq8 dps){
        return SpeedLimitCode_u16{static_cast<uint16_t>(dps)};
    }

    constexpr uq8 to_dps() const {
        return bits;
    }
};

struct [[nodiscard]] SpeedCtrlCode_i32{
    // 控制值 speedControl 为int32_t类型，对应实际转速为0.01dps/LSB,

    int32_t bits;
    static constexpr SpeedCtrlCode_i32 from_bits(const int32_t bits){
        return SpeedCtrlCode_i32{bits};
    }

    static constexpr  SpeedCtrlCode_i32 from_dps(const iq8 dps){
        return from_bits(static_cast<int32_t>(dps * 100));
    }

    [[nodiscard]] constexpr iq8 to_dps() const {
        return static_cast<iq8>(bits) / 100;
    }
};

struct [[nodiscard]] AccelCode_u32{
    uint32_t bits;

    static constexpr uq16 MAX_DPSS = 60000;
    static constexpr uq16 MIN_DPSS = 100;
    [[nodiscard]] static constexpr Result<AccelCode_u32, std::strong_ordering> 
    from_dpss(const uq16 dpss){
        if(dpss > MAX_DPSS) [[unlikely]]
            return Err(std::strong_ordering::greater);
        if(dpss < MIN_DPSS) [[unlikely]]
            return Err(std::strong_ordering::less);

        //1DPSS / LSB
        const uint32_t bits = static_cast<uint32_t>(dpss);
        return Ok(AccelCode_u32{bits});
    }
    [[nodiscard]] constexpr uint32_t to_dpss() const {
        return bits;
    }
};

struct [[nodiscard]] DegreeCode_i16{
    int16_t bits;

    [[nodiscard]] constexpr Angle<iq16> to_angle() const {
        return Angle<iq16>::from_degrees(bits);
    }
};

struct [[nodiscard]] PositionCode_i16{
    int16_t bits;

    constexpr Angle<iq16> to_angle() const {
        return Angle<iq16>::from_degrees(bits);
    }
};


struct [[nodiscard]] LapPosition_u16{
    uint16_t bits;
    constexpr Angle<uq16> to_angle() const {
        return Angle<uq16>::from_turns(uq16::from_bits(bits));
    }
};

struct [[nodiscard]] TemperatureCode_i8{
    int8_t bits;

    [[nodiscard]] int8_t to_celsius() const {
        return bits;
    }
};

struct [[nodiscard]] VoltageCode_u16{
    uint16_t bits;

    [[nodiscard]] uq16 to_volts() const {
        return uq16(0.1) * bits;
    }
};

struct [[nodiscard]] CurrentCode_i16{
    int16_t bits;

    [[nodiscard]] iq16 to_amps() const {
        return iq16(0.01) * bits;
    }
};


struct [[nodiscard]] PositionCode_i32{
    // 0.01degree/LSB
    int32_t bits;

    [[nodiscard]] Angle<iq16> to_angle() const {
        return Angle<iq16>::from_degrees(iq16(0.01) * bits);
    }
};

struct [[nodiscard]] LapPositionCode{
    // 0.01degree/LSB
    uint16_t bits;

    [[nodiscard]] Angle<uq16> to_angle() const {
        return Angle<uq16>::from_degrees(uq16(0.01) * bits);
    }
};

enum class CanAddr:uint8_t{};
struct [[nodiscard]] FaultStatus{
    uint16_t :1;
    uint16_t stall:1;
    uint16_t under_voltage:1;
    uint16_t over_voltage:1;

    uint16_t phase_over_current:1;
    uint16_t :1;
    uint16_t overdrive:1;
    uint16_t param_write:1;

    uint16_t overspeed:1;
    uint16_t :2;
    uint16_t ecs_overheat:1;

    uint16_t motor_overheat:1;
    uint16_t encoder_calibrate_failed:1;
    uint16_t encoder_data_incorrect:1;
    uint16_t :1;

    [[nodiscard]] constexpr uint16_t to_bits() const {
        return std::bit_cast<uint16_t>(*this);
    }

    [[nodiscard]] constexpr bool is_ok() const {
        return to_bits() == 0;
    }
};

enum class PidIndex:uint8_t{
    CurrentKp = 0x01,
    CurrentKi = 0x02,
    SpeedKp = 0x04,
    SpeedKi = 0x05,
    PositionKp = 0x07,
    PositionKi = 0x08,
    PositionKd = 0x09,
};

enum class PlanAccelKind:uint8_t{
    PositionAcc,
    PositionDec,
    SpeedAcc,
    SpeedDec,
};


enum class [[nodiscard]] Command:uint8_t{
    GetPidParameter = 0x30,
    GetPlanAccel = 0x42,
    SetPlanAccel = 0x43,
    GetStatus1 = 0x9A,
    GetStatus2 = 0x9c,
    GetStatus3 = 0x9d,
    ShutDown = 0x80,
    Stop = 0x81,
    SetTorque = 0xa1, 
    SetSpeed = 0xA2,

    BrakeOn = 0x78,
    BrakeOff = 0x79,
    GetRunMillis = 0xb1,
    GetSwVersion = 0xb2,
    SetOfflineTimeout = 0xb3,
    SetBaudrate = 0xb4,
    GetPackage = 0xb5
};

enum class Baudrate:uint8_t{
    RS485_115200 = 0,
    CAN_500K = 0,
    RS485_500K = 1,
    CAN_1M = 1
};

static constexpr size_t PAYLOAD_CAPACITY = 7;

struct [[nodiscard]] BytesFiller{
public:
    static constexpr size_t CAPACITY = PAYLOAD_CAPACITY;

    constexpr explicit BytesFiller(std::span<uint8_t, CAPACITY> bytes):
        bytes_(bytes){;}

    constexpr ~BytesFiller(){
        if(not is_full()) __builtin_abort();
    }

    constexpr __always_inline 
    void push_byte(const uint8_t byte){
        if(pos_ >= bytes_.size()) [[unlikely]] 
            on_overflow();
        bytes_[pos_++] = byte;
    }

    constexpr __always_inline 
    void push_zero(){
        push_byte(0);
    }

    constexpr __always_inline 
    void push_zeros(size_t n){
        #pragma GCC unroll(4)
        for(size_t i = 0; i < n; i++)
            push_byte(0);
    }

    constexpr __always_inline 
    void fill_remaining(const uint8_t byte){
        const size_t n = bytes_.size() - pos_;

        #pragma GCC unroll(4)
        for(size_t i = 0; i < n; i++){
            push_byte_unchecked(byte);
        }
    }

    template<size_t Extents>
    constexpr __always_inline 
    void push_bytes(const std::span<const uint8_t, Extents> bytes){
        if(pos_ + bytes.size() > bytes_.size()) [[unlikely]]
            on_overflow();
        push_bytes_unchecked(bytes);
    }

    constexpr __always_inline 
    void push_float(const float f_val){
        static_assert(sizeof(float) == 4);
        const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(float)>>(f_val);
        push_bytes(std::span(bytes));
    }

    template<typename T>
    requires (std::is_integral_v<T>)
    constexpr __always_inline 
    void push_int(const T i_val){
        const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(i_val);
        push_bytes(std::span(bytes));
    }


    [[nodiscard]] constexpr bool is_full() const {
        return pos_ == CAPACITY;
    }
private:
    std::span<uint8_t, CAPACITY> bytes_;
    size_t pos_ = 0;

    constexpr __always_inline 
    void push_byte_unchecked(const uint8_t byte){ 
        bytes_[pos_++] = byte;
    }

    template<size_t Extents>
    constexpr __always_inline 
    void push_bytes_unchecked(const std::span<const uint8_t, Extents> bytes){ 
        if constexpr(Extents == std::dynamic_extent){
            #pragma GCC unroll(4)
            for(size_t i = 0; i < bytes.size(); i++){
                push_byte(bytes[i]);
            }
        }else{
            #pragma GCC unroll(4)
            for(size_t i = 0; i < Extents; i++){
                push_byte(bytes[i]);
            }
        }
    }

    constexpr __always_inline void on_overflow(){
        __builtin_trap();
    }
};



struct [[nodiscard]] CommandHeadedDataField{
    Command cmd;
    std::array<uint8_t, PAYLOAD_CAPACITY> payload_bytes;

    static constexpr CommandHeadedDataField from_command_and_payload_bytes(
        const Command cmd,
        std::span<const uint8_t, BytesFiller::CAPACITY> payload_bytes
    ){
        CommandHeadedDataField ret;
        ret.cmd = cmd;
        std::copy(payload_bytes.begin(), payload_bytes.end(), ret.payload_bytes.begin());
        return ret;
    }

    static constexpr CommandHeadedDataField from_bytes(const std::span<uint8_t, BytesFiller::CAPACITY + 1> bytes){
        return from_command_and_payload_bytes(
            static_cast<Command>(bytes[0]),
            bytes.subspan<1, BytesFiller::CAPACITY>()
        );
    }
};

static_assert(sizeof(CommandHeadedDataField) == 1 + BytesFiller::CAPACITY);


namespace c2s_msg{

#define DEF_COMMAND_ONLY_C2S_MSG(cmd)\
struct [[nodiscard]] cmd{__always_inline constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {\
    BytesFiller(bytes).fill_remaining(0);}};

struct [[nodiscard]] GetPidParameter{
    PidIndex index;
    __always_inline constexpr void 
    fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_byte(std::bit_cast<uint8_t>(index));
        filler.fill_remaining(0);
    }
};


struct [[nodiscard]] GetPlanAccel{
    PlanAccelKind kind;
    __always_inline constexpr void 
    fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_byte(std::bit_cast<uint8_t>(kind));
        filler.fill_remaining(0);
    }
};

struct [[nodiscard]] SetPlanAccel{
    PlanAccelKind kind;
    AccelCode_u32 accel;
    __always_inline constexpr void 
    fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_byte(std::bit_cast<uint8_t>(kind));
        filler.push_zeros(2);
        filler.push_int<uint32_t>(accel.bits);
    }
};


DEF_COMMAND_ONLY_C2S_MSG(GetStatus1);
DEF_COMMAND_ONLY_C2S_MSG(GetStatus2);
DEF_COMMAND_ONLY_C2S_MSG(GetStatus3);
DEF_COMMAND_ONLY_C2S_MSG(ShutDown);

struct [[nodiscard]] SetTorque{
    CurrentCode_i16 q_current;
    __always_inline constexpr 
    void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_zeros(3);
        filler.push_int<int16_t>(q_current.bits);
        filler.push_zeros(2);
    };
};


struct [[nodiscard]] SetSpeed{
    Percentage<uint8_t> rated_current_ratio;
    SpeedCtrlCode_i32 speed;

    __always_inline constexpr void 
    fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_int<uint8_t>(rated_current_ratio.percents());
        filler.push_zeros(2);
        filler.push_int<int32_t>(speed.bits);
    };
};



struct SetPosition{
    SpeedLimitCode_u16 speed_limit;
    PositionCode_i32 abs_position;

    __always_inline constexpr void 
    fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_zeros(1);
        filler.push_int<uint16_t>(speed_limit.bits);
        filler.push_int<int32_t>(abs_position.bits);
    };
};


struct SetLapPosition{
    // 1.角度控制值angleControl 为 uint16_t 类型， 数值范围0~35999, 对应实际位置为
    // 0.01degree/LSB, 即实际角度范围0°~359.99°;
    //  2.spinDirection设置电机转动的方向， 为 uint8_t类型，0x00代表顺时针，0x01代表
    // 逆时针；
    // 3.maxSpeed限制了电机转动的最大速度，为uint16_t类型，对应实际转速1dps/LSB。
    LapPosition_u16 lap_position;
    bool is_ccw;
    SpeedLimitCode_u16 max_speed;

    __always_inline constexpr void 
    fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_int<uint8_t>(is_ccw);
        filler.push_int<uint16_t>(max_speed.bits);
        filler.push_int<uint16_t>(lap_position.bits);
        filler.push_zeros(2);
    };
};

// 该指令为控制指令，在电机没有故障的情况下可以运行该指令。主机发送该命令以
// 控制电机的位置(多圈角度), 若给定的电流大于堵转电流， 则不开启力控模式，
// 电机的最大转矩电流由上位机中的电机堵转电流值限制。
struct SetTorquePosition{
    // 控制值maxTorque限制了电机输出轴的最大扭矩，为uint8_t
    // 类型，取值范围为0~255, 以额定电流的百分比为单位， 即 1%*额定电流LSB
    Percentage<uint8_t> rated_current_ratio;

    // 控制值maxSpeed限制了电机输出轴转动的最大速度，为uint16t类型，
    // 对应实际转速1dps/LSB
    SpeedLimitCode_u16 max_speed;
    
    // 控制值angleControl为 int32t类型， 对应实际位置为0.01degree/LSB, 
    // 即 36000代表360° ,电机转动方向由目标位置和当前位置的差值决定
    PositionCode_i32 position;

    __always_inline constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = BytesFiller(bytes);
        filler.push_int<uint8_t>(rated_current_ratio.percents());
        filler.push_int<uint16_t>(max_speed.bits);
        filler.push_int<int32_t>(position.bits);
    };
};


struct MitParams{
    MitPositionCode_u16 position;
    MitSpeedCode_u12 speed;
    MitKpCode_u12 kp;
    MitKdCode_u12 kd;
    MitTorqueCode_u12 torque;

    __always_inline constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const {
        bytes[0] = static_cast<uint8_t>(position.to_bits() >> 8);
        bytes[1] = static_cast<uint8_t>(position.to_bits() & 0xff);
        bytes[2] = static_cast<uint8_t>(speed.to_bits() >> 4);
        bytes[3] = static_cast<uint8_t>(((speed.to_bits() & 0xf) << 4) | ((kp.to_bits() >> 8)));
        bytes[4] = static_cast<uint8_t>(kp.to_bits() & 0xff);
        bytes[5] = static_cast<uint8_t>(kd.to_bits() >> 4);
        bytes[6] = static_cast<uint8_t>(((kd.to_bits() & 0xf) << 4) | ((torque.to_bits() >> 8)));
        bytes[7] = static_cast<uint8_t>(torque.to_bits() & 0xf);
    };
};


#undef DEF_COMMAND_ONLY_C2S_MSG
};


enum class LoopWiring:uint8_t{
    Current,
    Speed,
    Position
};


namespace s2c_msg{

#define DEF_COMMAND_ONLY_S2C_MSG(cmd)\
struct [[nodiscard]] cmd{};
// struct cmd{constexpr CommandHeadedDataField fill_bytes(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {return CommandHeadedDataField::from_command(Command::cmd)};}

struct [[nodiscard]] GetPidParameter{
    using Self = GetPidParameter;
    PidIndex index;
    fp32 value;

    [[nodiscard]] static constexpr Self from_bytes(const std::span<const uint8_t, 7> bytes){
        return Self{
            .index = std::bit_cast<PidIndex>(bytes[0]),
            .value = le_bytes_ctor_bits(bytes.subspan<3, 4>())
        };
    }
};


struct [[nodiscard]] GetPlanAccel{
    using Self = GetPlanAccel;
    PlanAccelKind kind;
    AccelCode_u32 accel;
    [[nodiscard]] static constexpr Self from_bytes(const std::span<const uint8_t, 7> bytes){
        return Self{
            .kind = std::bit_cast<PlanAccelKind>(bytes[0]), 
            .accel = le_bytes_ctor_bits(bytes.subspan<3, 4>())
        };
    }
};


DEF_COMMAND_ONLY_S2C_MSG(GetStatus1);

DEF_COMMAND_ONLY_S2C_MSG(GetStatus2);

struct [[nodiscard]] GetStatus3{
    TemperatureCode_i8 motor_temperature;

    struct {
        CurrentCode_i16 a;
        CurrentCode_i16 b;
        CurrentCode_i16 c;
    }phase_current;
};

// 1.电机温度temperature (int8t类型，1Cstd::endian::little
//  2.电机的转矩电流值iq (int16_t类型， 0.01A/std::endian::little
//  3.电机输出轴转速speed (int16t类型， 1dps/std::endian::little
//  4.电机输出轴角度 (intl6t类型，1degree/LSB,最大范围±32767degree)。
template<typename Derived>
struct [[nodiscard]] _MotorStatusReport{
    using Self = Derived;
    TemperatureCode_i8 motor_temperature;
    CurrentCode_i16 q_current;
    SpeedCode_i16 axis_speed;
    DegreeCode_i16 axis_degrees;

    [[nodiscard]] static constexpr Derived from_bytes(const std::span<const uint8_t, 7> bytes){
        Derived ret;
        const auto exacter = make_bytes_ctor_bits_exacter<std::endian::little>(bytes);
        exacter.exact_to_elements(ret.motor_temperature, ret.q_current, ret.axis_speed, ret.axis_degrees);
        return ret;
    };
};

// 1.电机温度temperature (int8t类型，1Cstd::endian::little
//  2.电机的转矩电流值iq (int16_t类型， 0.01A/std::endian::little
//  3.电机输出轴转速speed (int16t类型， 1dps/std::endian::little
//  4.编码器位置值encoder(uint16 t类型，编码器的数值范围由编码器位数决定)
template<typename Derived>
struct [[nodiscard]] _MotorStatusReport2{
    using Self = Derived;
    TemperatureCode_i8 motor_temperature;
    CurrentCode_i16 q_current;
    SpeedCode_i16 axis_speed;
    LapPosition_u16 axis_lap_position;

    static constexpr Self from_bytes(const std::span<const uint8_t, 7> bytes){
        Self ret;
        const auto exacter = make_bytes_ctor_bits_exacter<std::endian::little>(bytes);
        exacter.exact_to_elements(ret.motor_temperature, ret.q_current, ret.axis_speed, ret.axis_lap_position);
        return ret;
    };
};

// 1.电机温度temperature (int8t类型，1Cstd::endian::little
//  2.电机的转矩电流值iq (int16_t类型， 0.01A/std::endian::little
//  3.电机输出轴转速speed (int16t类型， 1dps/std::endian::little
//  4.电机输出轴角度 (intl6t类型，1degree/LSB,最大范围±32767degree)。

struct [[nodiscard]] SetTorque:public _MotorStatusReport<SetTorque>{};
// 1.电机温度temperature (int8t类型， 1°Cstd::endian::little
//  2.电机的转矩电流值iq (intl6_t类型， 0.01A/std::endian::little
//  3.电机输出轴转速speed (int16t类型， 1dps/std::endian::little
//  4.电机输出轴角度 (intl6_t类型，1degree/LSB,最大范围±32767degree)。
struct [[nodiscard]] SetSpeed:public _MotorStatusReport<SetSpeed>{};


// 1.电机温度temperature (int8t类型， 1C/std::endian::little
//  2.电机的转矩电流值iq(int16t类型， 0.01A/std::endian::little
//  3.电机输出轴转速speed (int16_t类型， 1dps/std::endian::little
//  4.电机输出轴角度(intl6t类型，Idegree/LSB,最大范围±32767degree)。
struct [[nodiscard]] SetPosition:public _MotorStatusReport<SetPosition>{};


// 1.电机温度temperature (int8t类型，1Cstd::endian::little
//  2.电机的转矩电流值iq (int16_t类型， 0.01A/std::endian::little
//  3.电机输出轴转速speed (int16t类型， 1dps/std::endian::little
//  4.编码器位置值encoder(uint16 t类型，编码器的数值范围由编码器位数决定)
struct [[nodiscard]] SetLapPosition:public _MotorStatusReport2<SetLapPosition>{};

// 1.电机温度temperature (int8t类型， 1℃/std::endian::little
//  2.电机的转矩电流值iq(int16_t类型， 0.01A/std::endian::little
//  3.电机输出轴转速speed (int16_t类型， 1dps/std::endian::little
//  4.电机输出轴角度 (intl6_t类型，1degree/LSB,最大范围±32767degree)。
struct [[nodiscard]] DeltaPosition:public _MotorStatusReport<DeltaPosition>{};

// 1.电机温度temperature (int8t类型， 1℃/std::endian::little
//  2.电机的转矩电流值iq(int16_t类型， 0.01A/std::endian::little
//  3.电机输出轴转速speed (int16_t类型， 1dps/std::endian::little
//  4.电机输出轴角度 (intl6_t类型，1degree/LSB,最大范围±32767degree)。
struct [[nodiscard]] SetTorquePosition:public _MotorStatusReport<SetTorquePosition>{};


struct [[nodiscard]] GetPackage{
    using Self = GetPackage;
    static constexpr size_t MAX_STR_LENGTH = PAYLOAD_CAPACITY;
    char str[MAX_STR_LENGTH];

    static constexpr Self from_bytes(const std::span<const uint8_t, PAYLOAD_CAPACITY> bytes){
        Self self;
        for(size_t i = 0; i < MAX_STR_LENGTH; i++){
            self.str[i] = bytes[i];
        }
        return self;
    }

    friend OutputStream& operator<<(OutputStream & os, const Self & self){ 
        return os << StringView(self.str, MAX_STR_LENGTH);
    }
};

DEF_COMMAND_ONLY_S2C_MSG(ShutDown);




struct [[nodiscard]] MitParams{
    using Self = MitParams;
    CanAddr can_addr;
    MitPositionCode_u16 position;
    MitSpeedCode_u12 speed;
    MitTorqueCode_u12 torque;

    constexpr Self from_bytes(std::span<const uint8_t, 8> bytes) const {
        const uint8_t can_addr_bits = 
            bytes[0];
        const uint16_t position_bits = 
            (bytes[1] << 8) | bytes[2];
        const uint16_t speed_bits = 
            (bytes[3] << 4) | (bytes[4] >> 4);
        const uint16_t torque_bits = 
            ((bytes[4] & 0x0f) << 8) | (bytes[5]);
        return Self{
            .can_addr = CanAddr(can_addr_bits),
            .position = MitPositionCode_u16::from_bits(position_bits),
            .speed = MitSpeedCode_u12::from_bits(speed_bits),
            .torque = MitTorqueCode_u12::from_bits(torque_bits)
        };
    };
};


};



#if 1
[[maybe_unused]] static void static_test(){
    static constexpr auto bytes = std::to_array<uint8_t>({0x12, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07});
    static constexpr s2c_msg::SetTorque obj = s2c_msg::SetTorque::from_bytes(std::span(bytes));
    static_assert(obj.motor_temperature.bits == 0x12);
    // static constexpr auto it = le_bytes_into_bits(std::span(bytes));
    // static constexpr PositionCode_i32 temp = (it);
}
#endif

}


using namespace ymd;

using namespace drivers;

template<typename T>
__always_inline constexpr auto make_bytes(T && msg){

    std::array<uint8_t, 7> bytes;
    msg.fill_bytes(bytes);
    return bytes;
}

__no_inline constexpr auto make_bytes1(){
    return make_bytes(
        myact::c2s_msg::SetTorquePosition{
            .rated_current_ratio = Percentage<uint8_t>::from_percents_unchecked(30),
            .max_speed = myact::SpeedLimitCode_u16{2},
            .position = myact::PositionCode_i32{0x7fff}
        }
    );
}

__no_inline constexpr auto make_bytes2(){
    return make_bytes(
        myact::c2s_msg::SetTorquePosition{
            .rated_current_ratio = Percentage<uint8_t>::from_percents_unchecked(3),
            .max_speed = myact::SpeedLimitCode_u16{2},
            .position = myact::PositionCode_i32{0x7fff}
        }
    );
}
void myactuator_main(){
    auto & DBG_UART = DEBUGGER_INST;
    DBG_UART.init({576000});

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    DEBUG_PRINTLN(make_bytes1());
    DEBUG_PRINTLN(make_bytes2());
    auto & os = DEBUGGER;
    os.field("enabled")(make_bytes2());
    PANIC{};
    while(true);
}

