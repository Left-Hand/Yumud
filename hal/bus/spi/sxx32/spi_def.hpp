#pragma once

#include <cstdint>

namespace SXX32{
    namespace SPI_Regs{
        struct CTLR1_Reg{
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


        struct CTLR2_Reg{
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
        struct STATR_Reg{
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

        struct DATAR_Reg{
            uint16_t DR:16;
        };

        struct CRCR_Reg{
            uint16_t CRCPOLY:16;
        };

        struct RCRCR_Reg{
            uint16_t RXCRC:16;
        };

        struct TCRCR_Reg{
            uint16_t TXCRC:16;
        };

        //SPI 时钟寄存器
        struct CFGR_Reg{
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

        struct PR_Reg{
            uint16_t I2SDIV:8;
            uint16_t ODD:1;
            uint16_t MCKOE:1;
            uint16_t :6;
        };

        struct HSCR_Reg{
            uint16_t HSRXEN:1;
            uint16_t :15;
        };

        struct SPI_Def{
            CTLR1_Reg CTLR1;
            CTLR2_Reg CTLR2;
            STATR_Reg STATR;
            DATAR_Reg DATAR;
            CRCR_Reg CRCR;
            RCRCR_Reg RCRCR;
            TCRCR_Reg TCRCR;
            CFGR_Reg CFGR;
            HSCR_Reg HSCR;
        };


        static inline volatile SPI_Def * SPI1 = (volatile SPI_Def *)(0x40013000);
        static inline volatile SPI_Def * SPI2 = (volatile SPI_Def *)(0x40003800);
        static inline volatile SPI_Def * SPI3 = (volatile SPI_Def *)(0x40003C00);

    }
}