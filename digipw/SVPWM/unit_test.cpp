#include "svpwm3.hpp"

using namespace ymd;
using namespace ymd::digipw;

namespace{

// 测试零输入
[[maybe_unused]] void test_zero_input(){
    {
        static constexpr auto uvw = SVM({.alpha = iq16(0), .beta = 0});
        static constexpr float u = (float)uvw.u;
        static constexpr float v = (float)uvw.v;
        static constexpr float w = (float)uvw.w;

        // 零输入时，所有占空比应该等于0.5
        static_assert(std::abs(u - 0.5) < 1E-4);
        static_assert(std::abs(v - 0.5) < 1E-4);
        static_assert(std::abs(w - 0.5) < 1E-4);
    }
}

// 原始测试：alpha = sqrt(3)/2, beta = 0 (30度)
[[maybe_unused]] void test_full(){
    {
        static constexpr auto uvw = SVM({.alpha = iq16(SQRT3 / 2), .beta = 0});
        static constexpr float u = (float)uvw.u;
        static constexpr float v = (float)uvw.v;
        static constexpr float w = (float)uvw.w;

        static_assert(std::abs(u - ((1.0 + std::sqrt(3) / 2) * 0.5)) < 1E-4);
        static_assert(std::abs(v - ((1.0 - std::sqrt(3) / 2) * 0.5)) < 1E-4);
        static_assert(std::abs(w - ((1.0 - std::sqrt(3) / 2) * 0.5)) < 1E-4);
    }
}

// 测试扇区1: alpha=0.4, beta=0.2
[[maybe_unused]] void test_sector1(){
    {
        static constexpr auto uvw = SVM({.alpha = iq16(0.4), .beta = iq16(0.2)});
        static constexpr float u = (float)uvw.u;
        static constexpr float v = (float)uvw.v;
        static constexpr float w = (float)uvw.w;

        // 验证占空比在有效范围内 [0, 1]
        static_assert(u >= 0.0 && u <= 1.0);
        static_assert(v >= 0.0 && v <= 1.0);
        static_assert(w >= 0.0 && w <= 1.0);
        // 验证中点在0.5附近
        static_assert(std::abs((u + v + w) / 3.0 - 0.5) < 0.2);
    }
}

// 测试扇区2: alpha=0.3, beta=0.3
[[maybe_unused]] void test_sector2(){
    {
        static constexpr auto uvw = SVM({.alpha = iq16(0.3), .beta = iq16(0.3)});
        static constexpr float u = (float)uvw.u;
        static constexpr float v = (float)uvw.v;
        static constexpr float w = (float)uvw.w;

        static_assert(u >= 0.0 && u <= 1.0);
        static_assert(v >= 0.0 && v <= 1.0);
        static_assert(w >= 0.0 && w <= 1.0);
    }
}

// 测试扇区3: alpha=-0.2, beta=0.4
[[maybe_unused]] void test_sector3(){
    {
        static constexpr auto uvw = SVM({.alpha = iq16(-0.2), .beta = iq16(0.4)});
        static constexpr float u = (float)uvw.u;
        static constexpr float v = (float)uvw.v;
        static constexpr float w = (float)uvw.w;

        static_assert(u >= 0.0 && u <= 1.0);
        static_assert(v >= 0.0 && v <= 1.0);
        static_assert(w >= 0.0 && w <= 1.0);
    }
}

// 测试扇区4: alpha=-0.4, beta=-0.2
[[maybe_unused]] void test_sector4(){
    {
        static constexpr auto uvw = SVM({.alpha = iq16(-0.4), .beta = iq16(-0.2)});
        static constexpr float u = (float)uvw.u;
        static constexpr float v = (float)uvw.v;
        static constexpr float w = (float)uvw.w;

        static_assert(u >= 0.0 && u <= 1.0);
        static_assert(v >= 0.0 && v <= 1.0);
        static_assert(w >= 0.0 && w <= 1.0);
    }
}

// 测试扇区5: alpha=-0.3, beta=-0.3
[[maybe_unused]] void test_sector5(){
    {
        static constexpr auto uvw = SVM({.alpha = iq16(-0.3), .beta = iq16(-0.3)});
        static constexpr float u = (float)uvw.u;
        static constexpr float v = (float)uvw.v;
        static constexpr float w = (float)uvw.w;

        static_assert(u >= 0.0 && u <= 1.0);
        static_assert(v >= 0.0 && v <= 1.0);
        static_assert(w >= 0.0 && w <= 1.0);
    }
}

// 测试扇区6: alpha=0.2, beta=-0.4
[[maybe_unused]] void test_sector6(){
    {
        static constexpr auto uvw = SVM({.alpha = iq16(0.2), .beta = iq16(-0.4)});
        static constexpr float u = (float)uvw.u;
        static constexpr float v = (float)uvw.v;
        static constexpr float w = (float)uvw.w;

        static_assert(u >= 0.0 && u <= 1.0);
        static_assert(v >= 0.0 && v <= 1.0);
        static_assert(w >= 0.0 && w <= 1.0);
    }
}


// 测试对称性：相反向量应该产生互补的占空比
[[maybe_unused]] void test_symmetry(){
    {
        static constexpr auto uvw1 = SVM({.alpha = iq16(0.3), .beta = iq16(0.2)});
        static constexpr auto uvw2 = SVM({.alpha = iq16(-0.3), .beta = iq16(-0.2)});

        static constexpr float u1 = (float)uvw1.u;
        static constexpr float v1 = (float)uvw1.v;
        static constexpr float w1 = (float)uvw1.w;

        static constexpr float u2 = (float)uvw2.u;
        static constexpr float v2 = (float)uvw2.v;
        static constexpr float w2 = (float)uvw2.w;

        // 互补占空比应该加起来等于1
        static_assert(std::abs((u1 + u2) - 1.0) < 1E-3);
        static_assert(std::abs((v1 + v2) - 1.0) < 1E-3);
        static_assert(std::abs((w1 + w2) - 1.0) < 1E-3);
    }
}

}