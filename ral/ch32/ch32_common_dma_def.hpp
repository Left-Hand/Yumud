#pragma once

#include <cstdint>
#include "core/io/regs.hpp"


#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace ymd::ral::ch32::common_dma{

struct [[nodiscard]] R32_DMA_INTFR{
    uint32_t BITS;

    // constexpr bool get_global_flag(const uint8_t index){
    //     const uint32_t temp = uint32_t(BITS);
    //     return  temp & (1 << (index * 4 - 4));
    // }

    // constexpr bool get_transfer_done_flag(const uint8_t index){
    //     const uint32_t temp = uint32_t(BITS);
    //     return temp & (1 << (index * 4 - 3));
    // }



    // constexpr bool get_transfer_onhalf_flag(const uint8_t index){
    //     const uint32_t temp = uint32_t(BITS);
    //     return temp & (1 << (index * 4 - 2));
    // }


    // constexpr bool get_transfer_error_flag(const uint8_t index){
    //     const uint32_t temp = uint32_t(BITS);
    //     return temp & (1 << (index * 4 - 1));
    // }
};ASSERT_REG_IS_32BIT(R32_DMA_INTFR)

struct [[nodiscard]] R32_DMA_INTFCR{
    uint32_t BITS;

    // constexpr void clear_global_flag(const uint8_t index){
    //     BITS = (1 << (index * 4 - 4));
    // }

    // constexpr void clear_transfer_done_flag(const uint8_t index){
    //     BITS = (1 << (index * 4 - 3));
    // }
    // constexpr void clear_transfer_onhalf_flag(const uint8_t index){
    //     BITS = (1 << (index * 4 - 2));
    // }

    // constexpr void clear_transfer_error_flag(const uint8_t index){
    //     BITS = (1 << (index * 4 - 1));
    // }

};ASSERT_REG_IS_32BIT(R32_DMA_INTFCR)

struct [[nodiscard]] R32_DMA_CFGR{
    uint32_t EN:1;
    uint32_t TCIE:1;
    uint32_t HTIE:1;
    uint32_t TEIE:1;
    uint32_t DIR:1;
    uint32_t CIRC:1;
    uint32_t PINC:1;
    uint32_t MINC:1;

    uint32_t PSIZE:2;
    uint32_t MSIZE:2;
    uint32_t PL:2;
    uint32_t MEM2MEM:1;

    uint32_t :17;
};ASSERT_REG_IS_32BIT(R32_DMA_CFGR)

using R32_DMA_CNTR = uint32_t;
using R32_DMA_PADDR = uint32_t;
using R32_DMA_MADDR = uint32_t;

struct [[nodiscard]] R32_DMA2_EXTEM_INTFR{
    volatile uint32_t BITS;
};

struct [[nodiscard]] DMA_CH_Def{
    volatile R32_DMA_CFGR CFGR;
    volatile R32_DMA_CNTR CNTR;
    volatile R32_DMA_PADDR PADDR;
    volatile R32_DMA_MADDR MADDR;

    constexpr void enable(const Enable en){
        CFGR.EN = en == EN;
    }

    constexpr void enable_transfer_done_interrupt(const Enable en){
        CFGR.TCIE = en == EN;
    }

    constexpr void enable_transfer_onhalf_interrupt(const Enable en){
        CFGR.HTIE = en == EN;
    }

    void enable_transfer_error_interrupt(const Enable en){
        CFGR.TEIE = en == EN;
    }

    void set_source_is_mem(const Enable en){
        CFGR.DIR = en == EN;
    }

    void enable_circular_mode(const Enable en){
        CFGR.CIRC = en == EN;
    }

    void enable_periph_increment(const Enable en){
        CFGR.PINC = en == EN;
    }

    void enable_mem_increment(const Enable en){
        CFGR.MINC = en == EN;
    }

    void set_priority(const uint8_t prio){
        CFGR.PL = prio;
    }

    void enable_mem2mem(const Enable en){
        CFGR.MEM2MEM = en == EN;
    }

    void set_periph_address(const uint32_t addr){
        PADDR = addr;
    }

    void set_mem_address(const uint32_t addr){
        MADDR = addr;
    }

    void set_data_len(const uint16_t len){
        CNTR = len;
    }
};
static_assert((sizeof(DMA_CH_Def) == 16)); 

struct [[nodiscard]] DMA1_Def{
    volatile R32_DMA_INTFR INTFR;
    volatile R32_DMA_INTFCR INTFCR;
    DMA_CH_Def CH[8];

    constexpr void clear_global_flag(const uint8_t index){
        INTFCR.BITS = (1 << (index * 4 - 4));
    }

    constexpr bool get_global_flag(const uint8_t index){
        return  std::bit_cast<uint32_t>(INTFR.BITS) & (1 << (index * 4 - 4));
    }

    constexpr void clear_transfer_done_flag(const uint8_t index){
        INTFCR.BITS = (1 << (index * 4 - 3));
    }

    constexpr bool get_transfer_done_flag(const uint8_t index){
        return std::bit_cast<uint32_t>(INTFR.BITS) & (1 << (index * 4 - 3));
    }

    constexpr void clear_transfer_onhalf_flag(const uint8_t index){
        INTFCR.BITS = (1 << (index * 4 - 2));
    }

    constexpr bool get_transfer_onhalf_flag(const uint8_t index){
        return std::bit_cast<uint32_t>(INTFR.BITS) & (1 << (index * 4 - 2));
    }

    constexpr void clear_transfer_error_flag(const uint8_t index){
        INTFCR.BITS = (1 << (index * 4 - 1));
    }
    constexpr bool get_transfer_error_flag(const uint8_t index){
        return std::bit_cast<uint32_t>(INTFR.BITS) & (1 << (index * 4 - 1));
    }
};

struct [[nodiscard]] DMA2_Def{
    volatile R32_DMA_INTFR INTFR;
    volatile R32_DMA_INTFCR INTFCR;
    DMA_CH_Def CH[12];
    volatile R32_DMA_INTFR EXTEM_INTFR;
    volatile R32_DMA_INTFCR EXTEM_INTFCR;

    constexpr void clear_global_flag(const uint8_t index){
        auto & reg = index > 7 ? EXTEM_INTFCR : INTFCR; 
        reg.BITS = (1 << ((index & 0b111) * 4 - 4));
    }

    constexpr bool get_global_flag(const uint8_t index){
        auto & reg = index > 7 ? EXTEM_INTFR : INTFR; 
        return  std::bit_cast<uint32_t>(reg.BITS) & (1 << ((index & 0b111) * 4 - 4));
    }
    constexpr void clear_transfer_done_flag(const uint8_t index){
        auto & reg = index > 7 ? EXTEM_INTFCR : INTFCR; 
        reg.BITS = (1 << ((index & 0b111) * 4 - 3));
    }

    constexpr bool get_transfer_done_flag(const uint8_t index){
        auto & reg = index > 7 ? EXTEM_INTFR : INTFR; 
        return std::bit_cast<uint32_t>(reg.BITS) & (1 << ((index & 0b111) * 4 - 3));
    }

    constexpr void clear_transfer_onhalf_flag(const uint8_t index){
        auto & reg = index > 7 ? EXTEM_INTFCR : INTFCR; 
        reg.BITS = (1 << ((index & 0b111) * 4 - 2));
    }

    constexpr bool get_transfer_onhalf_flag(const uint8_t index){
        auto & reg = index > 7 ? EXTEM_INTFR : INTFR; 
        return std::bit_cast<uint32_t>(reg.BITS) & (1 << ((index & 0b111) * 4 - 2));
    }

    constexpr void clear_transfer_error_flag(const uint8_t index){
        auto & reg = index > 7 ? EXTEM_INTFCR : INTFCR; 
        reg.BITS = (1 << ((index & 0b111) * 4 - 1));
    }
    constexpr bool get_transfer_error_flag(const uint8_t index){
        auto & reg = index > 7 ? EXTEM_INTFR : INTFR; 
        return std::bit_cast<uint32_t>(reg.BITS) & (1 << ((index & 0b111) * 4 - 1));
    }
};

}

namespace ymd::ral{
using namespace ch32::common_dma;
}