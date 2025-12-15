#pragma once

#include "core/arch/critical.hpp"
#include <atomic>

namespace ymd::sync{

struct [[nodiscard]] CriticalSectionRawMutex{

    template<typename Fn>
    static auto lock(Fn && fn){
        return cpu_core::within_critical_section(std::forward<Fn>(fn)());
    }
};

struct [[nodiscard]] NoopSectionRawMutex{

    template<typename Fn>
    static auto lock(Fn && fn){
        return std::forward<Fn>(fn)();
    }
};

// concept RawMutex = requires(RawMutex m){
//     m.lock();
// };

}