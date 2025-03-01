#pragma once

#include <cstdint>

namespace CH32L103{

struct R32_DMA_INTFR{
    volatile uint32_t DATA;

    constexpr void clear_transmit_done_flag(const uint8_t index){
        DATA = (1 << (1 + index * 4));
    }

    constexpr void get_transmit_done_flag(const uint8_t index){
        DATA & (1 << (1 + index * 4));
    }

    constexpr void clear_transmit_onhalf_flag(const uint8_t index){
        DATA = (1 << (2 + index * 4));
    }

    constexpr void get_transmit_onhalf_flag(const uint8_t index){
        DATA & (1 << (2 + index * 4));
    }

    constexpr void clear_transmit_error_flag(const uint8_t index){
        DATA = (1 << (3 + index * 4));
    }
    constexpr void get_transmit_error_flag(const uint8_t index){
        DATA & (1 << (3 + index * 4));
    }
};

struct R32_DMA_INTENR{
};

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
};

using R32_DMA_CNTR = uint32_t;
using R32_DMA_PADDR = uint32_t;
using R32_DMA_MADDR = uint32_t;

struct DMA_CH_Def{
    volatile R32_DMA_CFGR INTFR;
    volatile R32_DMA_CNTR CNTR;
    volatile R32_DMA_PADDR PADDR;
    volatile R32_DMA_MADDR MADDR;

    constexpr void enable(const bool en){
        INTFR.EN = en;
    }

    constexpr void enable_transmit_done_interrupt(const bool en){
        INTFR.TCIE = en;
    }

    constexpr void enable_transmit_onhalf_interrupt(const bool en){
        INTFR.HTIE = en;
    }

    void enable_transmit_error_interrupt(const bool en){
        INTFR.TEIE = en;
    }

    void set_source_is_mem(const bool en){
        INTFR.DIR = en;
    }

    void enable_circular_mode(const bool en){
        INTFR.CIRC = en;
    }

    void enable_periph_increment(const bool en){
        INTFR.PINC = en;
    }

    void enable_mem_increment(const bool en){
        INTFR.MINC = en;
    }

    void set_periph_data_size_bytes(const uint8_t size){
        INTFR.PSIZE = size - 1;
    }

    void set_mem_data_size_bytes(const uint8_t size){
        INTFR.MSISE = size - 1;
    }

    void set_priority(const uint8_t prio){
        INTFR.PL = prio;
    }

    void enable_mem2mem(const bool en){
        INTFR.MEM2MEM = en;
    }

    void set_periph_address(const uint32_t addr){
        PADDR = addr;
    }

    void set_mem_address(const uint32_t addr){
        MADDR = addr;
    }

    void set_len(const uint16_t len){
        CNTR = len;
    }
};

struct DMA_Def{
    volatile R32_DMA_INTFR INTFR;
    volatile R32_DMA_INTENR INTENR;
    DMA_CH_Def CH[8];
};

}