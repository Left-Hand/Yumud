#include "../hightorque_primitive.hpp"
#include "../hightorque_slots.hpp"
#include "../hightorque_serialize.hpp"

#include <compare>

using namespace ymd;
using namespace robots::hightorque;

using namespace utils;

static constexpr bool is_nearly_equal(float a, float b, float epsilon = 0.0001f){
    return std::abs(a - b) < epsilon;
}


namespace {


[[maybe_unused]] static void test_ser(){
    {
        // 1.2.1 普通模式 (位置和速度不能同时控制)
        constexpr auto bytes = []{
            std::array<uint8_t, 8> buf;
            SlotFiller{
                SlotCommand::Write, 
                RegAddr{0x07}
            }.fill_bytes_from_elements(
                std::span(buf), 
                PositionCode{0},
                SpeedCode::nil(),
                TorqueCode{0}
            );

            return buf;
        }();

        static_assert(bytes[0] == 0x07);
        static_assert(bytes[1] == 0x07);
        static_assert(bytes[2] == 0x00);
        static_assert(bytes[3] == 0x00);
        static_assert(bytes[4] == 0x00);
        static_assert(bytes[5] == 0x80);
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);
    }

    {
        // 1.2.2 力矩模式
        constexpr auto bytes = []{
            std::array<uint8_t, 4> buf;
            SlotFiller{
                SlotCommand::Write, 
                RegAddr{0x13}
            }.fill_bytes_from_elements(
                std::span(buf), 
                TorqueCode{0}
            );

            return buf;
        }();

        static_assert(bytes[0] == 0x05);
        static_assert(bytes[1] == 0x13);
        static_assert(bytes[2] == 0x00);
        static_assert(bytes[3] == 0x00);
    }

    {
        // 1.2.3 位置速度最大力矩模式
        constexpr auto bytes = []{
            std::array<uint8_t, 8> buf;
            SlotFiller{
                SlotCommand::Write, 
                RegAddr{0x35}
            }.fill_bytes_from_elements(
                std::span(buf), 
                PositionCode::try_from_angle(Angular<iq16>::from_turns(0.5_iq16)).unwrap(),
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
        static_assert(bytes[5] == 0x00);
        static_assert(bytes[6] == 0x00);
        static_assert(bytes[7] == 0x00);
    }



    {
        constexpr auto bytes = []{
            std::array<uint8_t, 3> buf;
            SlotFiller{
                SlotCommand::Write, 
                RegAddr{0x00}

            }.fill_bytes_from_elements(
                std::span(buf), 
                Mode::DqVoltage
            );

            return buf;
        }();

        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == static_cast<uint8_t>(Mode::DqVoltage));
    }


    {
        constexpr auto bytes = []{
            std::array<uint8_t, 3> buf;
            SlotFiller{
                SlotCommand::Write, 
                RegAddr{0x00}

            }.fill_bytes_from_elements(
                std::span(buf), 
                Mode::DqCurrent
            );

            return buf;
        }();

        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x00);
        static_assert(bytes[2] == static_cast<uint8_t>(Mode::DqCurrent));
    }
}
}