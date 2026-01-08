#include "hightorque_primitive.hpp"
#include "hightorque_utils.hpp"
#include "hightorque_slots.hpp"

using namespace ymd;
using namespace robots::hightorque;

using namespace primitive;
using namespace utils;

static constexpr bool is_nearly_equal(float a, float b, float epsilon = 0.0001f){
    return std::abs(a - b) < epsilon;
}


namespace {

static_assert(element_type_v<uint8_t> == ElementType::B1);
static_assert(element_type_v<uint16_t> == ElementType::B2);
static_assert(element_type_v<uint32_t> == ElementType::B4);
static_assert(element_type_v<float> == ElementType::Float);
static_assert(element_type_v<CurrentCode> == ElementType::B2);
static_assert(element_type_v<PositionCode> == ElementType::B2);

static_assert(PositionCode::from_angle(Angular<iq16>::from_turns(0.5_iq16)).bits == 5000);
static_assert(PositionCode::from_angle(Angular<iq16>::from_turns(0.25_iq16)).bits == 2500);
static_assert(PositionCode{.bits = 32767}.bits == 0x7fff);


static_assert(common_element_type_v<int16_t, int16_t> == ElementType::B2);
static_assert(common_element_type_v<int8_t, int8_t> == ElementType::B1);
static_assert(common_element_type_v<PositionCode, SpeedCode> == ElementType::B2);

static_assert(make_slot_specifier<SpeedCode, PositionCode, TorqueCode>(SlotCommand::Write).to_bits() == 0x07);


[[maybe_unused]] static void position_code_test(){
    {
        constexpr auto i16_max = std::numeric_limits<int16_t>::max();
        constexpr auto turns1 = static_cast<float>(PositionCode{.bits = i16_max}.to_angle().to_turns());
        static_assert(is_nearly_equal(turns1, i16_max / 10000.0f));
    }

    {
        constexpr auto i16_min = std::numeric_limits<int16_t>::min();
        constexpr auto turns2 = static_cast<float>(PositionCode{.bits = i16_min}.to_angle().to_turns());
        static_assert(is_nearly_equal(turns2, i16_min / 10000.0f));
    }
}

[[maybe_unused]] static void speed_code_test(){
    // 速度：单位 0.00025 转/秒，如 val = 1000 表示 0.25 转/秒

    {
        constexpr auto i16_max = std::numeric_limits<int16_t>::max();
        constexpr auto turns1 = static_cast<float>(SpeedCode{.bits = i16_max}.to_angular_speed().to_turns());
        static_assert(is_nearly_equal(turns1, i16_max / 4000.0f));
    }

    {
        constexpr auto i16_min = std::numeric_limits<int16_t>::min();
        constexpr auto turns2 = static_cast<float>(SpeedCode{.bits = i16_min}.to_angular_speed().to_turns());
        static_assert(is_nearly_equal(turns2, i16_min / 4000.0f));
    }
}

[[maybe_unused]] static void accerlation_code_test(){
    // 加速度：单位 0.01 转/秒^2，如 acc = 40 表示 0.4 转/秒^2

    {
        constexpr auto i16_max = std::numeric_limits<int16_t>::max();
        constexpr auto turns1 = static_cast<float>(AccelerationCode{.bits = i16_max}.to_angular_acceleration().to_turns());
        static_assert(is_nearly_equal(turns1, i16_max / 100.0f));
    }

    {
        constexpr auto i16_min = std::numeric_limits<int16_t>::min();
        constexpr auto turns2 = static_cast<float>(AccelerationCode{.bits = i16_min}.to_angular_acceleration().to_turns());
        static_assert(is_nearly_equal(turns2, i16_min / 100.0f));
    }
}

[[maybe_unused]] static void torque_code_test(){
    // 扭矩：单位 0.00025 N*m，如 val = 1000  torque = 0.25 N*m

    // {
    //     constexpr auto i16_max = std::numeric_limits<int16_t>::max();
    //     constexpr auto torque1 = static_cast<float>(TorqueCode{.bits = i16_max}.to_torque().to_newton_meters());
    // }
}


[[maybe_unused]] static void test_ser(){
    {
        constexpr auto bytes = []{
            std::array<uint8_t, 8> buf;
            SlotBuilder{
                SlotCommand::Write, 
                RegAddr{0x35}
            }.build(
                std::span(buf), 
                PositionCode::from_angle(Angular<iq16>::from_turns(0.5_iq16)),
                SpeedCode{0},
                TorqueCode{0}
            );

            return buf;
        }();

        static_assert(bytes[0] == 0x07);
        static_assert(bytes[1] == 0x35);
        static_assert(bytes[2] == 5000 % 256);
        static_assert(bytes[3] == 5000 / 256);
        static_assert(bytes[4] == 0x00);
    }

    {
        constexpr auto bytes = []{
            std::array<uint8_t, 3> buf;
            SlotBuilder{
                SlotCommand::Write, 
                RegAddr{0x00}

            }.build(
                std::span(buf), 
                Mode::DqVoltage
            );

            return buf;
        }();

        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == static_cast<uint8_t>(Mode::DqVoltage));
    }
}
}