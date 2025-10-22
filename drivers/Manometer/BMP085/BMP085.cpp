#include "BMP085.hpp"

#define BMP085_DEBUG_EN 0

using namespace ymd;
using namespace ymd::drivers;

using Self = BMP085;
using Error = Self::Error;
template<typename T = void>
using IResult = Result<T, Error>;


IResult<> BMP085::validate(){
    static constexpr uint8_t REG_ADDR = 0xd0;
    static constexpr uint8_t KEY = 0x55;

    uint8_t id;
    if(const auto res = read8(REG_ADDR, id);
        res.is_err()) return res;
    
    if(id != KEY)
        return Err(Error::InvalidId);

    return Ok();
};


IResult<BMP085::Coeffs> BMP085::get_coeffs(){
    Coeffs coeffs;
    /* read calibration data */
    if(const auto res = read16(BMP085_CAL_AC1, coeffs.ac1);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read16(BMP085_CAL_AC2, coeffs.ac2);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read16(BMP085_CAL_AC3, coeffs.ac3);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read16(BMP085_CAL_AC4, coeffs.ac4);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read16(BMP085_CAL_AC5, coeffs.ac5);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read16(BMP085_CAL_AC6, coeffs.ac6);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = read16(BMP085_CAL_B1, coeffs.b1);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read16(BMP085_CAL_B2, coeffs.b2);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read16(BMP085_CAL_MB, coeffs.mb);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read16(BMP085_CAL_MC, coeffs.mc);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read16(BMP085_CAL_MD, coeffs.md);
        res.is_err()) return Err(res.unwrap_err());


    return Ok(coeffs);
}
IResult<> BMP085::init(const Config & cfg) {
    mode_ = cfg.mode;
    coeffs_ = ({
        const auto res = get_coeffs();
        if(res.is_err()) return Err(res.unwrap_err());
        res.unwrap();
    });

    return Ok();
}



IResult<uint16_t> BMP085::readRawTemperature(void) {
    if(const auto res = write8(BMP085_CONTROL, BMP085_READTEMPCMD);
        res.is_err()) return Err(res.unwrap_err());

    clock::delay(5ms);

    uint16_t tempdata;

    if(const auto res = read16(BMP085_TEMPDATA, tempdata);
        res.is_err()) return Err(res.unwrap_err());

    return Ok(tempdata);
}

IResult<uint32_t> BMP085::readRawPressure(void) {
    const uint8_t command = BMP085_READPRESSURECMD + (static_cast<uint8_t>(mode_) << 6);
    if(const auto res = write8(BMP085_CONTROL, command);
        res.is_err()) return Err(res.unwrap_err());

    const auto delay = [&]() {
        switch (mode_) {
            case Mode::UltraLowPower:       return 5ms;
            case Mode::Standard:        return 8ms;
            case Mode::HighRes:         return 14ms;
            case Mode::UltraHighRes:        return 25ms;
        }
        __builtin_unreachable();
    }();

    clock::delay(delay);

    uint16_t b1;
    uint8_t b2;
    if(const auto res = read16(BMP085_PRESSUREDATA, b1);
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = read8(BMP085_PRESSUREDATA + 2, b2);
        res.is_err()) return Err(res.unwrap_err());

    const size_t shift = 8 - std::bit_cast<uint8_t>(mode_);
    const uint32_t raw = (static_cast<uint32_t>(b1 << 8) | static_cast<uint8_t>(b2)) >> shift;

    return Ok(raw);
}


#endif