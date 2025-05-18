#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"


#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/wireless/Radio/LT8960/LT8960L.hpp"

#include "core/utils/SumtypeEnum.hpp"
#include "core/utils/Unit.hpp"
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

