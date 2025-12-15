#pragma once

#include <cstdint>
#include "core/io/regs.hpp"



namespace ymd::drivers::ch9434::details{

enum class RegAddr:uint8_t{
    RHR = 0x00,
    THR = 0x00,
    IER = 0x01,
    IIR = 0x02,
    FCR = 0x02,
    LCR = 0x03,
    MCR = 0x04,
    LSR = 0x05,
    MSR = 0x06,
    SPR = 0x07,
    CLK = 0x48,
    RS485 = 0x41,
    FIFO = 0x42,
    FIFOCL = 0x43,
    FIFOCH = 0x44,

    GPIO_EN_0 = 0x50,
    GPIO_EN_1 = 0x51,
    GPIO_EN_2 = 0x52,
    GPIO_EN_3 = 0x53,

    
    GPIO_DIR_0 = 0x54,
    GPIO_DIR_1,
    GPIO_DIR_2,
    GPIO_DIR_3,

    GPIO_PU_0 = 0x58,
    GPIO_PU_1,
    GPIO_PU_2,
    GPIO_PU_3,

    GPIO_PD_0 = 0x5c,
    GPIO_PD_1,
    GPIO_PD_2,
    GPIO_PD_3,

    GPIO_VAL_0 = 0x5c,
    GPIO_VAL_1,
    GPIO_VAL_2,
    GPIO_VAL_3,

    SPI_CONT_MODE = 0x64,
    CHIP_VER = 0x65,

    DLL = 0x00,
    DLH = 0x01
};

struct R8_IER:public Reg8<>{
    /* Enable RX data interrupt */
    uint8_t rdi  :1; 
    /* Enable TX holding register interrupt */
    uint8_t thri :1; 
    /* Enable RX line status interrupt */
    uint8_t rlsi :1; 
    /* Enable Modem status interrupt */
    uint8_t msi  :1; 
    uint8_t :1;

    /* IER enhanced register bits */

    /* Enable sleep mode */
    uint8_t sleep:1;

    /* Enable low power mode */
    uint8_t low_power:1;

    /* Enable Soft reset */
    uint8_t reset:1;
};




/* FCR register bits */


struct R8_FCR:public Reg8<>{
    uint8_t FIFO : 1; /* Enable FIFO */
    uint8_t RXRESET: 1; /* Reset RX FIFO */
    uint8_t TXRESET: 1; /* Reset TX FIFO */
    uint8_t :3;
    uint8_t RXLVLL : 1; /* RX Trigger level LSB */
    uint8_t RXLVLH : 1; /* RX Trigger level MSB */
};

struct R8_IIR:public Reg8<>{
    uint8_t noint:1;
    uint8_t iid1:1;
    uint8_t iid2:1;
    uint8_t iid3:1;
    uint8_t :2;
    uint8_t fifo_ens:1;
    uint8_t fifo_ens2:1;
};

enum class WordSize:uint8_t{
    _5 = 0x00,
    _6 = 0x01,
    _7 = 0x02,
    _8 = 0x03
};

enum class ParityMode:uint8_t{

};
struct R8_LCR:public Reg8<>{
    WordSize word_size:2;
    uint8_t stop_bit:1;
    uint8_t paren:1;
    ParityMode par_mode:2;
    uint8_t breaken:1;
    uint8_t dlab:1;
};

struct R8_MCR:public Reg8<>{
    /* DTR complement */
    uint8_t dtr:1;
    /* RTS complement */
    uint8_t rts:1;
    /* OUT1 */
    uint8_t out1:1;
    /* OUT2 */
    uint8_t out2:1;
    /* Enable loopback test mode */
    uint8_t loop:1;
    /* Enable Hardware Flow control */
    uint8_t afe:1;
    uint8_t :2;
};

struct R8_LSR:public Reg8<>{
    /* BI, FE, PE, OE bits */
    static constexpr uint8_t ERROR_MASK = 0x1e;

     /* Receiver data ready */
    uint8_t dr:1;
    /* Overrun Error */
    uint8_t oe:1;
    /* Parity Error */
    uint8_t pe:1;
    /* Frame Error */
    uint8_t fe:1;

    /* Break Interrupt */
    uint8_t bi:1;

     /* TX holding register empty */
    uint8_t thre:1;

     /* Transmitter empty */
    uint8_t txemt:1;
    /* Fifo Error */
    uint8_t fifoe:1;
};

/* MSR register bits */



struct R8_MSR:public Reg8<>{
    static constexpr uint8_t DELTA_MASK = 0x0F;     /* Any of the delta bits! */
    /* Delta CTS Clear To Send */
    uint8_t dcts   :1; 

    /* Delta DSR Data Set Ready */
    uint8_t ddsr   :1; 

    /* Delta RI Ring Indicator */
    uint8_t dri    :1; 

    /* Delta CD Carrier Detect */
    uint8_t dcd    :1; 

    /* CTS */
    uint8_t cts    :1; 

    /* DSR */
    uint8_t dsr    :1; 

    /* RI */
    uint8_t ri     :1; 

    /* CD */
    uint8_t cd     :1; 
};

struct R8_TNOW_CFG{
    static constexpr uint8_t ADDRESS = 0x41;
    uint8_t tnow_func:4;
    uint8_t tnow_level:4;
};

struct R8_FIFO_CTRL{
    static constexpr uint8_t ADDRESS = 0x42;
    uint8_t uart_num:4;
    uint8_t is_tx:1;
    uint8_t :3;
};

struct R8_FIFO_CNT_L{
    static constexpr uint8_t ADDRESS = 0x43;
    uint8_t bits;
};

struct R8_FIFO_CNT_H{
    static constexpr uint8_t ADDRESS = 0x44;
    uint8_t bits;
};

struct R8_CLK_CTRL{
    static constexpr uint8_t ADDRESS = 0x48;
    uint8_t div:5;
    uint8_t ext_power:1;
    uint8_t ext_en:1;
    uint8_t pll_en:1;
};

struct R8_CLK_CTRL{
    static constexpr uint8_t ADDRESS = 0x48;
    uint8_t div:5;
    uint8_t ext_power:1;
    uint8_t ext_en:1;
    uint8_t pll_en:1;
};

struct R8_SLEEP_CFG{
    static constexpr uint8_t ADDRESS = 0x4a;
    uint8_t lp_setting:3;
    uint8_t :5;
};


static constexpr size_t FIFO_SIZE = (1536);
static constexpr uint8_t CMD_DELAY = 3;

}