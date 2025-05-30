#include "mmc5603.hpp"

using namespace ymd;
using namespace ymd::drivers;

#ifdef MMC5603_DEBUG
#undef MMC5603_DEBUG
#define MMC5603_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MMC5603_PANIC(...) PANIC(__VA_ARGS__)
#define MMC5603_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define MMC5603_DEBUG(...)
#define MMC5603_PANIC(...)  PANIC()
#define MMC5603_ASSERT(cond, ...) ASSERT(cond)
#endif

using Error = ImuError;

template<typename T = void>
using IResult= Result<T, Error>;

IResult<> MMC5603::update(){
    auto reg = RegCopy(x_reg);
    return read_burst(reg.address_x, &reg.data_h, 6);
}

IResult<> MMC5603::reset(){
    auto reg = RegCopy(ctrl0_reg);
    reg.do_reset = true;
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    reg.do_reset = false;
    reg.apply();
    return Ok();
}

IResult<> MMC5603::validate(){
    auto reg = RegCopy(product_id_reg);
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    
    if(reg != reg.correct_id) return Err(Error::WrongWhoAmI);

    if(const auto res = set_self_test_threshlds(0,0,0);//TODO change
        res.is_err()) return res;

    return Ok();
}

IResult<> MMC5603::set_data_rate(const DataRate dr){
    {
        auto reg = RegCopy(odr_reg);
        reg.data_rate = dr;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(ctrl2_reg);
        if(reg.high_pwr != 1){
            reg.high_pwr = 1;
            if(const auto res = write_reg(reg);
                res.is_err()) return Err(res.unwrap_err());
        }
    }

    if(const auto res = enable_contious(EN);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}


IResult<> MMC5603::set_band_width(const BandWidth bw){
    auto reg = RegCopy(ctrl1_reg);
    reg.bandwidth = uint8_t(bw);
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> MMC5603::enable_contious(const Enable en){
    auto reg = RegCopy(ctrl2_reg);
    reg.cont_en = en == EN;
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


IResult<Vector3<q24>> MMC5603::read_mag(){
    return Ok{Vector3<q24>{
        s16_to_uni(int16_t(x_reg)),
        s16_to_uni(int16_t(y_reg)),
        s16_to_uni(int16_t(z_reg))
    }};
}

IResult<> MMC5603::set_self_test_threshlds(uint8_t x, uint8_t y, uint8_t z){
    auto x_st_reg_copy = RegCopy(x_st_reg);
    auto y_st_reg_copy = RegCopy(y_st_reg);
    auto z_st_reg_copy = RegCopy(z_st_reg);

    if(const auto res = write_reg(x_st_reg_copy);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(y_st_reg_copy);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(z_st_reg_copy);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> MMC5603::inhibit_channels(bool x, bool y, bool z){
    auto reg = RegCopy(ctrl1_reg);

    reg.x_inhibit = x;
    reg.y_inhibit = y;
    reg.z_inhibit = z;

    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}