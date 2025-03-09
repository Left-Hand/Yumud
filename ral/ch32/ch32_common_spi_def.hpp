#pragma once

#include <cstdint>

#ifndef BIT_CAST
#define BIT_CAST(type, source) __builtin_bit_cast(type, (source))
#endif

namespace CH32V20x{
    struct R16_SPI_CTLR1{
        uint16_t CPHA:1;
        uint16_t CPOL:1;
        uint16_t MSTR:1;
        uint16_t BR:3;
        uint16_t SPE:1;
        uint16_t LSB:1;

        uint16_t SSI:1;
        uint16_t SSM:1;
        uint16_t RXONLY:1;
        uint16_t DFF:1;
        uint16_t CRCNEXT:1;
        uint16_t CRCEN:1;
        uint16_t BIDIOE:1;
        uint16_t BIDIMODE:1;
    };


    struct R16_SPI_CTLR2{
        uint16_t RXDMAEN:1;
        uint16_t TXDMAEN:1;
        uint16_t SSOE:1;
        uint16_t :2;
        uint16_t ERRIR:1;
        uint16_t RXNEIE:1;
        uint16_t TXEIE:1;
        uint16_t :8;
    };



    //SPI 状态寄存器1
    struct R16_SPI_STATR{
        uint16_t RXNE:1;
        uint16_t TXE:1;
        uint16_t CHSIDE:1;
        uint16_t UDR:1;
        uint16_t CRCEER:1;
        uint16_t MODF:1;
        uint16_t OVR:1;
        uint16_t BSY:1;
        uint16_t :8;
    };

    struct R16_SPI_DATAR{
        uint16_t DR:16;
    };

    struct R16_SPI_CRCR{
        uint16_t CRCPOLY:16;
    };

    struct R16_SPI_RCRCR{
        uint16_t RXCRC:16;
    };

    struct R16_SPI_TCRCR{
        uint16_t TXCRC:16;
    };

    //SPI 时钟寄存器
    struct R16_SPI_CFGR{
        uint16_t CHLEN:1;
        uint16_t DATLEN:2;
        uint16_t CKPOL:1;

        uint16_t I2SSTD:2;
        uint16_t :1;
        uint16_t PCMSYNC:1;

        uint16_t I2SCFG:2;
        uint16_t ISSE:1;
        uint16_t I2SMOD:1;
        uint16_t :4;
    };

    struct R16_SPI_PR{
        uint16_t I2SDIV:8;
        uint16_t ODD:1;
        uint16_t MCKOE:1;
        uint16_t :6;
    };

    struct R16_SPI_HSCR{
        uint16_t HSRXEN:1;
        uint16_t :15;
    };

    struct SPI_Def{
        volatile R16_SPI_CTLR1 CTLR1;
        uint16_t :16;
        volatile R16_SPI_CTLR2 CTLR2;
        uint16_t :16;
        volatile R16_SPI_STATR STATR;
        uint16_t :16;
        volatile R16_SPI_DATAR DATAR;
        uint16_t :16;
        volatile R16_SPI_CRCR CRCR;
        uint16_t :16;
        volatile R16_SPI_RCRCR RCRCR;
        uint16_t :16;
        volatile R16_SPI_TCRCR TCRCR;
        uint16_t :16;
        volatile R16_SPI_CFGR CFGR;
        uint16_t :16;
        volatile R16_SPI_HSCR HSCR;
        uint16_t :16;

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

        constexpr void init_spi(){
        }

        constexpr void init_i2s(){

        }
        constexpr void enable_spi(const bool en){
            CTLR1.SPE = en;
        }

        constexpr void enable_i2c(const bool en){
            CFGR.ISSE = en;
        }

        constexpr void enable_dma_txconst (const bool en){
            CTLR2.TXDMAEN = en;
        }

        constexpr void enable_dma_rx(const bool en){
            CTLR2.RXDMAEN = en;
        }

        constexpr void send(const uint16_t data){
            DATAR.DR = data;
        }

        constexpr uint16_t receive(){
            return DATAR.DR;
        }

        constexpr void enable_soft_cs(const bool en){
            CTLR1.SSI = en;
        }

        constexpr void enable_ss_output(const bool en){
            CTLR2.SSOE = en;
        }

        constexpr void enable_dualbyte(const bool en){
            CTLR1.DFF = en;
        }

        constexpr void transmit_crc(){
            CTLR1.CRCNEXT = 1;
        }

        constexpr void enable_crc(const bool en){
            CTLR1.CRCNEXT = 1;
        }

        constexpr uint16_t get_rx_crc(){
            return RCRCR.RXCRC;
        }

        constexpr uint16_t get_tx_crc(){
            return TCRCR.TXCRC;
        }

        constexpr uint16_t get_crc_polynomial(){
            return CRCR.CRCPOLY;
        }

        constexpr void switch_bidi_to_txonly(){
            CTLR1.BIDIOE = 1;
        }

        constexpr void switch_bidi_to_rxonly(){
            CTLR1.BIDIOE = 0;
        }

        constexpr void enable_bidi(const bool en){
            CTLR1.BIDIMODE = en;
        }

        constexpr Events get_events(){
            return BIT_CAST(Events, uint8_t(BIT_CAST(uint16_t, STATR)));
        }

        constexpr void clear_events(const Events events){
            uint16_t mask = BIT_CAST(uint8_t, events);
            const_cast<R16_SPI_STATR &>(STATR) = 
                BIT_CAST(R16_SPI_STATR, 
                    uint16_t((~mask) & BIT_CAST(uint16_t, STATR))
                );
        }
    };


    static inline SPI_Def * SPI1 = (SPI_Def *)(0x40013000);
    static inline SPI_Def * SPI2 = (SPI_Def *)(0x40003800);
    static inline SPI_Def * SPI3 = (SPI_Def *)(0x40003C00);

}