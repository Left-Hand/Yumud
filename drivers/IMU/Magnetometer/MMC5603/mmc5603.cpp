#include "mmc5603.hpp"

using namespace ymd;
using namespace ymd::drivers;

#ifdef MMC5603_DEBUG_EN
#define MMC5603_TODO(...) TODO()
#define MMC5603_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define MMC5603_PANIC(...) PANIC{__VA_ARGS__}
#define MMC5603_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}


#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    PANIC{#x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define MMC5603_DEBUG(...)
#define MMC5603_TODO(...) PANIC_NSRC()
#define MMC5603_PANIC(...)  PANIC_NSRC()
#define MMC5603_ASSERT(cond, ...) ASSERT_NSRC(cond)

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif


using Error = ImuError;

template<typename T = void>
using IResult= Result<T, Error>;

IResult<> MMC5603::update(){
    auto reg = RegCopy(regs_.x_reg);
    return read_burst(REG_ADDR_X, std::span(&reg.data_h, 6));
}

IResult<> MMC5603::reset(){
    auto reg = RegCopy(regs_.ctrl0_reg);
    reg.do_reset = true;
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    reg.do_reset = false;
    reg.apply();
    return Ok();
}

IResult<> MMC5603::validate(){
    auto reg = RegCopy(regs_.product_id_reg);
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    
    if(reg.id != reg.correct_id) return Err(Error::InvalidChipId);

    if(const auto res = set_self_test_threshlds(0,0,0);//TODO change
        res.is_err()) return res;

    return Ok();
}

IResult<> MMC5603::set_datarate(const DataRate dr){
    {
        auto reg = RegCopy(regs_.odr_reg);
        reg.datarate = dr;
        if(const auto res = write_reg(reg);
            res.is_err()) return Err(res.unwrap_err());
    }

    {
        auto reg = RegCopy(regs_.ctrl2_reg);
        if(reg.high_pwr != 1){
            reg.high_pwr = 1;
            if(const auto res = write_reg(reg);
                res.is_err()) return Err(res.unwrap_err());
        }
    }

    if(const auto res = enable_continuous(EN);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}


IResult<> MMC5603::set_band_width(const BandWidth bw){
    auto reg = RegCopy(regs_.ctrl1_reg);
    reg.bandwidth = uint8_t(bw);
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> MMC5603::enable_continuous(const Enable en){
    auto reg = RegCopy(regs_.ctrl2_reg);
    reg.cont_en = (en == EN);
    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}


IResult<math::Vec3<iq24>> MMC5603::read_mag(){
    return Ok{math::Vec3{
        iq24(iq16::from_bits(regs_.x_reg.to_bits())),
        iq24(iq16::from_bits(regs_.y_reg.to_bits())),
        iq24(iq16::from_bits(regs_.z_reg.to_bits()))
    }};
}

IResult<> MMC5603::set_self_test_threshlds(uint8_t x, uint8_t y, uint8_t z){
    auto x_st_reg_copy = RegCopy(regs_.x_st_reg);
    auto y_st_reg_copy = RegCopy(regs_.y_st_reg);
    auto z_st_reg_copy = RegCopy(regs_.z_st_reg);

    x_st_reg_copy.bits = x;
    y_st_reg_copy.bits = y;
    z_st_reg_copy.bits = z;
    if(const auto res = write_reg(x_st_reg_copy);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(y_st_reg_copy);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = write_reg(z_st_reg_copy);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}

IResult<> MMC5603::inhibit_channels(bool x, bool y, bool z){
    auto reg = RegCopy(regs_.ctrl1_reg);

    reg.x_inhibit = x;
    reg.y_inhibit = y;
    reg.z_inhibit = z;

    if(const auto res = write_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok();
}