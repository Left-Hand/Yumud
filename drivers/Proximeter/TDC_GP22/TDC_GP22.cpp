#include "TDC_GP22.hpp"


using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

using Error = GP22::Error;

template<typename T = void>
using IResult = Result<T, Error>;


IResult<> GP22::validate(){
    // return phy_.transrecive_u8(0xB5); // 显示写寄存器1中的高8位, 用来测试通信
    return Ok();
}

IResult<> GP22::reset(){
    TODO();
    return Ok();
}

IResult<> GP22::init(){
    if(const auto res = reset();
        res.is_err()) return res;
    if(const auto res = validate();
        res.is_err()) return res;
    if(const auto res = reconf();
        res.is_err()) return res;
    return Ok();
}
IResult<> GP22::reconf() {
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
IResult<> GP22::start_measurement() {
    const auto res = phy_.transrecive_u8(0x70); // Init
    if(res.is_err()) return Err(res.unwrap_err());
    [[maybe_unused]] const auto val = res.unwrap();
    // if(val != )
    return Ok();
}

IResult<bool> GP22::is_measurement_done(){
    TODO();
}

IResult<iq_t<16>> GP22::blocking_get_meas_value(Milliseconds timeout_ms) {
    const auto t = clock::millis();

    while(true){
        clock::delay(1us);
        if (clock::millis() - t > timeout_ms) {
            return Err(Error::MeasurementNotDone); // 测量超时
        }

        if(({
            const auto res = is_measurement_done();
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap() == true;
        })){
            break;
        }
    }

    return get_meas_value();
}