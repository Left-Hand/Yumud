#include "src/testbench/tb.h"

#include <fmt/core.h>
#include <fmt/format.h>

#include "sys/math/real.hpp"
#include "sys/debug/debug.hpp"

using namespace ymd;
using namespace ymd::hal;
// using namespace fmt;
namespace fmt {
    // template<typename Char, size_t Q>
    // struct formatter<iq_t<Q>, Char> {
    //     auto parse(const basic_parse_context<Char>& ctx) {
    //         return ctx.begin(); // 简单实现暂不解析格式参数
    //     }

    //     template<typename FormatContext>
    //     auto format(const ymd::iq_t<Q>& value, FormatContext& ctx) {
    //         char buffer[64];
    //         // 调用现有qtoa实现转换（假设eps=3）
    //         ymd::StringUtils::qtoa<Q>(value, buffer, 3);
    //         return fmt::format_to(ctx.out(), "{}", buffer);
    //     }
    // };
}

void fmt_main(){
    uart2.init(576000);
    DEBUGGER.retarget(&uart2);
    DEBUGGER.setEps(4);
    DEBUGGER.setSplitter(",");


}