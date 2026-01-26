#include "heapless_string.hpp"

using namespace ymd;


namespace{

[[maybe_unused]] static void static_test(){
    constexpr auto str = HeaplessString<10>("Hello");
    constexpr auto str2 = []{
        auto _str = HeaplessString<10>("Hello");
        _str.push_back('!').unwrap();
        return _str;
    }();

    constexpr auto str3 = []{
        auto _str = HeaplessString<10>("Hello");
        _str.try_insert(0, '!').unwrap();
        return _str;
    }();

    constexpr auto str4 = []{
        auto _str = HeaplessString<10>("Hello");
        _str.try_erase(4).unwrap();
        return _str;
    }();

    static_assert(str.length() == 5);
    static_assert(str2.length() == 6);
    static_assert(str3.length() == 6);
    static_assert(str2.view() == StringView("Hello!"));
    static_assert(str3.view() == StringView("!Hello"));

    // 测试无效插入返回错误
    static_assert(
        HeaplessString<3>("A").try_insert(5, 'X').is_err()  // ✅ 应返回错误
    );
}
    
}