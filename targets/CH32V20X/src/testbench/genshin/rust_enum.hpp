#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"


#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/wireless/Radio/LT8960/LT8960L.hpp"

#include "core/utils/typetraits/function_traits.hpp"
#include "core/polymorphism/reflect.hpp"
#include "digipw/SVPWM/svpwm3.hpp"

using namespace ymd;

template<class... Fs> struct overload : Fs... { using Fs::operator()...; };
template<class... Fs> overload(Fs...) -> overload<Fs...>;



template<class... Ts>
struct matcher
{
    std::tuple<Ts...> vs;
    template<class... Vs> constexpr matcher(Vs&&... vs) : vs(std::forward<Vs>(vs)...) {}
    template<class Fs> constexpr auto operator->*(Fs&& f) const
    {
        auto curry = [&](auto&&... vs) { return std::visit(std::forward<Fs>(f), vs...); };
	return std::apply(curry, std::move(vs));
    }
};
template<class... Ts> matcher(Ts&&...)->matcher<Ts&&...>;
#define Match(...) matcher{__VA_ARGS__} ->* overload

namespace ymd{

// enum class Shape{
//     Circle,
//     Square
// };


// using Circle = std::tuple<int>;
// using Rectangle = std::tuple<int, int>;

template<typename ... Ts>
class SumtypeEnum{
public:
    using Self = SumtypeEnum<Ts...>;
    // using Ts...;

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr SumtypeEnum(Raw && val):
        value_(std::in_place_type<T>, static_cast<T>(val)) {
    }

    
    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)

    constexpr bool is() const{
        return std::holds_alternative<T>(value_);
    }

    template<typename T>
    constexpr Option<T &> as() {
        if(! this->is<T>()) return None;
        return Some<T *>(&std::get<T>(value_)); 
    }

    template<typename T>
    constexpr Option<const T &> as() const {
        if(! this->is<T>()) return None;
        return Some<const T *>(&std::get<T>(value_)); 
    }


    constexpr bool operator ==(const Self & other) const {
        return this->value_ == other.value_;
    }

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr bool operator ==(const Raw &rhs) const {
        if(not this->is<T>()) return false;
        return this->as<T>().unwrap() == static_cast<T>(rhs);
    }

    template<typename Raw, typename T = magic::first_convertible_arg_t<Raw, Ts...>>
    requires (!std::is_void_v<T>)
    constexpr bool operator !=(const Raw &rhs) const {
        return !(this->operator ==(rhs));
    }

    template<typename Fn, typename Ret = magic::functor_ret_t<Fn>>
    Ret visit(Fn && fn) const {
        auto & self = *this;
        return std::visit([&](const auto & value) {
            // using T = std::decay_t<decltype(value)>;

            std::forward<Fn>(fn)(value);
        }, self.value_);
    }

    friend OutputStream & operator <<(OutputStream & os,const SumtypeEnum & self){
        // 使用 std::visit 遍历 std::variant
        std::visit([&os](const auto& value) {
            using T = std::decay_t<decltype(value)>;

            // 检查类型是否可被 OutputStream 打印
            if constexpr (requires(OutputStream& os, const T& value) {os << value;}) {
                os << value; // 如果可打印，则直接打印
            } else {
                os << "[Unprintable type]"; // 否则打印提示信息
            }
        }, self.value_);

        return os;
    }
private:
    std::variant<Ts...> value_;

    constexpr size_t var_index() const {
        return value_.index();
    }
};


template <template <typename...> class Base, typename Derived>
concept is_template_base_of = requires {
    []<typename... Ts>(const Base<Ts...>*) {}(static_cast<Derived*>(nullptr));
};

// using Shape = SumtypeEnum<Circle, Rectangle>;

struct Circle{int radius;};
struct Rectangle{int a; int b;};
struct Square{int a;};

struct Shape:public SumtypeEnum<Circle, Rectangle>{
    using Circle = ::Circle;
    using Rectangle = ::Rectangle;
    using Square = ::Square;

    
    // struct Circle{int radius;};
    // struct Rectangle{int a; int b;};

};

static_assert(std::is_same_v<Circle, Shape::Circle>);


}


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


//描述了一个匹配的键值对 第一个模板参数为待匹配的模式的类型 第二个模板参数为右表达式的类型
template <typename Pattern, typename Rhs>
class PatternEquation{
public:
    constexpr PatternEquation(Pattern const &pattern, Rhs const &rhs)
        : pattern_{pattern}, rhs_{rhs} {}

    static constexpr bool rhs_is_functor = magic::is_functor_v<Rhs>;
    // static_assert(magic::is_functor_v<int> == false);
    // using rhs_args_tuple = std::conditional_t<
    //     // rhs_is_functor,
    //     magic::is_functor_v<Rhs>,
    //     tuple_decay_t<magic::functor_args_tuple_t<Rhs>>,
    //     // Rhs,
    //     // tuple_decay_t<magic::functor_args_tuple_t<Rhs>>
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
    //     using Params = tuple_decay_t<magic::functor_args_tuple_t<Rhs>>;
    //     if constexpr (std::is_same_v<Params, std::tuple<void>>) return rhs_();
    //     else if constexpr (std::is_same_v<Params, std::tuple<std::decay_t<Obj>>>) 
    //         return rhs_(std::forward<Obj>(obj));
    //     else {
    //         __builtin_abort();
    //         // static_assert(magic::false_v<std::pair<Obj, Params>>);
    //         // static_assert(magic::false_v<Params>);
    //         return None;
    //     }
    // }
    template<typename Obj>
    requires (rhs_is_functor 
        and (not std::is_same_v<tuple_decay_t<magic::functor_args_tuple_t<Rhs>>, std::tuple<void>>) 
        and (not std::is_same_v<tuple_decay_t<magic::functor_args_tuple_t<Rhs>>, std::tuple<std::decay_t<Obj>>>))
    constexpr auto execute(Obj && obj) const { 
        // static_assert(magic::false_v<std::pair<std::decay_t<Obj>, tuple_decay_t<magic::functor_args_tuple_t<Rhs>>>>, "unsupported para")
        return magic::functor_ret_t<Rhs>();
    }

    template<typename Obj>
    requires (rhs_is_functor and (std::is_same_v<tuple_decay_t<magic::functor_args_tuple_t<Rhs>>, std::tuple<void>>))
    constexpr auto execute(Obj && obj) const { 
        return rhs_();
    }


    template<typename Obj>
    requires (rhs_is_functor and (std::is_same_v<tuple_decay_t<magic::functor_args_tuple_t<Rhs>>, std::tuple<std::decay_t<Obj>>>))
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
    constexpr auto operator=(Func &&func){
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
requires magic::is_functor_v<T>
constexpr PatternLhs<std::decay_t<T>> operator |(PatternKeyword, T && other){
    static_assert(magic::false_v<T>);
    return PatternLhs<std::decay_t<T>>(std::forward<T>(other));
}

// template<typename T>
// struct KeyWordCrtp{

// };

namespace keywords{

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
// struct Keyword_Is_Dispatcher<T, SumtypeEnum<Args ...>>{
//     using Obj = SumtypeEnum<Args ...>;

//     static constexpr bool is_compatible(const Obj & obj){
//         return obj.template is<T>();
//     }
// };
template<typename T, typename Obj>
requires is_template_base_of<SumtypeEnum, Obj>
struct Keyword_Is_Dispatcher<T, Obj>{
    // using Obj = SumtypeEnum<Args ...>;

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



// static constexpr bool is_compatible(const T & )
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




namespace ymd::fp{
// constexpr auto pattern_as_int = pattern | keywords::as<int>;
// constexpr auto pattern_is_int = pattern | keywords::is<int>;
// // constexpr auto pattern_as_int = pattern | keywords::as<int>;
// constexpr auto pattern_as_int_e3 = pattern | keywords::as<int> = 3;
// constexpr auto pattern_as_int_f1 = pattern | keywords::as<int> = []{return 1;};
// constexpr auto rf1 = pattern_as_int_f1.execute();
// constexpr auto re3 = pattern_as_int_e3.execute();

constexpr auto me = match(1);

constexpr auto shape = Shape(Shape::Circle(1));
// constexpr auto shape = Circle(1);
constexpr auto is_circle = (pattern | keywords::is<Circle>).is_compatible(shape);
// constexpr auto is_circle = (pattern | keywords::unhandled).is_compatible(shape);
constexpr auto is_rectangle = (pattern | keywords::is<Rectangle>).is_compatible(shape);

// constexpr auto reflection = match(Rectangle(1,2))(
// constexpr auto reflection = match(Shape::Circle(1))(
// constexpr auto reflection = match(Shape::Rectangle(2,2))(
constexpr auto reflection = match(Shape::Square(1))(
    pattern | keywords::is<Circle> = [](Circle circle){return circle.radius;},
    // pattern | keywords::is<Circle> = 1,
    // pattern | keywords::is<Circle> = 1,
    pattern | keywords::is<Rectangle> = [](const Rectangle & rect){return rect.a + rect.b;},
    // pattern | keywords::is<Rectangle> = 4,
    pattern | keywords::is<Square> = 4
);

static_assert(is_circle, "");
// static_assert(reflection == 2, "");
static_assert(reflection == 4, "");
// static_assert(reflection == 1, "");



// static_assert(is_rectangle, "");
}
}