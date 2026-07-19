#pragma once

#include "tamagawa_msgs.hpp"

namespace ymd::drivers::tamagawa{


template<typename Backend>
struct ClientApiFacade{
    using State = typename Backend::State;

    constexpr auto get_all_info(this auto && self) noexcept {
        return Backend::convert(self.state, req_msgs::GetAllInfo{});
    }

    constexpr auto get_abs(this auto && self) noexcept {
        return Backend::convert(self.state, req_msgs::GetAbs{});
    }

    constexpr auto get_abm(this auto && self) noexcept {
        return Backend::convert(self.state, req_msgs::GetAbm{});
    }

    constexpr auto clear_abm_and_fault(this auto && self) noexcept {
        return Backend::convert(self.state, req_msgs::ClearAbmAndFault{});
    }

    constexpr auto clear_abs(this auto && self) noexcept {
        return Backend::convert(self.state, req_msgs::ClearAbs{});
    }

    constexpr auto write_eeprom(this auto && self, 
        const uint8_t address, const uint8_t value
    ) noexcept {
        const auto msg = req_msgs::WriteEEprom{.address = address, .value = value};
        return Backend::convert(self.state, msg);
    }
    
    constexpr auto read_eeprom(this auto && self, const uint8_t address) noexcept {
        const auto msg = req_msgs::ReadEEprom{.address = address};
        return Backend::convert(self.state, msg);
    }
};

template<typename Backend>
struct ServerApiFacade{
    using State = typename Backend::State;


};

}