#pragma once


#include "mcp2515_prelude.hpp"
#include "primitive/can/bxcan_frame.hpp"


// 参考资料：
// https://github.com/autowp/arduino-mcp2515/blob/master/mcp2515.h


namespace ymd::drivers{


struct MCP2515:public MCP2515_Prelude{

public:
    ERROR reset(void);
    ERROR setConfigMode();
    ERROR setListenOnlyMode();
    ERROR setSleepMode();
    ERROR setLoopbackMode();
    ERROR setNormalMode();
    ERROR setNormalOneShotMode();
    ERROR setClkOut(const CAN_CLKOUT divisor);
    ERROR setFilterMask(const MASK num, const bool ext, const uint32_t ulData);
    ERROR setFilter(const RXF num, const bool ext, const uint32_t ulData);
    ERROR transmit_frame(const TXBn txbn, const hal::ClassicCanFrame & can_frame);
    ERROR transmit_frame(const hal::ClassicCanFrame & can_frame);
    ERROR receive_frame(const RXBn rxbn, hal::ClassicCanFrame & can_frame);
    ERROR receive_frame(hal::ClassicCanFrame & can_frame);
    bool checkReceive(void);
    bool checkError(void);
    uint8_t getErrorFlags(void);
    uint8_t getControlRegister(void);
    void clearRXnOVRFlags(void);
    uint8_t getInterrupts(void);
    uint8_t getInterruptMask(void);
    void clearInterrupts(void);
    void clearTXInterrupts(void);
    uint8_t getStatus(void);
    void clearRXnOVR(void);
    void clearMERR();
    void clearERRIF();
    uint8_t errorCountRX(void);
    uint8_t errorCountTX(void);


private:

    ERROR setMode(const CANCTRL_REQOP_MODE mode);

    uint8_t read_reg(const RegAddr reg);
    void read_regs(const RegAddr reg, uint8_t * values, const uint8_t n);
    void write_reg(const RegAddr reg, const uint8_t value);
    void write_bulk(const RegAddr reg, const uint8_t * values, const uint8_t n);
    void modify_reg(const RegAddr reg, const uint8_t mask, const uint8_t data);
};

}