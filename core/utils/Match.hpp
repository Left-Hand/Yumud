#pragma once

#include <tuple>
#include "core/utils/sumtype.hpp"
#include "core/tmp/bits/width.hpp"

namespace ymd::fp{

namespace details{

// template <typename T>
// class Nullary : public T{
// public: using T::operator();
// };

// template <typename T>
// constexpr auto nullary(T const &t){return Nullary<T>{t};}
// // template <typename T>
// // constexpr auto expr(Id<T> &id){return nullary([&]{ return *id; });}

// template <typename T>
// constexpr auto expr(T const &v){return nullary([&]{ return v; });}
// template <typename T>
// constexpr auto to_nullary(T &&v){
//     if constexpr (std::is_invocable_v<std::decay_t<T>>) return v;
//     else return expr(v);
// }
}

template<typename T>
struct _tuple_decay_t{};

template<typename ... Ts>
struct _tuple_decay_t<std::tuple<Ts...>>{
    using type = std::tuple<std::decay_t<Ts>...>;
};

template<typename ... Ts>
using tuple_decay_t = _tuple_decay_t<Ts...>::type;


template <template <typename...> class Base, typename Derived>
concept is_template_base_of = requires {
    []<typename... Ts>(const Base<Ts...>*) {}(static_cast<Derived*>(nullptr));
};


//描述了一个匹配的键值对 第一个模板参数为待匹配的模式的类型 第二个模板参数为右表达式的类型
template <typename Pattern, typename Rhs>
class PatternEquation{
public:
    constexpr PatternEquation(Pattern const &pattern, Rhs const &rhs)
        : pattern_{pattern}, rhs_{rhs} {}

    static constexpr bool rhs_is_functor = tmp::is_functor_v<Rhs>;
    // static_assert(tmp::is_functor_v<int> == false);
    // using rhs_args_tuple = std::conditional_t<
    //     // rhs_is_functor,
    //     tmp::is_functor_v<Rhs>,
    //     tuple_decay_t<tmp::functor_args_tuple_t<Rhs>>,
    //     // Rhs,
    //     // tuple_decay_t<tmp::functor_args_tuple_t<Rhs>>
    //     Rhs
    //     // Rhs
    //     // void
    // >;

    // static_assert(rhs_is_functor xor std::is_same_v<rhs_args_tuple, int> == true);

    template<typename Obj>
    constexpr bool is_compatible(const Obj & obj){
        return pattern_.is_compatible(obj);
    }

    // template<typename Obj>
    // requires (rhs_is_functor)
    // constexpr auto execute(Obj && obj) const { 
    //     using Params = tuple_decay_t<tmp::functor_args_tuple_t<Rhs>>;
    //     if constexpr (std::is_same_v<Params, std::tuple<void>>) return rhs_();
    //     else if constexpr (std::is_same_v<Params, std::tuple<std::decay_t<Obj>>>) 
    //         return rhs_(std::forward<Obj>(obj));
    //     else {
    //         __builtin_abort();
    //         // static_assert(tmp::false_v<std::pair<Obj, Params>>);
    //         // static_assert(tmp::false_v<Params>);
    //         return None;
    //     }
    // }
    template<typename Obj>
    requires (rhs_is_functor 
        and (not std::is_same_v<tuple_decay_t<tmp::functor_args_tuple_t<Rhs>>, std::tuple<void>>) 
        and (not std::is_same_v<tuple_decay_t<tmp::functor_args_tuple_t<Rhs>>, std::tuple<std::decay_t<Obj>>>))
    constexpr auto execute(Obj && obj) const { 
        // static_assert(tmp::false_v<std::pair<std::decay_t<Obj>, tuple_decay_t<tmp::functor_args_tuple_t<Rhs>>>>, "unsupported para")
        return tmp::functor_ret_t<Rhs>();
    }

    template<typename Obj>
    requires (rhs_is_functor and (std::is_same_v<tuple_decay_t<tmp::functor_args_tuple_t<Rhs>>, std::tuple<void>>))
    constexpr auto execute(Obj && obj) const { 
        return rhs_();
    }


    template<typename Obj>
    requires (rhs_is_functor and (std::is_same_v<tuple_decay_t<tmp::functor_args_tuple_t<Rhs>>, std::tuple<std::decay_t<Obj>>>))
    constexpr auto execute(Obj && obj) const { 
        return rhs_(std::forward<Obj>(obj));
        // return rhs_(obj);
    }

    template<typename Obj>
    requires (!rhs_is_functor)
    constexpr auto execute(Obj) const { 
        return rhs_;
    }

    // template<typename Dummy = void>
    // requires (rhs_is_functor
    //     and std::is_same_v<Rhs, std::tuple<void>>)
    // constexpr auto execute() const {
    //     return rhs_();
    // }


    // template<typename Obj, std::enable_if_t<
    //     rhs_is_functor
    //     and (not std::is_same_v<Rhs, std::tuple<void>>)
    // >>
    // constexpr auto execute(const Obj & obj) const {
    //     using Params = rhs_args_tuple;
    //     return rhs_();
    // }

private:
    const Pattern pattern_;

    struct RhsContainer{};
    const Rhs rhs_;
};


template<typename Pattern>
class PatternLhs{
public:
    using type = Pattern;

    explicit constexpr PatternLhs(const Pattern & t):
        t_(t){}

    template <typename Func>
    constexpr auto operator>>=(Func &&func){
        return PatternEquation<Pattern, Func>{t_, func};
    }

    template<typename Obj>
    static constexpr bool is_compatible(const Obj & obj){
        return Pattern::is_compatible(obj);
    }
private:
    const Pattern t_;
};

struct PatternKeyword{};


template<typename T>
constexpr PatternLhs<std::decay_t<T>> operator |(PatternKeyword, T && other){
    return PatternLhs<std::decay_t<T>>(std::forward<T>(other));
}

template<typename T>
requires tmp::is_functor_v<T>
constexpr PatternLhs<std::decay_t<T>> operator |(PatternKeyword, T && other){
    static_assert(tmp::false_v<T>);
    return PatternLhs<std::decay_t<T>>(std::forward<T>(other));
}


namespace primitive{

// template<typename T>
template<typename T>
struct Keyword_As{
    using type = T;

};

template<typename T, typename Obj>
struct Keyword_Is_Dispatcher{
    static constexpr bool is_compatible(const Obj & obj){
        return std::is_same_v<T, Obj>;
    }
};


template<typename T>
struct Keyword_Is{
    using type = T;

    template<typename Obj>
    static constexpr bool is_compatible(const Obj & obj){
        return Keyword_Is_Dispatcher<T, Obj>::is_compatible(obj);
    }
};

struct KeyWord_Unhandled{
    static constexpr bool is_compatible(auto){ return true;}
};



// template<typename T, typename ... Args>
// struct Keyword_Is_Dispatcher<T, Sumtype<Args ...>>{
//     using Obj = Sumtype<Args ...>;

//     static constexpr bool is_compatible(const Obj & obj){
//         return obj.template is<T>();
//     }
// };
template<typename T, typename Obj>
requires is_template_base_of<Sumtype, Obj>
struct Keyword_Is_Dispatcher<T, Obj>{
    // using Obj = Sumtype<Args ...>;

    static constexpr bool is_compatible(const Obj & obj){
        return obj.template is<T>();
    }
};


template<typename T>
static constexpr auto as = Keyword_As<T>{};

template<typename T>
static constexpr auto is = Keyword_Is<T>{};

static constexpr auto unhandled = KeyWord_Unhandled{};
}

template<typename T>
struct MatcherEntry{
    template<typename U>
    constexpr MatcherEntry(U && value) : value_(std::forward<U>(value)){}

    template<typename FirstEquation, typename ... RestEquation>
    constexpr auto operator()(FirstEquation first, RestEquation && ... rest){
        if(first.is_compatible(value_))
            return first.execute(value_);    

        if constexpr(sizeof...(RestEquation))
            return this->operator()(std::forward<RestEquation>(rest)...);
        else __builtin_abort();
    };

    // template<typename Fn, typename ... Args>
    // constexpr void operator ()(const T kase, Fn && fn, Args && ...args){
    //     if(val_ == kase)return std::forward<Fn>(fn)();    
    //     else if constexpr(sizeof...(Args)) return this->operator()(std::forward<Args>(args)...);
    // }

    // template<typename Fn, typename ... Args>
    // constexpr void operator ()(const _None_t, Fn && fn, Args && ...args){
    //     return std::forward<Fn>(fn)();    
    // }
private:
    T value_;

    struct Helper{

    };
};

// struct MatchProperty{
//     struct MatchAll{};

//     static constexpr inline auto match_all = MatchAll{};
// };

// struct MatchPropertys{
//     static constexpr bool match_all = true;
// };


static constexpr inline PatternKeyword pattern;


// template<typename ... Args>

template<typename T>
static constexpr auto match(const T & value){
    return MatcherEntry<T>{value};
}

namespace fpm = fp::primitive;
}

