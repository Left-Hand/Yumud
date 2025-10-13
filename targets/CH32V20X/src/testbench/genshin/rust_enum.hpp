#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/utils/sumtype.hpp"
#include "core/utils/Match.hpp"
#include "digipw/SVPWM/svpwm3.hpp"

using namespace ymd;

// template<class... Fs> struct overload : Fs... { using Fs::operator()...; };
// template<class... Fs> overload(Fs...) -> overload<Fs...>;



// template<class... Ts>
// struct matcher
// {
//     std::tuple<Ts...> vs;
//     template<class... Vs> constexpr matcher(Vs&&... vs) : vs(std::forward<Vs>(vs)...) {}
//     template<class Fs> constexpr auto operator->*(Fs&& f) const
//     {
//         auto curry = [&](auto&&... vs) { return std::visit(std::forward<Fs>(f), vs...); };
// 	return std::apply(curry, std::move(vs));
//     }
// };
// template<class... Ts> matcher(Ts&&...)->matcher<Ts&&...>;
// #define Match(...) matcher{__VA_ARGS__} ->* overload

namespace ymd{


// using Shape = Sumtype<Circle, Rectangle>;
struct Shape_Prelude{
    struct Circle{int radius;};
    struct Rectangle{int a; int b;};
    struct Square{int a;};
};

struct Shape:public Sumtype<Shape_Prelude::Circle, Shape_Prelude::Rectangle>, public Shape_Prelude
{    
    // struct Circle{int radius;};
    // struct Rectangle{int a; int b;};

};

using namespace fp;
// constexpr auto pattern_as_int = pattern | fpm::as<int>;
// constexpr auto pattern_is_int = pattern | fpm::is<int>;
// // constexpr auto pattern_as_int = pattern | fpm::as<int>;
// constexpr auto pattern_as_int_e3 = pattern | fpm::as<int> = 3;
// constexpr auto pattern_as_int_f1 = pattern | fpm::as<int> = []{return 1;};
// constexpr auto rf1 = pattern_as_int_f1.execute();
// constexpr auto re3 = pattern_as_int_e3.execute();


constexpr auto shape = Shape(Shape::Circle(1));
// constexpr auto shape = Circle(1);
constexpr auto is_circle = (pattern | fpm::is<Shape::Circle>).is_compatible(shape);
// constexpr auto is_circle = (pattern | fpm::unhandled).is_compatible(shape);
constexpr auto is_rectangle = (pattern | fpm::is<Shape::Rectangle>).is_compatible(shape);

// constexpr auto reflection = match(Rectangle(1,2))(
// constexpr auto reflection = match(Shape::Circle(1))(
// constexpr auto reflection = match(Shape::Rectangle(2,2))(
constexpr auto reflection = match(Shape::Square(1))(
    pattern | fpm::is<Shape::Circle> >>= [](const Shape::Circle & circle){return circle.radius;},
    // pattern | fpm::is<Circle> = 1,
    // pattern | fpm::is<Circle> = 1,
    pattern | fpm::is<Shape::Rectangle> >>= [](const Shape::Rectangle & rect){return rect.a + rect.b;},
    // pattern | fpm::is<Rectangle> = 4,
    pattern | fpm::is<Shape::Square> >>= 4
);

static_assert(is_circle, "");
// static_assert(reflection == 2, "");
static_assert(reflection == 4, "");
// static_assert(reflection == 1, "");

static_assert(std::is_same_v<Shape::Circle, Shape::Circle>);


}

