
#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "primitive/can/can_msg.hpp"

#include "core/magic/size_traits.hpp"
#include "core/magic/function_traits.hpp"
#include "core/magic/serialize_traits.hpp"
#include "core/magic/enum_traits.hpp"
#include "rust_enum.hpp"
// https://github.com/SephDB/tmp-builder-pattern/tree/master
using namespace ymd;

namespace ymd::builder{
namespace details {
    template<auto p>
    struct ReadMemPtr {
        static_assert(p!=p,"not a member pointer");
    };

    template<typename Class, typename Result, Result Class::* ptr>
    struct ReadMemPtr<ptr> {
        static constexpr auto value = ptr;
        using type = Result;
        using containing_type = Class;
    };

    template<auto p>
    using memPtrType = typename ReadMemPtr<p>::type;
    
    template<auto,auto>
    struct isEqualMemPtr {
        static constexpr auto value = false;
    };

    template<typename Class, typename Result, Result Class::* ptr, Result Class::* ptr2>
    struct isEqualMemPtr<ptr,ptr2> {
        static constexpr auto value = (ptr == ptr2);
    };

    template<auto ptr, auto ptr2>
    constexpr bool isEqualMemPtr_v = isEqualMemPtr<ptr,ptr2>::value;

    template<auto ptr>
    struct canApplyMemPtr {
        template<typename T>
        using check = decltype(std::declval<T>().*ptr);
    };

}

template<typename T,auto...>
class BuilderImpl;

template<typename T>
class BuilderImpl<T> {
    T value;
    template<typename,auto...>
    friend class BuilderImpl;
    template<auto... newptrs>
    using addInFront = BuilderImpl<T,newptrs...>;
    constexpr T& getValue() {return value;}
    public:
        constexpr BuilderImpl() = default;
        constexpr BuilderImpl(const BuilderImpl&) = default;
        constexpr BuilderImpl(BuilderImpl&&) = default;
        constexpr explicit BuilderImpl(const T& t) : value(t) {};
        constexpr explicit BuilderImpl(T&& t) : value(std::move(t)) {};
        constexpr T& get() & {
            return value;
        }
        constexpr T&& get() && {
            return std::move(value);
        }
        template<auto p>
        constexpr BuilderImpl<T>& set(const details::memPtrType<p>& v) & {
            value.*p = v;
            return *this;
        }
        template<auto p>
        constexpr BuilderImpl<T>& set(details::memPtrType<p>&& v) & {
            value.*p = std::move(v);
            return *this;
        }
        template<auto p>
        constexpr BuilderImpl<T>&& set(const details::memPtrType<p>& v) && {
            value.*p = v;
            return std::move(*this);
        }
        template<auto p>
        constexpr BuilderImpl<T>&& set(details::memPtrType<p>&& v) && {
            value.*p = std::move(v);
            return std::move(*this);
        }
};

template<typename T, auto ptr, auto... ptrs>
class BuilderImpl<T,ptr,ptrs...> : private BuilderImpl<T,ptrs...> {
    using parent = BuilderImpl<T,ptrs...>;
    using info = details::ReadMemPtr<ptr>;
    using infoT = std::remove_reference_t<typename info::type>;
    // static_assert(std::experimental::is_detected_v<details::canApplyMemPtr<ptr>::template check,T>, "mismatching member pointer types");
    
    template<typename,auto...>
    friend class BuilderImpl;
    
    template<auto... newptrs>
    using addInFront = BuilderImpl<T,newptrs...,ptr,ptrs...>;

    constexpr BuilderImpl(const parent& t) : parent(t) {};
    constexpr BuilderImpl(parent&& t) : parent(std::move(t)) {};

    template<auto p, typename Val>
    [[nodiscard]] constexpr auto set_nested(Val&& v) & {
        using returnType = std::remove_reference_t<decltype(std::declval<parent>().template set<p>(std::forward<Val>(v)))>;
        return typename returnType::template addInFront<ptr>(static_cast<parent>(*this).template set<p>(std::forward<Val>(v)));
    }
    template<auto p, typename Val>
    [[nodiscard]] constexpr auto set_nested(Val&& v) && {
        using returnType = std::remove_reference_t<decltype(std::declval<parent&&>().template set<p>(std::forward<Val>(v)))>;
        return typename returnType::template addInFront<ptr>{static_cast<parent&&>(std::move(*this)).template set<p>(std::forward<Val>(v))};
    }

public:
    constexpr BuilderImpl() = default;
    constexpr BuilderImpl(const BuilderImpl&) = default;
    constexpr BuilderImpl(BuilderImpl&&) = default;
    constexpr explicit BuilderImpl(const T& t) : parent(t) {};
    constexpr explicit BuilderImpl(T&& t) : parent(std::move(t)) {};

    template<int check = 1>
    constexpr T& get() {
        static_assert(check != 1, "Can't get from incompleted build");
    }
    
    //Simple case: the member set is the same as this BuilderImpl's, return parent with changed value.
    template<auto p, typename = std::enable_if_t<details::isEqualMemPtr_v<p,ptr>>>
    [[nodiscard]] constexpr parent& set(const infoT& val) & {
        this->getValue().*ptr = val;
        return *this;
    }
    template<auto p, typename = std::enable_if_t<details::isEqualMemPtr_v<p,ptr>>>
    [[nodiscard]] constexpr parent& set(infoT&& val) & {
        this->getValue().*ptr = std::move(val);
        return *this;
    }
    template<auto p, typename = std::enable_if_t<details::isEqualMemPtr_v<p,ptr>>>
    [[nodiscard]] constexpr parent&& set(const infoT& val) && {
        this->getValue().*ptr = val;
        return std::move(*this);
    }
    template<auto p, typename = std::enable_if_t<details::isEqualMemPtr_v<p,ptr>>>
    [[nodiscard]] constexpr parent&& set(infoT&& val) && {
        this->getValue().*ptr = std::move(val);
        return std::move(*this);
    }

    //More difficult case: the member set is not ours, push set through in case it is a required parameter
    template<auto p, typename = std::enable_if_t<!details::isEqualMemPtr_v<p,ptr>>>
    [[nodiscard]] constexpr auto set(const details::memPtrType<p>& v) & {
        return set_nested<p>(v);
    }
    template<auto p, typename = std::enable_if_t<!details::isEqualMemPtr_v<p,ptr>>>
    [[nodiscard]] constexpr auto set(details::memPtrType<p>&& v) & {
        return set_nested<p>(std::move(v));
    }
    template<auto p, typename = std::enable_if_t<!details::isEqualMemPtr_v<p,ptr>>>
    [[nodiscard]] constexpr auto set(const details::memPtrType<p>& v) && {
        return std::move(*this).template set_nested<p>(v);
    }
    template<auto p, typename = std::enable_if_t<!details::isEqualMemPtr_v<p,ptr>>>
    [[nodiscard]] constexpr auto set(details::memPtrType<p>&& v) && {
        return std::move(*this).template set_nested<p>(std::move(v));
    }
};

}

using namespace builder;

struct P2{
    int x;
    int y;
    using Builder = BuilderImpl<P2, &P2::x, &P2::y>;
};

static constexpr auto r = P2::Builder().set<&P2::y>(1).set<&P2::x>(2).get();
// static constexpr auto r2 = P2::Builder().set<&P2::x>(1).get();