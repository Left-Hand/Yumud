#include "crsf_ser.hpp"
#include "../crsf_primitive.hpp"
#include "../crsf_packed_code.hpp"

using namespace ymd;
using namespace ymd::crsf;

namespace {


[[maybe_unused]] static void test_ser_funcs(){
    {
        [[maybe_unused]] static constexpr std::array<uint8_t, 4> obj{0x01, 0x02, 0x03, 0x04};
        auto fn = []{
            std::array<uint8_t, 8> bytes;
            const auto res = SerialzeFunctions<CheckedPolicy>::ser_0xff_terminated_uchars(std::span(bytes), std::span(obj));
            return std::make_tuple(bytes, res);
        };

        static constexpr auto bytes = std::get<0>(fn());
        static constexpr auto res = std::get<1>(fn());

        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x02);
        static_assert(bytes[4] == 0xff);
        static_assert(res.is_ok());
        static_assert(res.unwrap() == 5);
    }

    {
        [[maybe_unused]] static constexpr std::array<uint8_t, 4> obj{0x01, 0x02, 0x03, 0x04};
        auto fn = []{
            std::array<uint8_t, 8> bytes;
            const auto res = SerialzeFunctions<CheckedPolicy>::ser_zero_terminated_uchars(std::span(bytes), std::span(obj));
            return std::make_tuple(bytes, res);
        };

        static constexpr auto bytes = std::get<0>(fn());
        static constexpr auto res = std::get<1>(fn());

        static_assert(bytes[0] == 0x01);
        static_assert(bytes[1] == 0x02);
        static_assert(bytes[4] == 0x00);
        static_assert(res.is_ok());
        static_assert(res.unwrap() == 5);
    }

    {
        [[maybe_unused]] static constexpr std::array<uint8_t, 4> obj{0x01, 0x02, 0x03, 0x04};
        auto fn = []{
            std::array<uint8_t, 4> bytes;
            const auto res = SerialzeFunctions<CheckedPolicy>::ser_zero_terminated_uchars(std::span(bytes), std::span(obj));
            return std::make_tuple(bytes, res);
        };
        static constexpr auto res = std::get<1>(fn());

        static_assert(res.is_err());
    }

    {
        [[maybe_unused]] static constexpr uint32_t obj = 0x12345678;
        auto fn = []{
            std::array<uint8_t, 4> bytes;
            const auto res = SerialzeFunctions<CheckedPolicy>::ser_be_int<uint32_t>(std::span(bytes), obj);
            return std::make_tuple(bytes, res);
        };

        static constexpr auto bytes = std::get<0>(fn());
        static constexpr auto res = std::get<1>(fn());

        static_assert(res.is_ok());
        static_assert(res.unwrap() == 4);
        static_assert(bytes[0] == 0x12);
        static_assert(bytes[1] == 0x34);
        static_assert(bytes[2] == 0x56);
        static_assert(bytes[3] == 0x78);
    }

    {
        [[maybe_unused]] static constexpr auto obj = VoltageCode{.bits = 0x1234};
        static_assert(sizeof(obj) == 2);
        static_assert(std::is_same_v<tmp::to_bits_t<std::decay_t<decltype(obj)>>, int16_t> );
        auto fn = []{
            std::array<uint8_t, 4> bytes;
            const auto res = SerialzeFunctions<CheckedPolicy>::ser_bits_intoable(std::span(bytes), obj);
            return std::make_tuple(bytes, res);
        };

        static constexpr auto bytes = std::get<0>(fn());
        static constexpr auto res = std::get<1>(fn());

        static_assert(res.is_ok());
        static_assert(res.unwrap() == 2);
        static_assert(bytes[0] == 0x12);
        static_assert(bytes[1] == 0x34);
    }
}

[[maybe_unused]] void test_ser_receiver(){
    {
        [[maybe_unused]] static constexpr std::array<uint8_t, 4> obj{0x00, 0x07, 0x02, 0x01};
        auto fn = []{
            std::array<uint8_t, 5> bytes{0xff, 0xff, 0xff, 0xff, 0xff};
            auto receiver = SerializeReceiver{std::span(bytes), 0};
            receiver.push_zero_terminated_uchars(std::span(obj)).unwrap();
            return std::make_tuple(bytes, receiver);
        };

        static constexpr auto bytes = std::get<0>(fn());
        static_assert(bytes[0] == 0x00);
        static_assert(bytes[1] == 0x07);
        static_assert(bytes[2] == 0x02);
        static_assert(bytes[3] == 0x01);
        static_assert(bytes[4] == 0x00);

    }
}
}