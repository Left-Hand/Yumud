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
    static OutputStream& display(OutputStream& os, const T & value) {
        using AggregateType = std::decay_t<T>;
        os.scoped(reflect::type_name<AggregateType>())([&]<size_t... Is>(std::index_sequence<Is...>) -> OutputStream& {
            // Fold expression with conditional splitter insertion
            bool first = true;
            return(..., [&](const auto& field) -> OutputStream& {
                if (!first) os << os.splitter();
                first = false;
                return os << os.field(reflect::member_name<Is>(value))(os << reflect::get<Is>(value));
            }(reflect::get<Is>(value)));
        }(std::make_index_sequence<reflect::size<AggregateType>()>{}));
        
        return os;
    }
};


}