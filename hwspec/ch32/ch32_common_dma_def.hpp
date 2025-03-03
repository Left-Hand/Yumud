#pragma once

#include <cstdint>

#ifndef ASSERT_REG_IS_32BIT
#define ASSERT_REG_IS_32BIT(name) static_assert((sizeof(name) == 4),  #name " is not 4 bytes"); 
#endif

namespace CH32{

struct R32_DMA_INTFR{
    uint32_t DATA;

    // constexpr bool get_global_flag(const uint8_t index){
    //     const uint32_t temp = uint32_t(DATA);
    //     return  temp & (1 << (index * 4 - 4));
    // }

    // constexpr bool get_transfer_done_flag(const uint8_t index){
    //     const uint32_t temp = uint32_t(DATA);
    //     return temp & (1 << (index * 4 - 3));
    // }



    // constexpr bool get_transfer_onhalf_flag(const uint8_t index){
    //     const uint32_t temp = uint32_t(DATA);
    //     return temp & (1 << (index * 4 - 2));
    // }


    // constexpr bool get_transfer_error_flag(const uint8_t index){
    //     const uint32_t temp = uint32_t(DATA);
    //     return temp & (1 << (index * 4 - 1));
    // }
};ASSERT_REG_IS_32BIT(R32_DMA_INTFR)

struct R32_DMA_INTFCR{
    uint32_t DATA;

    // constexpr void clear_global_flag(const uint8_t index){
    //     DATA = (1 << (index * 4 - 4));
    // }

    // constexpr void clear_transfer_done_flag(const uint8_t index){
    //     DATA = (1 << (index * 4 - 3));
    // }
    // constexpr void clear_transfer_onhalf_flag(const uint8_t index){
    //     DATA = (1 << (index * 4 - 2));
    // }

    // constexpr void clear_transfer_error_flag(const uint8_t index){
    //     DATA = (1 << (index * 4 - 1));
    // }

};ASSERT_REG_IS_32BIT(R32_DMA_INTFCR)

struct R32_DMA_CFGR{
    uint32_t EN:1;
    uint32_t TCIE:1;
    uint32_t HTIE:1;
    uint32_t TEIE:1;
    uint32_t DIR:1;
    uint32_t CIRC:1;
    uint32_t PINC:1;
    uint32_t MINC:1;
    uint32_t PSIZE:2;
    uint32_t MSISE:2;
    uint32_t PL:2;
    uint32_t MEM2MEM:1;
    uint32_t :17;
};ASSERT_REG_IS_32BIT(R32_DMA_CFGR)

using R32_DMA_CNTR = uint32_t;
using R32_DMA_PADDR = uint32_t;
using R32_DMA_MADDR = uint32_t;

struct R32_DMA2_EXTEM_INTFR{
    volatile uint32_t DATA;
};

struct DMA_CH_Def{
    volatile R32_DMA_CFGR CFGR;
    volatile R32_DMA_CNTR CNTR;
    volatile R32_DMA_PADDR PADDR;
    volatile R32_DMA_MADDR MADDR;

    constexpr void enable(const bool en){
        CFGR.EN = en;
    }

    constexpr void enable_transfer_done_interrupt(const bool en){
        CFGR.TCIE = en;
    }

    constexpr void enable_transfer_onhalf_interrupt(const bool en){
        CFGR.HTIE = en;
    }

    void enable_transfer_error_interrupt(const bool en){
        CFGR.TEIE = en;
    }

    void set_source_is_mem(const bool en){
        CFGR.DIR = en;
    }

    void enable_circular_mode(const bool en){
        CFGR.CIRC = en;
    }

    void enable_periph_increment(const bool en){
        CFGR.PINC = en;
    }

    void enable_mem_increment(const bool en){
        CFGR.MINC = en;
    }

    void set_periph_data_size_bytes(const uint8_t size){
        CFGR.PSIZE = size - 1;
    }

    void set_mem_data_size_bytes(const uint8_t size){
        CFGR.MSISE = size - 1;
    }

    void set_priority(const uint8_t prio){
        CFGR.PL = prio;
    }

    void enable_mem2mem(const bool en){
        CFGR.MEM2MEM = en;
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
};static_assert((sizeof(DMA_CH_Def) == 16)); 

struct DMA1_Def{
    volatile R32_DMA_INTFR INTFR;
    volatile R32_DMA_INTFCR INTFCR;
    volatile DMA_CH_Def CH[8];

    constexpr void clear_global_flag(const uint8_t index){
        INTFCR.DATA = (1 << (index * 4 - 4));
    }

    constexpr bool get_global_flag(const uint8_t index){
        return  std::bit_cast<uint32_t>(INTFR.DATA) & (1 << (index * 4 - 4));
    }
    constexpr void clear_transfer_done_flag(const uint8_t index){
        INTFCR.DATA = (1 << (index * 4 - 3));
    }

    constexpr bool get_transfer_done_flag(const uint8_t index){
        return std::bit_cast<uint32_t>(INTFR.DATA) & (1 << (index * 4 - 3));
    }

    constexpr void clear_transfer_onhalf_flag(const uint8_t index){
        INTFCR.DATA = (1 << (index * 4 - 2));
    }

    constexpr bool get_transfer_onhalf_flag(const uint8_t index){
        return std::bit_cast<uint32_t>(INTFR.DATA) & (1 << (index * 4 - 2));
    }

    constexpr void clear_transfer_error_flag(const uint8_t index){
        INTFCR.DATA = (1 << (index * 4 - 1));
    }
    constexpr bool get_transfer_error_flag(const uint8_t index){
        return std::bit_cast<uint32_t>(INTFR.DATA) & (1 << (index * 4 - 1));
    }
};

struct DMA2_Def{
    volatile R32_DMA_INTFR INTFR;
    volatile R32_DMA_INTFCR INTFCR;
    volatile DMA_CH_Def CH[12];

    constexpr void clear_global_flag(const uint8_t index){
        INTFCR.DATA = (1 << (index * 4 - 4));
    }

    constexpr bool get_global_flag(const uint8_t index){
        return  std::bit_cast<uint32_t>(INTFR.DATA) & (1 << (index * 4 - 4));
    }
    constexpr void clear_transfer_done_flag(const uint8_t index){
        INTFCR.DATA = (1 << (index * 4 - 3));
    }

    constexpr bool get_transfer_done_flag(const uint8_t index){
        return std::bit_cast<uint32_t>(INTFR.DATA) & (1 << (index * 4 - 3));
    }

    constexpr void clear_transfer_onhalf_flag(const uint8_t index){
        INTFCR.DATA = (1 << (index * 4 - 2));
    }

    constexpr bool get_transfer_onhalf_flag(const uint8_t index){
        return std::bit_cast<uint32_t>(INTFR.DATA) & (1 << (index * 4 - 2));
    }

    constexpr void clear_transfer_error_flag(const uint8_t index){
        INTFCR.DATA = (1 << (index * 4 - 1));
    }
    constexpr bool get_transfer_error_flag(const uint8_t index){
        return std::bit_cast<uint32_t>(INTFR.DATA) & (1 << (index * 4 - 1));
    }
};

}