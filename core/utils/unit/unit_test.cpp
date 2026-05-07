#include "unit.hpp"

using namespace ymd;
using namespace ymd::unit;


namespace {

[[maybe_unused]] void test_units(){
    {
        [[maybe_unused]] static constexpr auto l1 = 1_mm;
        [[maybe_unused]] static constexpr auto l2 = MilliMeter<float>(1);
        [[maybe_unused]] static constexpr auto t1 = 1_s;

        [[maybe_unused]] static constexpr auto v1 = l1 / t1;
        [[maybe_unused]] static constexpr auto v2 = 1_m_s;
        [[maybe_unused]] static constexpr auto v3 = v1 + v2;
        [[maybe_unused]] static constexpr auto v4 = v2 + v1;

        static_assert(l1 == l2);
        static_assert(sizeof(l1) == sizeof(float));
    }

    {

        [[maybe_unused]] static constexpr auto force1 = 2.5_kn + 300_n;      // 2800 N
        [[maybe_unused]] static constexpr auto energy1 = 1.8_J * 5;          // 9 J
        [[maybe_unused]] static constexpr auto power1 = energy1 / 2_s;       // 4.5 W
        [[maybe_unused]] static constexpr auto torque1 = 150_n * 0.3_m; 
        [[maybe_unused]] static constexpr auto work = 5_n * 2_m;        // 10_J (energy)
        [[maybe_unused]] static constexpr auto rotation = 3.14_r;
        [[maybe_unused]] static constexpr auto torque = 150_n * 0.3_m / 1_r; // 45_Nm (torque)
        [[maybe_unused]] static constexpr auto dist = 1_km;
        [[maybe_unused]] static constexpr auto volt = 1_W / 1_A;

        static_assert(sizeof(force1) == sizeof(float));

    }

    {
        [[maybe_unused]] static constexpr auto L = 100_mH;
        [[maybe_unused]] static constexpr auto dI = 2_A;
        [[maybe_unused]] static constexpr auto dt = 50_ms;
        [[maybe_unused]] static constexpr auto V = L * dI / dt;  // 100mH * 2A / 50ms = 4V

        // Magnetic flux calculation
        [[maybe_unused]] static constexpr auto B = 1.5_T;
        [[maybe_unused]] static constexpr auto A = 0.01_m * 0.02_m; // 0.0002m²
        [[maybe_unused]] static constexpr auto phi = B * A;         // 0.0003 Wb

        // Inductive reactance XL = 2πfL
        [[maybe_unused]] static constexpr auto f = 50_ghz; // 1Hz = 1/s
        [[maybe_unused]] static constexpr auto XL = f * L * (M_PI); // ~31.4159 Ω
    }
}


}