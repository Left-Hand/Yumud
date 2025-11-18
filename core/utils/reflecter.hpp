#pragma once

#include "core/polymorphism/reflect.hpp"
#include "core/stream/ostream.hpp"

namespace ymd::reflecter{

template<typename T>
requires std::is_aggregate_v<std::decay_t<T>>
struct MemberPtrReflecter{
    template<size_t N>
    static constexpr std::nullptr_t member_ptr_v = nullptr;
};

template<typename T>
requires std::is_aggregate_v<std::decay_t<T>>
struct Reflecter{

    template<size_t N>
    using member_t = reflect::member_type<N, T>;

    template<size_t N>
    static constexpr auto member_name_v = reflect::member_name<N, T>;

    static constexpr auto member_count_v = reflect::size<T>();

    template<size_t N>
    static constexpr size_t member_size_v = []{
        T tmp{};
        return reflect::size_of<N>(tmp);
    }();

    template<size_t N>
    static constexpr size_t member_align_v = []{
        T tmp{};
        return reflect::align_of<N>(tmp);
    }();

    template<size_t N>
    static constexpr size_t member_offset_v = []{
        T tmp{};
        return reflect::offset_of<N>(tmp);
    }();

    template<size_t N>
    static constexpr auto member_ptr_v = MemberPtrReflecter<T>::template member_ptr_v<N>;


private:
    // 递归构建tuple的辅助模板
    template<size_t... Is>
    static constexpr auto make_members_tuple_impl(std::index_sequence<Is...>) {
        return std::tuple<typename reflect::member_type<Is, T>...>{};
    }

public:
    // 最终暴露的成员tuple类型
    using members_as_tuple_t = decltype(make_members_tuple_impl(
        std::make_index_sequence<member_count_v>{}));
        
};

template<typename T>
requires std::is_aggregate_v<std::decay_t<T>>
struct Displayer{
    using AggregateType = std::decay_t<T>;
    static constexpr size_t N = reflect::size<AggregateType>();
    static constexpr auto NAME = reflect::type_name<AggregateType>();
    static OutputStream& display(OutputStream& os, const T & value) {


        if constexpr (N == 0) return os << os.scoped(NAME);
        else {
            os.scoped(NAME)([&]<size_t... Is>(std::index_sequence<Is...>){
                // Fold expression with conditional splitter insertion
                bool first = true;
                (..., [&](const auto& field) -> void{
                    if (!first) os << os.splitter();
                    first = false;
                    os.field(reflect::member_name<Is>(value))(reflect::get<Is>(value));
                }(reflect::get<Is>(value)));
                return std::ignore;
            }(std::make_index_sequence<N>{}));
        }
        
        return os;
    }
};




#if 0
// 辅助宏：计算参数数量
#define DEF_GET_NTH_ARG(_1, _2, _3, _4, _5, N, ...) N
#define DEF_COUNT_ARGS(...) DEF_GET_NTH_ARG(__VA_ARGS__, 5, 4, 3, 2, 1)

// 宏重载分发器
#define DEF_DERIVE_MEM_REFLECTER(...) \
    DEF_CONCAT(DEF_DERIVE_MEM_REFLECTER_, DEF_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)

// 连接宏
#define DEF_CONCAT(a, b) DEF_CONCAT_INNER(a, b)
#define DEF_CONCAT_INNER(a, b) a ## b
#endif

// 定义不同参数数量的实现
#define DEF_DERIVE_MEM_REFLECTER_0(T) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        return nullptr;\
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_1(T, m0) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else static_assert(N < 1, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_2(T, m0, m1) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else static_assert(N < 2, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_3(T, m0, m1, m2) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else static_assert(N < 3, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_4(T, m0, m1, m2, m3) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else if constexpr (N == 3) return &T::m3; \
        else static_assert(N < 4, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_5(T, m0, m1, m2, m3, m4) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else if constexpr (N == 3) return &T::m3; \
        else if constexpr (N == 4) return &T::m4; \
        else static_assert(N < 5, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_6(T, m0, m1, m2, m3, m4, m5) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else if constexpr (N == 3) return &T::m3; \
        else if constexpr (N == 4) return &T::m4; \
        else if constexpr (N == 5) return &T::m5; \
        else static_assert(N < 6, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_7(T, m0, m1, m2, m3, m4, m5, m6) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else if constexpr (N == 3) return &T::m3; \
        else if constexpr (N == 4) return &T::m4; \
        else if constexpr (N == 5) return &T::m5; \
        else if constexpr (N == 6) return &T::m6; \
        else static_assert(N < 7, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_8(T, m0, m1, m2, m3, m4, m5, m6, m7) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else if constexpr (N == 3) return &T::m3; \
        else if constexpr (N == 4) return &T::m4; \
        else if constexpr (N == 5) return &T::m5; \
        else if constexpr (N == 6) return &T::m6; \
        else if constexpr (N == 7) return &T::m7; \
        else static_assert(N < 8, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_9(T, m0, m1, m2, m3, m4, m5, m6, m7, m8) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else if constexpr (N == 3) return &T::m3; \
        else if constexpr (N == 4) return &T::m4; \
        else if constexpr (N == 5) return &T::m5; \
        else if constexpr (N == 6) return &T::m6; \
        else if constexpr (N == 7) return &T::m7; \
        else if constexpr (N == 8) return &T::m8; \
        else static_assert(N < 9, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_10(T, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else if constexpr (N == 3) return &T::m3; \
        else if constexpr (N == 4) return &T::m4; \
        else if constexpr (N == 5) return &T::m5; \
        else if constexpr (N == 6) return &T::m6; \
        else if constexpr (N == 7) return &T::m7; \
        else if constexpr (N == 8) return &T::m8; \
        else if constexpr (N == 9) return &T::m9; \
        else static_assert(N < 10, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_11(T, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else if constexpr (N == 3) return &T::m3; \
        else if constexpr (N == 4) return &T::m4; \
        else if constexpr (N == 5) return &T::m5; \
        else if constexpr (N == 6) return &T::m6; \
        else if constexpr (N == 7) return &T::m7; \
        else if constexpr (N == 8) return &T::m8; \
        else if constexpr (N == 9) return &T::m9; \
        else if constexpr (N == 10) return &T::m10; \
        else static_assert(N < 11, "Index out of bounds"); \
    }(); \
};

#define DEF_DERIVE_MEM_REFLECTER_12(T, m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11) \
template<> \
struct reflecter::MemberPtrReflecter<T> { \
    template<size_t N> \
    static constexpr auto member_ptr_v = [] { \
        if constexpr (N == 0) return &T::m0; \
        else if constexpr (N == 1) return &T::m1; \
        else if constexpr (N == 2) return &T::m2; \
        else if constexpr (N == 3) return &T::m3; \
        else if constexpr (N == 4) return &T::m4; \
        else if constexpr (N == 5) return &T::m5; \
        else if constexpr (N == 6) return &T::m6; \
        else if constexpr (N == 7) return &T::m7; \
        else if constexpr (N == 8) return &T::m8; \
        else if constexpr (N == 9) return &T::m9; \
        else if constexpr (N == 10) return &T::m10; \
        else if constexpr (N == 11) return &T::m11; \
        else static_assert(N < 12, "Index out of bounds"); \
    }(); \
};
}