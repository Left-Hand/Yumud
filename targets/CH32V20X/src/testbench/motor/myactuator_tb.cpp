#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/utils/bytes_spawner.hpp"
#include "core/utils/bits_caster.hpp"
#include "core/utils/percentage.hpp"
#include "core/utils/from_bits_debinder.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "types/transforms/Basis.hpp"

#include "robots/vendor/DJI/M3508/m3508.hpp"
#include "robots/vendor/DJI/DR16/DR16.hpp"
#include "hal/gpio/gpio_port.hpp"

#if 1
namespace ymd::drivers::myact { 

struct [[nodiscard]]SpeedCode_i16{
    int16_t bits;

    constexpr iq8 to_dps() const {
        return bits;
    }
};

struct [[nodiscard]]MaxSpeedCode_u16{
    uint16_t bits;

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

struct [[nodiscard]] DegreeCode_i16{
    int16_t bits;

    constexpr Angle<iq16> to_angle() const {
        return Angle<iq16>::from_degrees(bits);
    }
};

struct [[nodiscard]] PositionCode_i16{
    int16_t bits;

    constexpr Angle<iq16> to_angle() const {
        return Angle<iq16>::from_degrees(bits);
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

    [[nodiscard]] constexpr uint16_t as_bits() const {
        return std::bit_cast<uint16_t>(*this);
    }

    [[nodiscard]] constexpr bool is_ok() const {
        return as_bits() == 0;
    }
};


enum class [[nodiscard]] Command:uint8_t{
    GetStatus1 = 0x9A,
    GetStatus2 = 0x9c,
    GetStatus3 = 0x9d,
    ShutDown = 0x80,
    Stop = 0x81,
    SetTorque = 0xa1, 
    SetSpeed = 0xA2
};

static constexpr size_t PAYLOAD_CAPACITY = 7;

struct [[nodiscard]] PayloadFiller{
public:
    static constexpr size_t CAPACITY = PAYLOAD_CAPACITY;

    constexpr explicit PayloadFiller(std::span<uint8_t, CAPACITY> bytes):
        bytes_(bytes){;}

    constexpr ~PayloadFiller(){
        if(not is_full()) __builtin_abort();
    }

    constexpr inline void push_byte(const uint8_t byte){
        if(size_ >= bytes_.size()) [[unlikely]] __builtin_trap();
        bytes_[size_++] = byte;
    }

    constexpr inline void push_zero(){
        push_byte(0);
    }

    constexpr inline void push_zeros(size_t n){
        for(size_t i = 0; i < n; i++) push_byte(0);
    }

    constexpr inline void fill_remaining(const uint8_t byte){
        const size_t n = bytes_.size() - size_;
        for(size_t i = 0; i < n; i++){
            push_byte(byte);
        }
    }

    constexpr inline void push_bytes(const std::span<const uint8_t> bytes){
        for(const auto byte : bytes){
            push_byte(byte);
        }
    }

    constexpr inline void push_float(const float f){
        static_assert(sizeof(float) == 4);
        const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(float)>>(f);
        push_bytes(std::span(bytes));
    }

    template<typename T>
    requires (std::is_integral_v<T>)
    constexpr inline void push_int(const T i){
        const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(i);
        push_bytes(std::span(bytes));
    }


    [[nodiscard]] constexpr bool is_full() const {
        return size_ == CAPACITY;
    }


private:
    std::span<uint8_t, CAPACITY> bytes_;
    size_t size_ = 0;
};



struct [[nodiscard]] DataField{
    Command cmd;
    std::array<uint8_t, PAYLOAD_CAPACITY> payload_bytes;

    static constexpr DataField from_command_and_payload_bytes(
        const Command cmd,
        std::span<const uint8_t, PayloadFiller::CAPACITY> payload_bytes
    ){
        DataField ret;
        ret.cmd = cmd;
        std::copy(payload_bytes.begin(), payload_bytes.end(), ret.payload_bytes.begin());
        return ret;
    }

    static constexpr DataField from_bytes(const std::span<uint8_t, PayloadFiller::CAPACITY + 1> & bytes){
        return from_command_and_payload_bytes(
            static_cast<Command>(bytes[0]),
            bytes.subspan<1, PayloadFiller::CAPACITY>()
        );
    }
};

static_assert(sizeof(DataField) == 1 + PayloadFiller::CAPACITY);


namespace req_msg{

#define DEF_COMMAND_ONLY_REQ_MSG(cmd)\
struct cmd{constexpr void fill_payload(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {\
    PayloadFiller(bytes).fill_remaining(0);}};

DEF_COMMAND_ONLY_REQ_MSG(GetStatus1);
DEF_COMMAND_ONLY_REQ_MSG(GetStatus2);
DEF_COMMAND_ONLY_REQ_MSG(GetStatus3);
DEF_COMMAND_ONLY_REQ_MSG(ShutDown);

struct SetTorque{
    CurrentCode_i16 q_current;
    constexpr void fill_payload(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = PayloadFiller(bytes);
        filler.push_zeros(3);
        filler.push_int(q_current.bits);
        filler.push_zeros(2);
    };
};


struct SetSpeed{
    Percentage<uint8_t> rated_current_ratio;
    SpeedCtrlCode_i32 speed;

    constexpr void fill_payload(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {
        auto filler = PayloadFiller(bytes);
        filler.push_int(rated_current_ratio.percents());
        filler.push_zeros(2);
        filler.push_int(speed.bits);
    };
};

struct SetPosition{
    MaxSpeedCode_u16 speed_limit;
    
};

#undef DEF_COMMAND_ONLY_REQ_MSG
};





namespace resp_msg{

#define DEF_COMMAND_ONLY_RESP_MSG(cmd)\
struct cmd{}
// struct cmd{constexpr DataField fill_payload(std::span<uint8_t, PAYLOAD_CAPACITY> bytes) const {return DataField::from_command(Command::cmd)};}



struct GetStatus1{

};

struct GetStatus2{

};

struct GetStatus3{
    TemperatureCode_i8 motor_temperature;

    struct {
        CurrentCode_i16 a;
        CurrentCode_i16 b;
        CurrentCode_i16 c;
    }phase_current;
};

#define SPAWNER_FETCH_CTOR_BITS spn.fetch_leading_ctor_bits<LSB>()




template<typename Derived>
struct MotorStatusResp{
    using Self = Derived;
    TemperatureCode_i8 motor_temperature;
    CurrentCode_i16 q_current;
    SpeedCode_i16 axis_speed;
    DegreeCode_i16 axis_degrees;

    [[nodiscard]] static constexpr Derived from_bytes(const std::span<const uint8_t, 7> bytes){
        Derived ret;
        const auto exacter = make_bytes_ctor_bits_exacter<LSB>(bytes);
        exacter.exact_to_elements(ret.motor_temperature, ret.q_current, ret.axis_speed, ret.axis_degrees);
        return ret;
    };
};

struct [[nodiscard]] SetTorque:public MotorStatusResp<SetTorque>{};
struct [[nodiscard]] SetSpeed:public MotorStatusResp<SetSpeed>{};
struct [[nodiscard]] SetPosition:public MotorStatusResp<SetSpeed>{};


struct MotorStatusResp2{
    using Self = MotorStatusResp2;
    TemperatureCode_i8 motor_temperature;
    CurrentCode_i16 q_current;
    SpeedCode_i16 axis_speed;
    DegreeCode_i16 axis_degrees;

    static constexpr Self from_bytes(const std::span<const uint8_t, 7> bytes){
        Self ret;
        const auto exacter = make_bytes_ctor_bits_exacter<LSB>(bytes);
        exacter.exact_to_elements(ret.motor_temperature, ret.q_current, ret.axis_speed, ret.axis_degrees);
        return ret;
    };
};

DEF_COMMAND_ONLY_RESP_MSG(ShutDown);


};



#if 1
[[maybe_unused]] static void static_test(){
    static constexpr auto bytes = std::to_array<uint8_t>({0x12, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07});
    static constexpr resp_msg::SetTorque obj = resp_msg::SetTorque::from_bytes(std::span(bytes));
    static_assert(obj.motor_temperature.bits == 0x12);
    // static constexpr auto it = le_bytes_into_bits(std::span(bytes));
    // static constexpr PositionCode_i32 temp = (it);
}
#endif

}

#endif