#pragma once

// TDC-GP22 SPI通信格式
// Clock Polarity Low：空闲时钟为低电平
// Clock Phase 2：数据在第二个时钟边沿（下降沿）被采样
// MSB First：从最高位（MSB）开始传输以最低位（LSB）结束
// SSN：传输数据时为低电平


#include "details/tdc_gp22_prelude.hpp"


namespace ymd::drivers{


class GP22 final:
    public GP22_Prelude{

public:


    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> reset();


    [[nodiscard]] IResult<> init();
    
    [[nodiscard]] IResult<> reconf();

    [[nodiscard]] IResult<> start_measurement();

    [[nodiscard]] IResult<bool> is_measurement_done();

    [[nodiscard]] IResult<q16> blocking_get_meas_value(Milliseconds timeout_ms);
    

    // 等待读取TDC测量结果，需要先启动TDC测量，超时返回0xFFFFFFFF
    // 测量模式1下最大时间差：2*Tref*DIV_CLKHS=1us，超时TDC读出的数据为0xFFFFFFFF
    [[nodiscard]] IResult<q16> get_meas_value() {
        clock::delay(1us);
        return phy_.trans_u8_receive_u32(0xB0)
            .transform([](const uint32_t x){return q16::from_i32(x);}); // Read REG0
    }
private:
    GP22_Phy phy_;

    [[nodiscard]] IResult<> soft_reset() {
        if(const auto res = phy_.write_u8(0x50);
            res.is_err()) return res;
        clock::delay(1ms);
        return Ok();
    }

};


}