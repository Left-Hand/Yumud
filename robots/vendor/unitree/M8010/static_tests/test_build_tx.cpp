#include "../m8010_prelude.hpp"
#include "../m8010_utils.hpp"
#include "../m8010_tx_builder.hpp"

using namespace ymd;
using namespace robots::unitree::m8010;



namespace{

[[maybe_unused]] static void test_build_tx_frame(){
    {
        static constexpr auto buffer = []{
            alignas(4) std::array<uint8_t, 17> temp_buffer;
            build_tx_frame(temp_buffer)
                .push_header()
                .push_mode(ModeInfo{.bits = 0x1e})  // 示例模式信息
                .push_tau_code(TorqueCode{.bits = 100})  // 示例扭矩码
                .push_x2_code(X2Code{.bits = 200})  // 示例速度码
                .push_x1_code(X1Code{.bits = 5000})  // 示例位置码
                .push_kp_code(KpCode{.bits = 1500})  // 示例Kp码
                .push_kd_code(KdCode{.bits = 1000})  // 示例Kd码
                .finalize();
            
            return temp_buffer;
        }();

        static_assert(buffer[0] == 0xee);
        static_assert(buffer[1] == 0xfe);
        static_assert(buffer[2] == 0x1e);  // 模式信息应该在这里
    }
}

}