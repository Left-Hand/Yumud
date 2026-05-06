#pragma once

#include <cstdint>
#include "core/io/regs.hpp"
#include "hal/conn/spi/spidrv.hpp"

// 参考资料：
// https://github.com/autowp/arduino-mcp2515/blob/master/mcp2515.h

namespace ymd::drivers{


struct MCP2515_Prelude{



enum class [[nodiscard]] Instruction : uint8_t {
    WRITE       = 0x02,
    READ        = 0x03,
    BITMOD      = 0x05,
    LOAD_TX0    = 0x40,
    LOAD_TX1    = 0x42,
    LOAD_TX2    = 0x44,
    RTS_TX0     = 0x81,
    RTS_TX1     = 0x82,
    RTS_TX2     = 0x84,
    RTS_ALL     = 0x87,
    READ_RX0    = 0x90,
    READ_RX1    = 0x94,
    READ_STATUS = 0xA0,
    RX_STATUS   = 0xB0,
    RESET       = 0xC0
};

enum class [[nodiscard]] RegAddr : uint8_t {
    RXF0SIDH = 0x00,
    RXF0SIDL = 0x01,
    RXF0EID8 = 0x02,
    RXF0EID0 = 0x03,
    RXF1SIDH = 0x04,
    RXF1SIDL = 0x05,
    RXF1EID8 = 0x06,
    RXF1EID0 = 0x07,
    RXF2SIDH = 0x08,
    RXF2SIDL = 0x09,
    RXF2EID8 = 0x0A,
    RXF2EID0 = 0x0B,
    CANSTAT  = 0x0E,
    CANCTRL  = 0x0F,
    RXF3SIDH = 0x10,
    RXF3SIDL = 0x11,
    RXF3EID8 = 0x12,
    RXF3EID0 = 0x13,
    RXF4SIDH = 0x14,
    RXF4SIDL = 0x15,
    RXF4EID8 = 0x16,
    RXF4EID0 = 0x17,
    RXF5SIDH = 0x18,
    RXF5SIDL = 0x19,
    RXF5EID8 = 0x1A,
    RXF5EID0 = 0x1B,
    TEC      = 0x1C,
    REC      = 0x1D,
    RXM0SIDH = 0x20,
    RXM0SIDL = 0x21,
    RXM0EID8 = 0x22,
    RXM0EID0 = 0x23,
    RXM1SIDH = 0x24,
    RXM1SIDL = 0x25,
    RXM1EID8 = 0x26,
    RXM1EID0 = 0x27,
    CNF3     = 0x28,
    CNF2     = 0x29,
    CNF1     = 0x2A,
    CANINTE  = 0x2B,
    CANINTF  = 0x2C,
    EFLG     = 0x2D,
    TXB0CTRL = 0x30,
    TXB0SIDH = 0x31,
    TXB0SIDL = 0x32,
    TXB0EID8 = 0x33,
    TXB0EID0 = 0x34,
    TXB0DLC  = 0x35,
    TXB0DATA = 0x36,
    TXB1CTRL = 0x40,
    TXB1SIDH = 0x41,
    TXB1SIDL = 0x42,
    TXB1EID8 = 0x43,
    TXB1EID0 = 0x44,
    TXB1DLC  = 0x45,
    TXB1DATA = 0x46,
    TXB2CTRL = 0x50,
    TXB2SIDH = 0x51,
    TXB2SIDL = 0x52,
    TXB2EID8 = 0x53,
    TXB2EID0 = 0x54,
    TXB2DLC  = 0x55,
    TXB2DATA = 0x56,
    RXB0CTRL = 0x60,
    RXB0SIDH = 0x61,
    RXB0SIDL = 0x62,
    RXB0EID8 = 0x63,
    RXB0EID0 = 0x64,
    RXB0DLC  = 0x65,
    RXB0DATA = 0x66,
    RXB1CTRL = 0x70,
    RXB1SIDH = 0x71,
    RXB1SIDL = 0x72,
    RXB1EID8 = 0x73,
    RXB1EID0 = 0x74,
    RXB1DLC  = 0x75,
    RXB1DATA = 0x76
};

enum class Error {
    AllTxBusy,
    FailInit,
    FailTx,
    NoMsg
};


template<typename T = void>
using IResult = Result<T, Error>;

enum MASK {
    MASK0,
    MASK1
};

enum RXF {
    RXF0 = 0,
    RXF1 = 1,
    RXF2 = 2,
    RXF3 = 3,
    RXF4 = 4,
    RXF5 = 5
};

enum RXBn {
    RXB0 = 0,
    RXB1 = 1
};

enum TXBn {
    TXB0 = 0,
    TXB1 = 1,
    TXB2 = 2
};

static constexpr uint8_t CANINTF_RX0IF = 0x01;
static constexpr uint8_t CANINTF_RX1IF = 0x02;
static constexpr uint8_t CANINTF_TX0IF = 0x04;
static constexpr uint8_t CANINTF_TX1IF = 0x08;
static constexpr uint8_t CANINTF_TX2IF = 0x10;
static constexpr uint8_t CANINTF_ERRIF = 0x20;
static constexpr uint8_t CANINTF_WAKIF = 0x40;
static constexpr uint8_t CANINTF_MERRF = 0x80;

struct R8_Intf{
    uint8_t RX0IF:1;
    uint8_t RX1IF:1;
    uint8_t TX0IF:1;
    uint8_t TX1IF:1;
    uint8_t TX2IF:1;
    uint8_t ERRIF:1;
    uint8_t WAKIF:1;
    uint8_t MERRF:1;
};

enum class ReqOp:uint8_t{
    Normal = 0b000,
    Sleep = 0b001,
    Ringback = 0b010,
    Slient = 0b011,
    Configure = 0b100
};

enum class ClkDiv:uint8_t {
    _1 = 0x0,
    _2 = 0x1,
    _4 = 0x2,
    _8 = 0x3,
};


struct R8_CanCtrl{
    ClkDiv clk_div:2;
    uint8_t clken:1;
    uint8_t osm:1;
    uint8_t abat:1;
    ReqOp reqop:3;
};


// struct TXBn_REGS {
//     RegAddr CTRL;
//     RegAddr SIDH;
//     RegAddr DATA;
// };

// struct RXBn_REGS {
//     RegAddr CTRL;
//     RegAddr SIDH;
//     RegAddr DATA;
//     R8_Intf  CANINTF_RXnIF;
// };


static constexpr size_t N_TXBUFFERS = 3;
static constexpr size_t N_RXBUFFERS = 2;

// static constexpr TXBn_REGS TXB[N_TXBUFFERS] = {
//     {RegAddr::TXB0CTRL, RegAddr::TXB0SIDH, RegAddr::TXB0DATA},
//     {RegAddr::TXB1CTRL, RegAddr::TXB1SIDH, RegAddr::TXB1DATA},
//     {RegAddr::TXB2CTRL, RegAddr::TXB2SIDH, RegAddr::TXB2DATA}
// };

// static constexpr RXBn_REGS RXB[N_RXBUFFERS] = {
//     {RegAddr::RXB0CTRL, RegAddr::RXB0SIDH, RegAddr::RXB0DATA, CANINTF_RX0IF},
//     {RegAddr::RXB1CTRL, RegAddr::RXB1SIDH, RegAddr::RXB1DATA, CANINTF_RX1IF}
// };
};

}