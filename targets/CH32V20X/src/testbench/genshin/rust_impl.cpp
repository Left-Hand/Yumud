#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "core/utils/typetraits/size_traits.hpp"
#include "core/utils/typetraits/function_traits.hpp"
// #include "core/utils/typetraits/typetraits_details.hpp"
#include "core/utils/typetraits/serialize_traits.hpp"
#include "core/utils/typetraits/enum_traits.hpp"



using namespace ymd;

namespace ymd{
template<typename T, typename U>
struct ImplFor{};

struct MyStruct {
private:
    int private_data;

    // template<typename T>
    // friend struct ImplFor<T, MyStruct>;
    template<typename T, typename U>
    friend struct ImplFor;
};


template<>
struct ImplFor<int, MyStruct> {
    static void method(MyStruct& obj) {
        obj.private_data = 42;
        DEBUGGER << "Set to 42 via int\n";
    }
};

template<>
struct ImplFor<real_t, MyStruct> {
    static void method(MyStruct& obj) {
        obj.private_data = int(42.0_r);
        DEBUGGER << "Set to 42.0 via real_t\n";
    }
};

template<typename T>
void my_method(MyStruct & obj) {
    ImplFor<T, MyStruct>::method(obj);
}

void genshin_main() {
    MyStruct obj;
    my_method<int>(obj); // 输出：Set to 42 via int
    my_method<real_t>(obj); // 输出：Set to 42.0 via real_t
}

}