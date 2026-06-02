#include "core/debug/debug.hpp"
#include <type_traits>
#include <utility>
#include <string_view>
#include "core/string/view/string_view.hpp"

#define THROW(s) __builtin_unreachable();

using namespace ymd;

// 类型与格式说明符的编译期匹配规则
template <typename T>
consteval bool check_format_specifier(char spec) {
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
        // 整数：接受空说明符 或 'd'
        return spec == '\0' || spec == 'd';
    } else if constexpr (std::is_floating_point_v<T>) {
        // 浮点数：接受空说明符 或 'f'
        return spec == '\0' || spec == 'f';
    } else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, std::string_view>) {
        // 字符串：接受空说明符 或 's'
        return spec == '\0' || spec == 's';
    } else {
        // 其他类型仅允许空说明符
        return spec == '\0';
    }
}


// 编译期格式串包装器（核心”关卡”）
template <typename... Args>
struct FormatPolicy {
public:
    const char* str;

    consteval FormatPolicy(const char * s) : str(s) {
        const char* p = str;
        // const char* p;
        int arg_index = 0;

        while (*p) {
        // while(p < str.end()){
            if (*p == '{') {
                ++p;  // 跳过 '{'
                char spec = '\0';
                if (*p != '}') {
                    spec = *p;      // 暂只支持单字符说明符
                    ++p;
                }
                if (*p != '}') {
                    THROW("缺少 '}'");   // 编译期直接报错
                }
                // 用 index_sequence 取出第 arg_index 个参数的类型并检查
                bool ok = false;
                [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                    // 折叠表达式，只在 Is == arg_index 时执行检查
                    ((Is == arg_index ? (ok = check_format_specifier<Args>(spec), 0) : 0), ...);
                }(std::index_sequence_for<Args...>{});

                if (!ok) {
                    THROW("格式说明符与参数类型不匹配");   // 编译期报错
                }
                ++arg_index;
            } else if (*p == '}') {
                THROW("未配对的 '}'");
            }
            ++p;
        }

        // （可选）严格检查占位符数量与参数数量一致
        if constexpr (sizeof...(Args) > 0) {
            if (arg_index != sizeof...(Args)) {
                THROW("占位符数量与参数数量不一致");
            }
        }
    }


};



// 编译期字符串包装 - 用于 NTTP
template <std::size_t N>
struct FixedString {
    char str[N];

    constexpr FixedString(const char (&s)[N]) {
        std::copy_n(s, N, str);
    }
};

// fmt 实现 - 格式串作为模板参数
template <FixedString FmtStr, typename... Args>
void fmt_impl(Args&&... args) {
    FormatPolicy<Args...> policy(FmtStr.str);
    DEBUGGER << "格式串: " << FmtStr.str << "\n参数: ";
    ((DEBUGGER << args << ' '), ...);
    DEBUGGER << '\n';
}

// 宏 - 自动推导格式串并套上模板参数
#define fmt(str, ...) fmt_impl<FixedString(str)>(__VA_ARGS__)

// 使用示例
[[maybe_unused]] void test_fmt() {
    fmt("Hello, {d}!", 42);               // ✅ 通过 - 编译期检查
    fmt("int{d}", 100);                   // ✅ 通过
    fmt("float{f}", 3.14159);             // ✅ 通过
    fmt("str{s}", std::string_view("world")); // ✅ 通过

    // 以下任何一条取消注释都会导致编译失败
    // fmt("{:d}", 3.14);                // ❌ double 不接受 'd'
    // fmt("{:f}", 10);                  // ❌ int 不接受 'f'
    // fmt("{} {}", 1);                  // ❌ 占位符数量与参数数量不一致
}


