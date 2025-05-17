#pragma once

#include "core/platform.hpp"

#include <functional>
#include <chrono>

using namespace std::chrono_literals;

namespace ymd{
using Milliseconds = std::chrono::milliseconds;
using Microseconds = std::chrono::microseconds;
using Nanoseconds = std::chrono::nanoseconds;

}

namespace ymd::clock{


Milliseconds millis(void);
Microseconds micros(void);
Nanoseconds nanos(void);

void delay(Milliseconds ms);
void delay(Microseconds us);
void delay(Nanoseconds ns);

namespace details{
template<typename Clock>
struct ClockCtrp{};
}

struct SystemClock final:public details::ClockCtrp<SystemClock>{
    using rep = int64_t;
    /// The period must be provided by the backend.
    using period = std::ratio<1,1000>;
    /// Alias for durations representable with this clock.
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<SystemClock>;
    /// The epoch must be provided by the backend.

    /// The time points of this clock cannot decrease, however the time between
    /// ticks of this clock may slightly vary due to sleep modes. The duration
    /// during sleep may be ignored or backfilled with another clock.
    static constexpr bool is_monotonic = true;
    static constexpr bool is_steady = false;

    /// The now() function may not move forward while in a critical section or
    /// interrupt. This must be provided by the backend.
    static constexpr bool is_free_running = false;

    /// The clock must stop while in halting debug mode.
    static constexpr bool is_stopped_in_halting_debug_mode = true;

    /// The now() function can be invoked at any time.
    static constexpr bool is_always_enabled = true;

    /// The now() function may work in non-maskable interrupt contexts (e.g.
    /// exception/fault handlers), depending on the backend. This must be provided
    /// by the backend.
    static constexpr bool is_nmi_safe = false;

    /// This is thread and IRQ safe. This must be provided by the backend.
    static time_point now() noexcept {
        return time_point(duration(millis()));
    }

    // /// This is purely a helper, identical to directly using std::chrono::ceil, to
    // /// convert a duration type which cannot be implicitly converted where the
    // /// result is rounded up.
    // template <class Rep, class Period>
    // static constexpr duration for_at_least(std::chrono::duration<Rep, Period> d) {
    //     return std::chrono::ceil<duration>(d);
    // }
};

inline auto now(){
    return SystemClock::now();
} 

}

namespace ymd{
void bindSystickCb(std::function<void(void)> && cb);
}



extern "C"{
void Systick_Init(void);
__interrupt void SysTick_Handler(void);
}