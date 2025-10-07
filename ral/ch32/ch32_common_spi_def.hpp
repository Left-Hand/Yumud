#pragma once

#include <cstdint>
#include "core/io/regs.hpp"

#ifndef BIT_CAST
#define BIT_CAST(type, source) __builtin_bit_cast(type, (source))
#endif

namespace ymd::ral::CH32V20x{
    struct R32_SPI_CTLR1{
        uint32_t CPHA:1;
        uint32_t CPOL:1;
        uint32_t MSTR:1;
        uint32_t BR:3;
        uint32_t SPE:1;
        uint32_t LSB:1;

        uint32_t SSI:1;
        uint32_t SSM:1;
        uint32_t RXONLY:1;
        uint32_t DFF:1;
        uint32_t CRCNEXT:1;
        uint32_t CRCEN:1;
        uint32_t BIDIOE:1;
        uint32_t BIDIMODE:1;
        uint32_t :16;
    };CHECK_R32(R32_SPI_CTLR1);


    struct R32_SPI_CTLR2{
        uint32_t RXDMAEN:1;
        uint32_t TXDMAEN:1;
        uint32_t SSOE:1;
        uint32_t :2;
        uint32_t ERRIR:1;
        uint32_t RXNEIE:1;
        uint32_t TXEIE:1;
        uint32_t :24;
    };CHECK_R32(R32_SPI_CTLR2);



    //SPI 状态寄存器1
    struct R32_SPI_STATR{
        uint32_t RXNE:1;
        uint32_t TXE:1;
        uint32_t CHSIDE:1;
        uint32_t UDR:1;

        uint32_t CRCEER:1;
        uint32_t MODF:1;
        uint32_t OVR:1;
        uint32_t BSY:1;

        uint32_t :24;
    };CHECK_R32(R32_SPI_STATR);

    struct R32_SPI_DATAR{
        uint32_t DR;
    };

    struct R32_SPI_CRCR{
        uint32_t CRCPOLY;
    };

    struct R32_SPI_RCRCR{
        uint32_t RXCRC;
    };

    struct R32_SPI_TCRCR{
        uint32_t TXCRC;
    };

    //SPI 时钟寄存器
    struct R32_SPI_CFGR{
        uint32_t CHLEN:1;
        uint32_t DATLEN:2;
        uint32_t CKPOL:1;

        uint32_t I2SSTD:2;
        uint32_t :1;
        uint32_t PCMSYNC:1;

        uint32_t I2SCFG:2;
        uint32_t ISSE:1;
        uint32_t I2SMOD:1;
        uint32_t :20;
    };

    struct R32_SPI_PR{
        uint32_t I2SDIV:8;
        uint32_t ODD:1;
        uint32_t MCKOE:1;
        uint32_t :22;
    };

    struct R32_SPI_HSCR{
        uint32_t HSRXEN:1;
        uint32_t :31;
    };

    struct SPI_Def{
        volatile R32_SPI_CTLR1 CTLR1;
        volatile R32_SPI_CTLR2 CTLR2;
        volatile R32_SPI_STATR STATR;
        volatile R32_SPI_DATAR DATAR;
        volatile R32_SPI_CRCR CRCR;
        volatile R32_SPI_RCRCR RCRCR;
        volatile R32_SPI_TCRCR TCRCR;
        volatile R32_SPI_CFGR CFGR;
        volatile R32_SPI_HSCR HSCR;

        struct Events{
            uint8_t RXNE:1;
            uint8_t TXE:1;
            uint8_t CHSIDE:1;
            uint8_t UDR:1;
            uint8_t CRCEER:1;
            uint8_t MODF:1;
            uint8_t OVR:1;
            uint8_t BSY:1;
        };



        constexpr void enable_spi(const Enable en){
            CTLR1.SPE = en == EN;
        }

        constexpr void enable_i2s(const Enable en){
            CFGR.ISSE = en == EN;
        }

        constexpr void enable_dma_tx(const Enable en){
            CTLR2.TXDMAEN = en == EN;
        }

        constexpr void enable_dma_rx(const Enable en){
            CTLR2.RXDMAEN = en == EN;
        }

        constexpr void send(const uint32_t data){
            DATAR.DR = data;
        }

        constexpr uint32_t receive(){
            return std::bit_cast<uint32_t>(DATAR.DR);
        }

        constexpr void enable_soft_cs(const Enable en){
            CTLR1.SSI = en == EN;
        }

        constexpr void enable_ss_output(const Enable en){
            CTLR2.SSOE = en == EN;
        }

        constexpr void enable_dualbyte(const Enable en){
            CTLR1.DFF = en == EN;
        }

        constexpr void transmit_crc(){
            CTLR1.CRCNEXT = 1;
        }

        constexpr void enable_crc(const Enable en){
            CTLR1.CRCNEXT = 1;
        }

        constexpr uint32_t get_rx_crc(){
            return std::bit_cast<uint32_t>(RCRCR.RXCRC);
        }

        constexpr uint32_t get_tx_crc(){
            return std::bit_cast<uint32_t>(TCRCR.TXCRC);
        }

        constexpr uint32_t get_crc_polynomial(){
            return std::bit_cast<uint32_t>(CRCR.CRCPOLY);
        }

        constexpr void switch_bidi_to_txonly(){
            CTLR1.BIDIOE = 1;
        }

        constexpr void switch_bidi_to_rxonly(){
            CTLR1.BIDIOE = 0;
        }

        constexpr void enable_bidi(const Enable en){
            CTLR1.BIDIMODE = en == EN;
        }

        constexpr Events get_events(){
            return BIT_CAST(Events, uint8_t(std::bit_cast<uint32_t>(STATR)));
        }

        constexpr void clear_events(const Events events){
            uint32_t mask = BIT_CAST(uint8_t, events);
            const_cast<R32_SPI_STATR &>(STATR) = 
                BIT_CAST(R32_SPI_STATR, 
                    uint32_t((~mask) & std::bit_cast<uint32_t>(STATR))
                );
        }
    };


[[maybe_unused]] static inline SPI_Def * SPI1_Inst = reinterpret_cast<SPI_Def *>(0x40013000);
[[maybe_unused]] static inline SPI_Def * SPI2_Inst = reinterpret_cast<SPI_Def *>(0x40003800);
[[maybe_unused]] static inline SPI_Def * SPI3_Inst = reinterpret_cast<SPI_Def *>(0x40003C00);

}