#include "common.hpp"
#include <cstdint>

namespace ymd::cpu_core{


struct [[nodiscard]] CriticalSectionGuard final{
    explicit CriticalSectionGuard() : mask_(cpu_core::enter_critical()) {}
    ~CriticalSectionGuard() {cpu_core::exit_critical(mask_); }

    CriticalSectionGuard(const CriticalSectionGuard&) = delete;
    CriticalSectionGuard(const CriticalSectionGuard&&) = delete;
    void operator=(const CriticalSectionGuard&) = delete;
    void operator=(const CriticalSectionGuard&&) = delete;
private:
    uint32_t mask_;
};


#define CRITICAL_SECTION() if (CriticalSectionGuard __critical_section_context{}; true)

template<typename Fn>
static constexpr auto within_critical_section(Fn && fn) -> decltype(fn()){
    CRITICAL_SECTION(){
        return std::forward<Fn>(fn)();
    }
}

}