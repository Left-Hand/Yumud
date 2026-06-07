#include "../jvci_primitive.hpp"
#include "../jvci_framefactory.hpp"
#include "core/math/realmath.hpp"

using namespace ymd;
using namespace ymd::robots::jvci;


namespace {
[[maybe_unused]] static void test_degrees(){
    using namespace utils;
    {
        static constexpr auto f_1 = (degree001_to_turns(360 * 100));
        static constexpr auto f_1000 = (degree001_to_turns(1000 * 360 * 100));

        static constexpr auto f_30000 = (degree001_to_turns(30000 * 360 * 100));
        static constexpr auto f_n30000 = (degree001_to_turns(-30000 * 360 * 100));

        static constexpr auto f_32700 = (degree001_to_turns(32700 * 360 * 100));
        static constexpr auto f_n32700 = (degree001_to_turns(-32700 * 360 * 100));

        static constexpr auto f_32767 = (degree001_to_turns(32767 * 360 * 100));
        static constexpr auto f_n32767 = (degree001_to_turns(-32767 * 360 * 100));

        static_assert((float)math::abs(f_1 - 1.0_iq16) < 1E-4);
        static_assert((float)math::abs(f_1000 - 1000.0_iq16) < 1E-4);
        static_assert((float)math::abs(f_30000 - 30000.0_iq16) < 1E-4);
        static_assert((float)math::abs(f_n30000 - -30000.0_iq16) < 1E-4);

        static_assert((float)math::abs(f_32700 - 32700.0_iq16) < 1E-4);
        static_assert((float)math::abs(f_n32700 - -32700.0_iq16) < 1.2E-4);

        static_assert((float)math::abs(f_32767 - 32767.0_iq16) < 1E-4);
        static_assert((float)math::abs(f_n32767 - -32767.0_iq16) < 1.2E-4);
    }

    {
        auto abs_i32 = [](int32_t a){
            // return (a > b) ? (a - b) : (b - a);
            if(a > 0) return a;
            return -a;
        };

        // Test reverse conversion: turns -> degree001
        static constexpr auto b_1 = turns_to_degree001(1.0_iq16);
        static constexpr auto b_1000 = turns_to_degree001(1000.0_iq16);
        static constexpr auto b_30000 = turns_to_degree001(30000.0_iq16);
        static constexpr auto b_n30000 = turns_to_degree001(-30000.0_iq16);
        static constexpr auto b_32700 = turns_to_degree001(32700.0_iq16);
        static constexpr auto b_n32700 = turns_to_degree001(-32700.0_iq16);

        static constexpr auto b_32767 = turns_to_degree001(32767.0_iq16);
        static constexpr auto b_n32767 = turns_to_degree001(-32767.0_iq16);

        static_assert(abs_i32(b_1 - 360 * 100) < 2);
        static_assert(abs_i32(b_1000 - 1000 * 360 * 100) < 2);
        static_assert(abs_i32(b_30000 - 30000 * 360 * 100) < 6);
        static_assert(abs_i32(b_n30000 - (-30000 * 360 * 100)) < 6);

        static_assert(abs_i32(b_32700 - 32700 * 360 * 100) < 6);
        static_assert(abs_i32(b_n32700 - (-32700 * 360 * 100)) < 5);

        static_assert(abs_i32(b_32767 - 32767 * 360 * 100) < 3);
        static_assert(abs_i32(b_n32767 - (-32767 * 360 * 100)) < 5);
    }

}

// 测试强类型转换
[[maybe_unused]] static void test_strong_types() {
    {
        // PositionCode 正值测试
        static constexpr auto p1 = PositionCode::from_degrees(360.5f);
        static_assert(std::abs(p1.to_degrees() - 360.5f) < 0.01f);
    }
    {
        // PositionCode 负值测试
        static constexpr auto p2 = PositionCode::from_degrees(-114.514f);
        static_assert(std::abs(p2.to_degrees() - (-114.514f)) < 0.01f);
    }


    {
        // SpeedCode 正值测试
        static constexpr auto s1 = SpeedCode::from_rpm_bounded(500.23f);
        static constexpr auto s2 = SpeedCode::from_rpm_bounded(500.23_iq16);
        static constexpr auto s3 = SpeedCode::from_rpm_bounded(9999.23_iq16);

        static_assert(std::abs(s1.to_rpm<float>() - 500.23f) < 0.02f);
        static_assert(std::abs(s2.to_rpm<float>() - 500.23f) < 0.02f);
        static_assert(std::abs(s3.to_rpm<float>() - 9999.23f) < 0.02f);

        static_assert(std::abs(s1.to_rpm<iq16>() - 500.23_iq16) < 0.02_iq16);
        static_assert(std::abs(s2.to_rpm<iq16>() - 500.23_iq16) < 0.02_iq16);
        static_assert(std::abs(s3.to_rpm<iq16>() - 9999.23_iq16) < 0.02_iq16);
    }

    {
        // SpeedCode 正值测试
        static constexpr auto s1 = SpeedCode::from_tps_bounded(1.0_iq16);
        static constexpr auto s2 = SpeedCode::from_tps_bounded(iq16(500.23 / 60));
        static constexpr auto s3 = SpeedCode::from_tps_bounded(iq16(9999.23 / 60));

        static_assert(s1.bits == SpeedCode::from_tps_bounded(1.0_iq24).bits);
        static_assert(std::abs(s1.to_tps<float>() - 1.00f) < 0.02f);
        static_assert(std::abs(s2.to_tps<float>() - 500.23f / 60) < 0.02f);
        static_assert(std::abs(s3.to_tps<float>() - 9999.23f / 60) < 0.02f);
    }


    {
        // SpeedCode 负值测试
        static constexpr auto s1 = SpeedCode::from_rpm_bounded(-500.23f);
        static constexpr auto s2 = SpeedCode::from_rpm_bounded(-500.23_iq16);
        static constexpr auto s3 = SpeedCode::from_rpm_bounded(-9999.23_iq16);
        static_assert(std::abs(s1.to_rpm<float>() - (-500.23f)) < 0.02f);
        static_assert(std::abs(s2.to_rpm<float>() - (-500.23f)) < 0.02f);
        static_assert(std::abs(s3.to_rpm<float>() - -9999.23f) < 0.02f);
    }


    {
        // BusbarVoltageCode 测试
        static constexpr auto v1 = BusbarVoltageCode::from_volt(12.3f);
        static_assert(std::abs(v1.to_volt() - 12.3f) < 0.01f);
    }
    {
        // TorqueCode 测试
        static constexpr auto t1 = TorqueCode::from_newton_meter_bounded(0.2f);
        static_assert(std::abs(t1.to_newton_meter() - 0.2f) < 0.01f);
    }
}

}


