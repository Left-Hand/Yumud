#pragma once

#include "core/platform.hpp"
#include "chipname.hpp"

namespace ymd::sys{
    void preinit();
    void reset();

    void exit();
    void halt();
    void jumpto(const uint32_t addr);

    namespace chip{
        uint64_t get_chip_id();
        uint32_t get_chip_id_crc();
        uint32_t get_flash_size();
        uint64_t get_mac_address();

        uint32_t get_dev_id();
        uint32_t get_rev_id();
    };

    namespace exception{
        void disable_interrupt();
        void enable_interrupt();
        bool is_interrupt_pending();
        bool is_intrrupt_acting();
        uint8_t get_interrupt_depth();
    }

    namespace clock{
        void delay_ms(const uint32_t ms);
        void delay_us(const uint32_t us);

        uint32_t get_system_freq();
        uint32_t get_apb1_freq();
        void set_apb1_freq(const uint32_t ferq);
        uint32_t get_apb2_freq();
        void set_apb2_freq(const uint32_t ferq);
        uint32_t get_ahb_freq();
        void set_ahb_freq(const uint32_t ferq);

        void set_apb1_div(const uint8_t div);
        void set_apb2_div(const uint8_t div);
        void set_ahb_div(const uint8_t div);;
    };
};


