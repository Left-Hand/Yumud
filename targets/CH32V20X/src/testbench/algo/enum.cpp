#include "core/math/realmath.hpp"

#include "../tb.h"

#include "core/math/real.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/clock/clock.hpp"

#include "core/utils/enum/enum_dict.hpp"
#include "core/utils/Option.hpp"
#include "core/tmp/serialize_traits.hpp"

#include "core/tmp/functor.hpp"
#include "core/tmp/bits/width.hpp"
#include "hal/bus/uart/hw_singleton.hpp"

using namespace ymd;


enum class Fruit:uint8_t{
    BANANA = 5,
    APPLE = 12,
    MANGO = 14,
};

namespace ymd{
DEF_DERIVE_DEBUG(Fruit)
}



auto pfunc(const int16_t a, const int16_t b){
    return int(a) * a + int(b) * b;
    // return a;
};

class Ball{
public:
    int a;
    iq16 mset_xy(iq16 x, iq16 y){
        return x * x + y * y;
    }

    iq16 mset_xy2(iq16 x, iq16 y){
        return x + y;
    }
};

// static constexpr bool test_if = is_functor_v<decltype(pfunc)>;

auto func = [](const iq30 a, iq30 b){
    return a * a + b * b;
    // return a;
};

auto runtime_true(){
    return sys::chip::get_chip_id_crc();
}



void enum_main(){

    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
    });
    DEBUGGER.retarget(&hal::usart2);
    DEBUGGER.set_eps(4);
    DEBUGGER.set_splitter(",");

    Ball ball;

    while(true){
        // auto set_xy = [](iq16 x, iq16 y){
        //     return x * x + y * y;
        // };

        const auto t = clock::seconds();

        // const auto s = sin<30>(t);
        // const auto c = cos<30>(t);
        const auto s = math::sin(t);
        const auto c = math::cos(t);
        // const auto s = int16_t(sin(t) * 300);
        // const auto c = int16_t(cos(t) * 300);
        
        real_t r = 0; 
        // int r = 0; 

        auto dyn_func = runtime_true() ? &Ball::mset_xy : &Ball::mset_xy2;
        const auto bytes = tmp::serialize_args_to_bytes(s, c);
        for(size_t i = 0; i < 100000; i++){
            // r = invoke_func_by_serialzed_bytes(set_xy, std::span(bytes));
            // r = invoke_func_by_serialzed_bytes<int, int16_t, int16_t>(pfunc, std::span(bytes));
            // r = invoke_func_by_serialzed_bytes<int>(pfunc, std::span(bytes));
            // r = invoke_func_by_serialzed_bytes<int>(pfunc, std::span(bytes));
            // r = invoke_func_by_serialzed_bytes(ball, &Ball::mset_xy, std::span(bytes));
            r = tmp::invoke_func_by_serialzed_bytes(ball, dyn_func, std::span(bytes));
        }
        // DEBUG_PRINTLN(t);
        // DEBUG_PRINTLN(s,c,r, uint32_t(micros() - begin_m), Fruit::APPLE);
        // DEBUG_PRINTLN(Fruit::APPLE);
        // const auto name = tmp::enum_item_name_v<Fruit, Fruit::APPLE>;
        // const auto name = tmp::enum_item_name_v<Fruit, Fruit::BANANA>;
        // const auto name = tmp::enum_item_name_v<Fruit, Fruit::APPLE>;
        const auto dyn_name = runtime_true() ? 
            tmp::enum_item_name_v<Fruit, Fruit::MANGO> : tmp::enum_item_name_v<Fruit, Fruit::BANANA>;
        DEBUG_PRINTLN(dyn_name);
        clock::delay(10ms);
        // clock::delay(10ms);
    }


    {
        // [[maybe_unused]] constexpr const char * banana = tmp::enum_item_name_v<Fruit, Fruit::BANANA>;
        // [[maybe_unused]] constexpr const char * _10 = tmp::enum_item_name_v<Fruit, Fruit(10)>;

        // constexpr const char * banana2 = enum_item_name_v2<Fruit::BANANA>;
        // using type = decltype(Fruit::BANANA);
        // constexpr const char * banana = tmp::enum_item_name_v<Fruit, Fruit::BANANA>;
    
        // static_assert(!, "!tmp::enum_is_valid_v<Fruit, Fruit::10>()");
        // const auto f = __PRETTY_FUNCTION__;
        static_assert(tmp::enum_is_valid_v<Fruit, Fruit::BANANA>, "enum_is not _valid_v<Fruit, Fruit::BANANA>()");
        static_assert(!tmp::enum_is_valid_v<Fruit, Fruit(10)>, "tmp::enum_is_valid_v<Fruit, Fruit::10>()");
        // static_assert(!tmp::enum_is_valid_v<Fruit, Fruit>, "!tmp::enum_is_valid_v<Fruit, Fruit::10>()");
    }


    // static constexpr size_t a = tuple_bytes_v<int, int, uint8_t>;
    {
        // using tup_t = std::tuple<int, uint16_t, uint8_t, iq16>;
        
        // static constexpr size_t a1 = tuple_element_bytes_v<1, tup_t>;
        // static constexpr size_t a2 = tuple_element_bytes_v<2, tup_t>;
        // static constexpr size_t a2 = tuple_element_bytes_v<2, tup_t>;

        // static constexpr size_t a3 = element_bytes_v<2, int, uint16_t, uint8_t, iq16>;
        using t0 = tmp::args_element_t<0, int, uint16_t, uint8_t, iq16>;
        using t1 = tmp::args_element_t<1, int, uint16_t, uint8_t, iq16>;
        
        using t3 = std::tuple_element_t<3, std::tuple<int, uint16_t, uint8_t, iq16 >>;

        static_assert(std::is_same_v<t0, int>);
        static_assert(std::is_same_v<t1, uint16_t>);
        static_assert(std::is_same_v<t3, iq16>);

        // static constexpr auto tup1 = std::make_tuple<int, uint16_t, uint8_t, iq16 >(1, 2, 3, 4.0_r);
        
        // static_assert(packed_tuple_total_bytes_v<tup_t> == sizeof(tup_t));
        // total_bytes_v<int, uint16_t, uint8_t, iq16>;
        // packed_tuple_total_bytes_v<tup_t>;
        // static constexpr size_t a0 = element_bytes_v<0, int, uint16_t, uint8_t, iq16>;
        // static constexpr size_t a1 = element_bytes_v<1, int, uint16_t, uint8_t, iq16>;
        
        // static constexpr size_t a = element_bytes_v<1, int, short>;
        // static_assert(tuple_bytes_v<tup_t> == sizeof(tup_t));
        // static_assert(tuple_bytes_v<tup_t> ==);
    }
    
    
    {
        // func_ty
        static constexpr uint32_t u = 0x12345678;
        
        static_assert(uint8_t(tmp::get_byte_from_arg(0, u)) == 0x78);
        static_assert(uint8_t(tmp::get_byte_from_arg(1, u)) == 0x56);
        static_assert(uint8_t(tmp::get_byte_from_arg(2, u)) == 0x34);
        static_assert(uint8_t(tmp::get_byte_from_arg(3, u)) == 0x12);

        static constexpr float f = 1.234f;

        static_assert(uint8_t(tmp::get_byte_from_arg(0, f)) == 0xB6);
        static_assert(uint8_t(tmp::get_byte_from_arg(1, f)) == 0xF3);
        static_assert(uint8_t(tmp::get_byte_from_arg(2, f)) == 0x9D);
        static_assert(uint8_t(tmp::get_byte_from_arg(3, f)) == 0x3F);

        static constexpr auto arr1 = tmp::make_bytes_from_arg(u);
        static constexpr auto arr2 = tmp::make_bytes_from_arg(f);

        static constexpr auto u_ = tmp::make_arg_from_bytes<decltype(u)>(std::span(arr1));
        static constexpr auto f_ = tmp::make_arg_from_bytes<decltype(f)>(std::span(arr2));

        static_assert(u == u_);
        static_assert(f == f_);

        static constexpr uint16_t ui16 = 0xabcd;
        static constexpr auto q21 = 0.12_iq21;
        // static constexpr auto q21 = 16;

        static constexpr auto arr3 = tmp::serialize_args_to_bytes(u, f, ui16, q21);
        static constexpr auto arr4 = tmp::make_bytes_from_tuple(std::make_tuple(u, f, ui16, q21));
        static_assert(arr3 == arr4);

        static constexpr auto tup = std::make_tuple(u, f, ui16, q21);
        using tup_t = std::decay_t<decltype(tup)>;

        // [[maybe_unused]] using a0 = tuple_element_t<0, tup_t>;
        // [[maybe_unused]] using a1 = tuple_element_t<1, tup_t>;
        // [[maybe_unused]] using a2 = tuple_element_t<2, tup_t>;

        static constexpr auto u_2 = tmp::fetch_arg_from_bytes<0, tup_t>(std::span(arr3));
        static constexpr auto f_2 = tmp::fetch_arg_from_bytes<1, tup_t>(std::span(arr3));

        static_assert(u == u_2);
        static_assert(f == f_2);
        // static constexpr auto f_2 = tmp::fetch_arg_from_bytes<1, tup_t>(std::span(arr3));

        // static_ass
        static constexpr auto tup_ = tmp::make_tuple_from_bytes<tup_t>(std::span<const uint8_t, 14>(arr4));
        static_assert(tup == tup_);
        // static const auto tmp::make_arg_from_bytes
    }

    {
        // constexpr 
        auto func1 = [](const uint16_t u, const iq10 q){
            return iq20(q + u);
        };

        static constexpr uint16_t u1 = 10;
        static constexpr iq10 q1 = 0.1_r;
        static constexpr auto r1 = func1(u1, q1);

        static constexpr auto bytes = tmp::serialize_args_to_bytes(u1, q1);
        
        static_assert(tmp::is_functor_v<decltype(func1)>);

        using traits = tmp::details::_functor_traits<std::decay_t<decltype(func1)>>;
        static_assert(std::is_same_v<traits::return_type, iq20>);
        static constexpr auto r2 = tmp::invoke_func_by_serialzed_bytes(func1, std::span(bytes));
        static_assert(r1 == r2);
    }

    {
        // EnumDict<Fruit, int> dict;
    }
}