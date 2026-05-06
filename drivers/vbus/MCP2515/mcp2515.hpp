#pragma once


#include "mcp2515_prelude.hpp"
#include "primitive/can/bxcan_frame.hpp"


// 参考资料：
// https://github.com/autowp/arduino-mcp2515/blob/master/mcp2515.h


namespace ymd::drivers{


struct MCP2515:public MCP2515_Prelude{

public:
    IResult<> reset(void);

    IResult<> set_normal_one_shot_mode();
    IResult<> set_clk_out(const ClkDiv divisor);
    IResult<> set_filter_mask(const MASK num, const bool ext, const uint32_t ulData);
    IResult<> set_filter(const RXF num, const bool ext, const uint32_t ulData);
    IResult<> transmit_frame(const TXBn txbn, const hal::ClassicCanFrame & can_frame);
    IResult<> transmit_frame(const hal::ClassicCanFrame & can_frame);
    IResult<> receive_frame(const RXBn rxbn, hal::ClassicCanFrame & can_frame);
    IResult<> receive_frame(hal::ClassicCanFrame & can_frame);


    // IResult<bool> checkReceive(void);
    // IResult<bool> checkError(void);
    // IResult<uint8_t> getErrorFlags(void);
    // IResult<uint8_t> getControlRegister(void);
    // IResult<void> clearRXnOVRFlags(void);
    // IResult<uint8_t> getInterrupts(void);
    // IResult<uint8_t> getInterruptMask(void);
    // IResult<void> clearInterrupts(void);
    // IResult<void> clearTXInterrupts(void);
    // IResult<uint8_t> getStatus(void);
    // IResult<void> clearRXnOVR(void);
    // IResult<void> clearMERR();
    // IResult<void> clearERRIF();
    // IResult<uint8_t> errorCountRX(void);
    // IResult<uint8_t> errorCountTX(void);


private:
    IResult<> set_config_mode();
    IResult<> set_listen_only_mode();
    IResult<> set_sleep_mode();
    IResult<> set_loopback_mode();
    IResult<> set_normal_mode();
    IResult<> set_mode(const ReqOp mode);

    IResult<void> read_reg(const RegAddr reg_addr, uint8_t & reg_val);
    IResult<void> read_regs(const RegAddr reg_addr, uint8_t * values, const uint8_t n);
    IResult<void> write_reg(const RegAddr reg_addr, const uint8_t value);
    IResult<void> write_bulk(const RegAddr reg_addr, const uint8_t * values, const uint8_t n);
    IResult<void> modify_reg(const RegAddr reg_addr, const uint8_t mask, const uint8_t data);
};

}