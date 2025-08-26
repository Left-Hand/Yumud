#pragma once

// TDC-GP22 SPI通信格式
// Clock Polarity Low：空闲时钟为低电平
// Clock Phase 2：数据在第二个时钟边沿（下降沿）被采样
// MSB First：从最高位（MSB）开始传输以最低位（LSB）结束
// SSN：传输数据时为低电平


#include "details/tdc_gp22_prelude.hpp"


namespace ymd::drivers{



class GP22_Phy final:
    private GP22_Prelude{
public:
        // TDC硬件初始化
    void reset() {
        if(reset_gpio_.is_none()) return;
        auto & RSTN = reset_gpio_.unwrap();
        RSTN.set();
        clock::delay(1us);
        RSTN.clr();
        clock::delay(1us);
        RSTN.set();
        clock::delay(1ms);
    }

    bool has_reset_gpio();

    IResult<> write_u8(const uint8_t data);
    IResult<uint8_t> transrecive_u8(const uint8_t data);
    IResult<> write_u32(const uint32_t data);
    IResult<uint32_t> trans_u8_receive_u32(const uint8_t data);
private:
    Option<hal::GpioIntf &> reset_gpio_;
};

class GP22 final:
    public GP22_Prelude{

public:


    IResult<> validate();

    IResult<> reset();


    IResult<> init();
    
    IResult<> reconf();

    IResult<> start_measurement();

    IResult<bool> is_measurement_done();

    IResult<iq_t<16>> blocking_get_meas_value(Milliseconds timeout_ms);
    

    // 等待读取TDC测量结果，需要先启动TDC测量，超时返回0xFFFFFFFF
    // 测量模式1下最大时间差：2*Tref*DIV_CLKHS=1us，超时TDC读出的数据为0xFFFFFFFF
    IResult<iq_t<16>> get_meas_value() {
        clock::delay(1us);
        return phy_.trans_u8_receive_u32(0xB0)
            .transform([](const uint32_t x){return iq_t<16>::from_i32(x);}); // Read REG0
    }
private:
    GP22_Phy phy_;

    static constexpr Nanoseconds value2ns(q16 tdc_value) {
        return Nanoseconds(uint64_t(tdc_value * 500));
    }

    IResult<> soft_reset() {
        if(const auto res = phy_.write_u8(0x50);
            res.is_err()) return res;
        clock::delay(1ms);
        return Ok();
    }

};


}