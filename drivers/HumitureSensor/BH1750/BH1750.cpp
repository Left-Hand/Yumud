#include "bh1750.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = BH1750::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> BH1750::change_measure_time(const uint16_t ms){
    uint8_t x;
    if(current_mode_ == Mode::HMode || current_mode_ == Mode::HMode2){
        x = CLAMP(ms * 69 / 120, 31, 254);
        lsb.numerator = 5 * 69;
        lsb.denominator = 6 * x;
        if(current_mode_ == Mode::HMode2){
            lsb.denominator *= 2;
        }
    }else{
        x = CLAMP(ms * 69 / 16, 31, 254);
        lsb.numerator = 5 * 69 * 4;
        lsb.denominator = 6 * x;
    }

    if(const auto res = send_command(
            std::bit_cast<uint8_t>(Command::ChangeMeasureTimeH) | (x >> 5));
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = send_command(
            std::bit_cast<uint8_t>(Command::ChangeMeasureTimeL) | (x & 31));
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> BH1750::start_conv(){
    if(cont_en_){
        return send_command(0x10 | std::bit_cast<uint8_t>(current_mode_));
    }else{
        return send_command(0x20 | std::bit_cast<uint8_t>(current_mode_));
    }
}

IResult<> BH1750::send_command(const uint8_t cmd){
    // if(const auto res = i2c_drv_.write(cmd);
    //     res.is_err()) return Err(res.unwrap_err());

    return Ok();
}


IResult<uint32_t> BH1750::get_lx(){
    uint8_t data[2] = {0};
    // i2c_drv.read(data, 2);
    TODO();
    return Ok(int(lsb * (int)((data[0] << 8) | data[1])));
}