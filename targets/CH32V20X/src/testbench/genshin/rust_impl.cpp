#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/can/can_msg.hpp"

#include "core/magic/size_traits.hpp"
#include "core/magic/function_traits.hpp"

#include "core/magic/serialize_traits.hpp"
#include "core/magic/enum_traits.hpp"
#include "rust_enum.hpp"


using namespace ymd;


namespace ymd{
template<typename Trait, typename Obj>
struct ImplFor{};

template<typename Protocol>
struct SerializeAs;

template<typename Protocol>
struct DeserializeFrom;

struct RawBytes;

template<std::integral D>
struct ImplFor<SerializeAs<RawBytes>, D> {
    static constexpr size_t N = magic::type_to_bytes_v<D>;
    static constexpr std::array<uint8_t, N> serialize(const D obj) {
        using Raw = magic::type_to_uint_t<D>;
        const auto raw = std::bit_cast<Raw>(obj);
        using IS = std::make_index_sequence<N>;

        return [&]<size_t... Idx>(std::index_sequence<Idx...>) {
            return std::array<uint8_t, N>{static_cast<uint8_t>((raw >> (8 * Idx)) & 0xFF)...};
        }(IS{});
    }
};

template<std::integral D>
struct ImplFor<DeserializeFrom<RawBytes>, D> {
    static constexpr size_t N = magic::type_to_bytes_v<D>;
    
    static constexpr D deserialize(const std::span<const uint8_t, N> bytes) {
        using IS = std::make_index_sequence<N>;

        auto transform = [](const uint8_t byte, const size_t idx) {
            return static_cast<D>(byte) << (8 * idx);
        };

        return [&]<size_t... Idx>(std::index_sequence<Idx...>) {
            return (transform(bytes[Idx], Idx) | ...);
        }(IS{});
    }
};


template<size_t Q>
struct ImplFor<SerializeAs<RawBytes>, fixed_t<Q, int32_t>> {
    static constexpr std::array<uint8_t, 4> serialize(const fixed_t<Q, int32_t> obj){
        return ImplFor<SerializeAs<RawBytes>, int32_t>::serialize(obj.as_bits());
    }
};

template<size_t Q>
struct ImplFor<DeserializeFrom<RawBytes>, fixed_t<Q, int32_t>> {
    static constexpr fixed_t<Q, int32_t> deserialize(const std::span<const uint8_t, 4> bytes){
        return fixed_t<Q, int32_t>::from_bits(ImplFor<DeserializeFrom<RawBytes>, int32_t>::deserialize(bytes));
    }
};

template<std::floating_point F>
struct ImplFor<SerializeAs<RawBytes>, F> {
    static constexpr size_t N = magic::type_to_bytes_v<F>;
    using Raw = magic::type_to_uint_t<F>;
    static constexpr std::array<uint8_t, N> serialize(const F obj){
        return ImplFor<SerializeAs<RawBytes>, Raw>::serialize(std::bit_cast<Raw>(obj));
    }
};

template<std::floating_point F>
struct ImplFor<DeserializeFrom<RawBytes>, F> {
    static constexpr size_t N = magic::type_to_bytes_v<F>;
    using Raw = magic::type_to_uint_t<F>;

    static constexpr F deserialize(const std::span<const uint8_t, N> bytes){
        return std::bit_cast<F>(ImplFor<DeserializeFrom<RawBytes>, Raw>::deserialize(bytes));
    }
};

template<typename Protocol, typename T>
static constexpr auto serialize(T && obj){
    return ImplFor<SerializeAs<Protocol>, std::decay_t<T>>::serialize(obj);
}

template<typename Protocol, typename ... Args>
static constexpr auto serialize(Args && ... args){
    constexpr size_t N = magic::total_bytes_of_args_v<std::decay_t<Args> ... >;

    std::array<uint8_t, N> result;
    size_t offset = 0;

    ([&](auto && arg) {
        const auto bytes = serialize<Protocol>(arg);
        std::copy(bytes.begin(), bytes.end(), result.begin() + offset);
        offset += bytes.size();
    }(args), ...);
    
    return result;
}

template<
    typename Protocol, 
    typename T, 
    size_t N = magic::type_to_bytes_v<T>
>
requires (std::is_same_v<Protocol, RawBytes>)
static constexpr auto _deserialize(const std::span<const uint8_t, N> bytes){
    return ImplFor<DeserializeFrom<Protocol>, T>::deserialize(bytes);
}

// template<
//     typename Protocol, 
//     typename T,
//     size_t N = magic::type_to_bytes_v<T>>
// requires (std::is_same_v<Protocol, RawBytes>)
// static constexpr auto _deserialize(T && bytes){
//     return ImplFor<DeserializeFrom<Protocol>, T>::deserialize(std::span<const T >(&bytes, 1));
// }


template<typename Protocol, typename... Args,
    size_t N = magic::total_bytes_of_args_v<std::decay_t<Args>...>>
requires (std::is_same_v<Protocol, RawBytes>)
static constexpr auto deserialize(const std::span<const uint8_t, N> bytes) {
    // 逐个处理参数（避免递归展开）
    if constexpr (sizeof...(Args) == 1) {
        using First = std::tuple_element_t<0, std::tuple<Args...>>;
        return _deserialize<Protocol, First>(bytes);
    } else {
        using Is = std::make_index_sequence<sizeof...(Args)>;
        using Tup = std::tuple<std::decay_t<Args>...>;

        // 根据索引序列展开并构造元组
        auto construct_tuple = [&bytes]<std::size_t... Indices>(std::index_sequence<Indices...>) {
            return std::make_tuple(magic::fetch_arg_from_bytes<Indices, Tup>(bytes)...);
        };

        // 使用索引序列调用辅助函数
        return construct_tuple(Is{});
    }
}

template<
    typename Protocol, 
    typename T, 
    size_t N = magic::type_to_bytes_v<T>
>
requires (std::is_same_v<Protocol, hal::CanMsg>)
static constexpr auto deserialize(const hal::CanMsg msg){
    return ImplFor<DeserializeFrom<hal::CanMsg>, T>::deserialize(msg);
}

}


namespace ymd{


struct MyStruct {

    uint32_t private_data;

    template<typename T, typename U>
    friend struct ImplFor;
};


template<>
struct ImplFor<DeserializeFrom<hal::CanMsg>, MyStruct> {
    static constexpr Option<MyStruct> deserialize(const hal::CanMsg & msg){

        
        switch(msg.size()){
            default: return None;

            case 4: 
            case 8: {
                const auto bytes = msg.to_span_with_length<4>();
                return Some(MyStruct{
                    .private_data = ::deserialize<RawBytes, uint32_t>(bytes)
                });
            }
        }
    }
};

template<>
struct ImplFor<int, MyStruct> {
    static void method(MyStruct& obj) {
        obj.private_data = 42;
        DEBUGGER << "Set to 42 via int\n";
    }
};



static constexpr auto serialized1 = serialize<RawBytes>(uint8_t(42));
static constexpr auto serialized2 = serialize<RawBytes>(1_iq16);
static constexpr auto serialized3 = serialize<RawBytes>(1.0f);
static constexpr auto serialized4 = serialize<RawBytes>(1.0f, 1_iq16);

static constexpr auto deserialized1 = deserialize<RawBytes, uint8_t>(std::span(serialized1));
static constexpr auto deserialized2 = deserialize<RawBytes, iq16>(std::span(serialized2));
static constexpr auto deserialized3 = deserialize<RawBytes, float>(std::span(serialized3));
static constexpr auto deserialized4 = deserialize<RawBytes, float, iq16>(std::span(serialized4));
static constexpr auto deserialized4f = std::get<0>(deserialized4);
static constexpr auto deserialized4q = std::get<1>(deserialized4);

// static constexpr auto msg = hal::CanMsg::from_bytes(CanStdId(0x123), std::span(serialized2));
static constexpr auto msg = hal::CanMsg::from_bytes(hal::CanStdId(0x123), std::span(serialized2));
// static constexpr auto deserialized4m = deserialize<CanMsg, MyStruct>(msg);
static constexpr auto msg_size = msg.size();
static constexpr auto deserialized4m = deserialize<hal::CanMsg, MyStruct>(msg).unwrap();

// static_assert(deserialized1 == 42, "deserialized1 != 42");
static_assert(deserialized2 == 1_iq16, "deserialized2 != 1_iq16");
static_assert(deserialized4q == 1_iq16, "deserialized2 != 1_iq16");
static_assert(deserialized4f == 1.0f, "deserialized2 != 1_iq16");
// static_assert(deserialized3 == 1.0f, "deserialized3 != 1.0f");


static_assert(msg_size == 4, "msg_size != 4");
// static_assert(deseru == 4, "msg_size != 4");

template<>
struct ImplFor<SerializeAs<uint8_t>, MyStruct> {
    static void method(MyStruct& obj) {
        obj.private_data = 42;
        DEBUGGER << "Set to 42 via int\n";
    }
};


template<>
struct ImplFor<real_t, MyStruct> {
    static auto method(const MyStruct & obj) {
        // return magic::make_bytes_from_tuple(std::make_tu)
    }
};

template<typename T>
void my_method(MyStruct & obj) {
    ImplFor<T, MyStruct>::method(obj);
}


class Event{
protected:
    // enum class Kind{
    //     Circle,
    //     Square,
    //     Triangle,
    // };

    struct Circle{real_t radius;};
    struct Square{real_t length;};
    struct Triangle{real_t a,b,c;};

private:    
    using Element = std::variant<Circle, Square, Triangle>;
    Element element;
};


void genshin_main() {
    MyStruct obj;
    my_method<int>(obj); // 输出：Set to 42 via int
    my_method<real_t>(obj); // 输出：Set to 42.0 via real_t
}

}