#include "im6014_prelude.hpp"
#include "im6014_msgs.hpp"


using namespace ymd;
using namespace robots::unitree::im6014;

namespace{

static_assert(CodeFactory::make_tau_code_from_nm(1.0f).bits == 2560);
static_assert(CodeFactory::make_x2code_from_speed(Angular<float>::from_turns(1.0f)).bits == 64);
static_assert(CodeFactory::make_x1code_from_turns(1.0f).bits == 32768);
static_assert(CodeFactory::make_kpcode(1.0f).bits == 12800);
static_assert(CodeFactory::make_kdcode(1.0f).bits == 51200);

static_assert(CodeFactory::make_tau_code_from_nm(1_iq16).bits == 2560);
static_assert(CodeFactory::make_x2code_from_speed(Angular<iq16>::from_turns(1_iq16)).bits == 64);

static_assert(CodeFactory::make_x1code_from_turns(1_iq16).bits == 32768);
static_assert(CodeFactory::make_x1code_from_turns(2_iq16).bits == 32768 * 2);

[[maybe_unused]] static void test_code(){
    constexpr auto x = std::numeric_limits<iq15>::max();
    static_assert(CodeFactory::make_x1code_from_turns(x).bits == INT32_MAX);
}

static_assert(CodeFactory::make_kpcode(1_iq16).bits == 12800);
static_assert(CodeFactory::make_kdcode(1_iq16).bits == 51200);

[[maybe_unused]] static void test_build_cmd_packet(){
    {
        static constexpr auto buffer = []{
            alignas(4) std::array<uint8_t, 20> temp_buffer;
            build_cmd_packet(temp_buffer)
                .push_header()
                .push_status(StatusCode{
                    .id = 0xe, .status = 2, .timeout = 1
                })
                .push_tau_code(CodeFactory::make_tau_code_from_nm(1.0f))
                .push_x2_code(CodeFactory::make_x2code_from_speed(Angular<float>::from_radians(1)))
                .push_x1_code(CodeFactory::make_x1code_from_turns(5.0))
                .push_kp_code(CodeFactory::make_kpcode(3.0f))
                .push_kd_code(CodeFactory::make_kdcode(2.0f))
                .finalize();
            ;

            return temp_buffer;
        }();

        static_assert(buffer[0] == 0xee);
        static_assert(buffer[1] == 0xfe);
        static_assert(buffer[2] == 0xae);
        static_assert(buffer[3] == 0x00);
        static_assert(buffer[19] == 5);
    }


}

}