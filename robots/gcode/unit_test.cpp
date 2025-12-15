#include "gcode.hpp"

using namespace ymd::gcode;

namespace test{

static inline void static_test(){
    // 测试助记符解析
    static_assert(GcodeLine("G28").query_mnemonic().unwrap() == Mnemonic::General);
    static_assert(GcodeLine("M106").query_mnemonic().unwrap() == Mnemonic::Miscellaneous);
    // static_assert(not GcodeLine("X10").query_mnemonic().is_ok()); // 假设X不是合法助记符

    // 测试主/次编号
    static_assert(GcodeLine("G28").query_major(Mnemonic::General).unwrap() == 28);
    static_assert(GcodeLine("G28.3").query_minor(Mnemonic::General).unwrap() == 3);
    static_assert(GcodeLine("G1").query_minor(Mnemonic::General).is_err()); // 无次编号

    // static_assert(GcodeLine("G1 F1000").query_major(Mnemonic::General).is_ok());

}


}