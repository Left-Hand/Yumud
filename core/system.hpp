#pragma once

#include "core/platform.hpp"

namespace ymd::sys{
    void preinit();
    
    __attribute__((noreturn))
    void abort();

    void trip();

    __attribute__((noreturn))
    void reset();

    void dump();

    void jumpto(const uint32_t addr);

    namespace chip{
        [[nodiscard]] uint64_t get_chip_id();
        [[nodiscard]] uint32_t get_chip_id_crc();
        [[nodiscard]] uint32_t get_flash_size();
        [[nodiscard]] uint64_t get_mac_address();
        [[nodiscard]] uint32_t get_dev_id();
        [[nodiscard]] uint32_t get_rev_id();
    };

    namespace exception{
        void disable_interrupt();
        void enable_interrupt();
        [[nodiscard]] bool is_interrupt_pending();
        [[nodiscard]] bool is_interrupt_acting();
        [[nodiscard]] uint8_t get_interrupt_depth();
    }

    namespace clock{
        [[nodiscard]] uint32_t get_sys_clk_freq();
        [[nodiscard]] uint32_t get_apb1_clk_freq();
        [[nodiscard]] uint32_t get_apb2_clk_freq();
        [[nodiscard]] uint32_t get_ahb_clk_freq();

        void set_apb1_clk_freq(const uint32_t ferq);
        void set_apb2_clk_freq(const uint32_t ferq);
        void set_ahb_clk_freq(const uint32_t ferq);

        void set_apb1_div(const uint8_t div);
        void set_apb2_div(const uint8_t div);
        void set_ahb_div(const uint8_t div);;
    };
};


