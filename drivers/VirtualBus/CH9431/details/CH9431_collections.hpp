#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/spi/spidrv.hpp"


namespace ymd::drivers{

struct CH9431_Prelude{
    enum class Error_Kind:uint8_t{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)



    template<typename T = void>
    using IResult = Result<T, Error>;
};

struct CH9431_Regs :public CH9431_Prelude{
    enum class WorkMode:uint8_t{
        // 000：正常工作模式； 
        // 001：配置模式； 
        // 010：仅监听模式； 
        // 011：环回模式； 
        // 100：浅休眠模式； 
        // 101：深休眠模式； 
        // 110: 固件更新模式

        Normal = 0b000,
        Config = 0b001,
        Listen = 0b010,
        Loopback = 0b011,
        LightSleep = 0b100,
        DeepSleep = 0b101,
        FirwareUpdate = 0b110,
        Reserved = 0b111
    };
    
    enum class ClockPrescale:uint8_t{ 
        // CLKPRE 
        // CLKOUT 引脚预分频比设置： 
        // 00：FCLKOUT = 20MHz； 
        // 01：FCLKOUT = 10MHz； 
        // 10：FCLKOUT = 8MHz； 
        // 11：FCLKOUT = 4MHz。 

        _20Mhz = 0b00,
        _10Mhz = 0b01,
        _8Mhz = 0b10,
        _4Mhz = 0b11,
    };

    enum class InterruptFlag:uint8_t{
        // 000：无中断； 
        // 001：出错中断； 
        // 010：唤醒中断； 
        // 011：TXB0 中断； 
        // 100：TXB1 中断； 
        // 101：TXB2 中断； 
        // 110：RXB0 中断； 
        // 111：RXB1 中断。

        None = 0b000,
        Error = 0b001,
        Wakeup = 0b010,
        TxB0 = 0b011,
        TxB1 = 0b100,
        TxB2 = 0b101,
        RxB0 = 0b110,
        RxB1 = 0b111
    };

    struct TimingConfig{
        uint8_t bs1;
        uint8_t bs2;
        uint8_t brp;
    };

    struct R8_SYSCTRL:public Reg8<>{
        ClockPrescale CLKRPE:2;
        uint8_t CLKEN:1;
        uint8_t OSM:1;
        uint8_t ABAT:1;
        WorkMode REQOP:3;
    }DEF_R8(sysctrl_reg)

    struct R8_SYSSTAT:public Reg8<>{
        uint8_t __RESV__:1;
        uint8_t ICOD:3;
        uint8_t __RESV2__:1;
        WorkMode OPMOD:3;
    }DEF_R8(sysstat_reg)

    struct R8_RXnIP:public Reg8<>{
        uint8_t B0BFM:1;
        uint8_t B1BFM:1;
        uint8_t B0BFE:1;
        uint8_t B1BFE:1;
        uint8_t B0BFS:1;
        uint8_t B1BFS:1;
        uint8_t __RESV__:2;
    }DEF_R8(rxnip_reg)

    struct R8_BTIMER1:public Reg8<>{
        uint8_t BRP:8;

        static constexpr uint8_t calc_brp(
            const uint32_t baudrate, 
            const uint8_t bs1, 
            const uint8_t bs2)
        {
            return uint8_t((20_MHz / (baudrate * (bs1 + bs2 + 3))) - 1);
        }
    }DEF_R8(btimer1_reg)

    struct R8_BTIMER2:public Reg8<>{
        uint8_t TS1:4;
        uint8_t SJW:2;
        uint8_t __RESV__:2;
    }DEF_R8(btimer2_reg)

    struct R8_BTIMER3:public Reg8<>{
        uint8_t TS:3;
        uint8_t __RESV__:5;
    }DEF_R8(btimer3_reg)


    struct R8_SYSINTE:public Reg8<>{
        uint8_t RX0IE:1;
        uint8_t RX1IE:1;
        uint8_t TX0IE:1;
        uint8_t TX1IE:1;
        uint8_t TX2IE:1;
        uint8_t ERRIE:1;
        uint8_t WAKIE:1;
        uint8_t MERIE:1;
    }DEF_R8(sysinte_reg)

        
    struct R8_SYSINTF:public Reg8<>{
        uint8_t RX0IF:1;
        uint8_t RX1IF:1;
        uint8_t TX0IF:1;
        uint8_t TX1IF:1;
        uint8_t TX2IF:1;
        uint8_t ERRIF:1;
        uint8_t WAKIF:1;
        uint8_t MERIF:1;
    }DEF_R8(btimer3_reg)

    struct R8_EFLAG:public Reg8<>{
        uint8_t EWARN:1;
        uint8_t EXWAR:1;
        uint8_t TXWAR:1;
        uint8_t RXEP:1;
        uint8_t TXEP:1;
        uint8_t TXBO:1;
        uint8_t RX0OVR:1;
        uint8_t RX1OVR:1;
    }DEF_R8(eflag_reg)

};
}

