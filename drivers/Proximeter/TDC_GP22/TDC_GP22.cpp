#include "TDC_GP22.hpp"


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

using Self = GP22;
using Error = Self::Error;

template<typename T = void>
using IResult = Result<T, Error>;

static constexpr Nanoseconds value2ns(iq16 tdc_value) {
    return Nanoseconds(uint64_t(tdc_value * 500));
}

IResult<> Self::validate(){
    // return phy_.transrecive_u8(0xB5); // 显示写寄存器1中的高8位, 用来测试通信
    TODO();
    return Ok();
}

IResult<> Self::reset(){
    TODO();
    return Ok();
}

IResult<> Self::init(){
    if(const auto res = reset();
        res.is_err()) return res;
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = reconf();
        res.is_err()) return res;
    return Ok();
}

IResult<> Self::reconf() {
    if(const auto res = phy_.write_u32(0x80009620); // 测量范围1，4M晶振二分频，4M晶振上电后一直起振，自动校准，校准陶瓷晶振时间为8个32K周期，上升沿敏感
        res.is_err()) return res;
    if(const auto res = phy_.write_u32(0x81014100); // STOP1-START
        res.is_err()) return res;
    if(const auto res = phy_.write_u32(0x82E00000); // 开启所有中断源，单边沿敏感
        res.is_err()) return res;
    if(const auto res = phy_.write_u32(0x83180000); // 几乎全为默认值
        res.is_err()) return res;
    if(const auto res = phy_.write_u32(0x84200000); // 全为默认值
        res.is_err()) return res;
    if(const auto res = phy_.write_u32(0x85080000); // 关闭噪声单元
        res.is_err()) return res;
    if(const auto res = phy_.write_u32(0x86010000); // 测量精度从90ps加倍到45ps
        res.is_err()) return res;

    return Ok();
}


// 启动一次TDC测量，start、stop开始接收信号
IResult<> Self::start_measurement() {
    const auto res = phy_.transrecive_u8(0x70); // Init
    if(res.is_err()) return Err(res.unwrap_err());
    [[maybe_unused]] const auto val = res.unwrap();
    // if(val != )
    return Ok();
}

IResult<bool> Self::is_measurement_done(){
    TODO();
}

IResult<iq16> Self::blocking_get_meas_value(Milliseconds timeout_ms) {
    const auto t = clock::millis();

    while(true){
        clock::delay(1us);
        if (clock::millis() - t > timeout_ms) {
            return Err(Error::MeasurementNotDone); // 测量超时
        }

        const bool measurement_done = ({
            const auto res = is_measurement_done();
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap() == true;
        });

        if(measurement_done) break;
    }

    return get_meas_value();
}