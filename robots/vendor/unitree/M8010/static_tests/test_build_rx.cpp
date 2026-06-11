#include "../m8010_prelude.hpp"
#include "../m8010_utils.hpp"
#include "../m8010_rx_builder.hpp"

using namespace ymd;
using namespace robots::unitree::m8010;

namespace{

[[maybe_unused]] static void test_build_rx_frame(){
    {
        static constexpr auto buffer = []{
            alignas(4) std::array<uint8_t, 16> temp_buffer;
            build_rx_frame(temp_buffer)
                .push_header()
                .push_mode(ModeInfo{.bits = 0x0e})  // 示例模式信息
                .push_tau_code(TorqueCode{.bits = 50})  // 示例扭矩码
                .push_x2_code(X2Code{.bits = 100})  // 示例速度码
                .push_x1_code(X1Code{.bits = 2500})  // 示例位置码
                .push_temp_code(TempCode{.bits = 25})  // 示例温度码
                .push_misc(RxMisc{.err_bits = 0, .force_bits = 1, .__resv__ = 0})  // 示例杂项信息
                .finalize();
            
            return temp_buffer;
        }();

        static_assert(buffer[0] == 0xee);
        static_assert(buffer[1] == 0xfd);
        static_assert(buffer[2] == 0x0e);  // 模式信息应该在这里
    }
}

}