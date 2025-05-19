#include "AK8963.hpp"

using namespace ymd;
using namespace ymd::drivers;


using Error = AK8963::Error;

// #define AK8963_DEBUG_EN
// #define AK8963_NOTHROW_EN

#ifdef AK8963_DEBUG_EN
#define AK8963_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define AK8963_PANIC(...) PANIC{__VA_ARGS__}
#define AK8963_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define AK8963_DEBUG(...)
#define AK8963_PANIC(...)  PANIC_NSRC()
#define AK8963_ASSERT(cond, ...) ASSERT_NSRC(cond)

#endif

using Error = ImuError;

template<typename T = void>
using IResult= Result<T, Error>;
Result<void, Error> AK8963::write_reg(const uint8_t addr, const uint8_t data){
    auto res = phy_.write_reg(uint8_t(addr), data);
    // AK8963_ASSERT(res.is_ok(), "AK8963 write reg failed", res);
    return res;
}

Result<void, Error> AK8963::read_reg(const uint8_t addr, uint8_t & data){
    auto res = phy_.read_reg(uint8_t(addr), data);
    // AK8963_ASSERT(res.is_ok(), "AK8963 read reg failed", res);
    return res;
}

Result<void, Error> AK8963::read_burst(const uint8_t reg_addr, int16_t * datas, const size_t len){
    auto res = phy_.read_burst(reg_addr, datas, len);
    // AK8963_ASSERT(res.is_ok(), "AK8963 read reg failed");
    return res;
}

Result<void, Error> AK8963::init(){
    AK8963_DEBUG("AK8963 init begin");
    AK8963_ASSERT(validate().is_ok(), "AK8963 verify failed");

    clock::delay(2ms);
    if(const auto res = reset();
        res.is_err()) return Err(res.unwrap_err());
    clock::delay(2ms);

    {
        const auto coeff_res = get_coeff();
        if(coeff_res.is_err()) return Err(coeff_res.unwrap_err());
        const auto coeff = coeff_res.unwrap();

        auto coeff2adj = [&](const uint8_t _coeff) -> real_t{
            return ((iq_t<16>(_coeff - 128) >> 8) + 1);
        };

        adj_scale_ = Vector3_t<q24>(
            coeff2adj(coeff.x), coeff2adj(coeff.y), coeff2adj(coeff.z)
        );
    }

    clock::delay(10ms);

    if(const auto res = set_mode(Mode::ContMode2);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = set_data_width(16);
        res.is_err()) return Err(res.unwrap_err());

    AK8963_DEBUG("AK8963 init successfully!!");
    return Ok();
}


Result<void, Error> AK8963::validate(){

    if (!phy_.validate().is_ok()){
        return Err{Error::PhyVerifyFailed};
    }

    if(const auto res = read_reg(wia_reg.address, wia_reg.data);
        res.is_err()) return Err(res.unwrap_err());

    if (wia_reg.data != wia_reg.correct){
        if(wia_reg.data == 63){
            AK8963_DEBUG("it is normal to read 63 when comm speed too high");
        }
        AK8963_PANIC("AK8963 verify failed", wia_reg.data);
        return Err{Error::WrongWhoAmI};
    }
    return Ok();
}
Result<Vector3_t<uint8_t>, Error> AK8963::get_coeff(){
    // Vector3_t coeff = {};
    if(const auto res = write_reg(0x0a, 0x0f);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(asax_reg.address, asax_reg.as_ref());
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(asay_reg.address, asay_reg.as_ref());
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(asaz_reg.address, asaz_reg.as_ref());
        res.is_err()) return Err(res.unwrap_err());
    return Ok(Vector3_t<uint8_t>{asax_reg.data, asay_reg.data, asaz_reg.data});
}

Result<void, Error> AK8963::reset(){
    cntl2_reg.srst = 1;
    const auto res = write_reg(cntl2_reg.address, cntl2_reg);
    cntl2_reg.srst = 0;
    clock::delay(1ms);
    return res;
}

Result<void, Error> AK8963::busy(){
    return Ok();
}

Result<void, Error> AK8963::stable(){return Ok();}


Result<void, Error> AK8963::disable_i2c(){
    return Ok();
}


Result<void, Error> AK8963::update(){
    // ak8963c-datasheet 8.3.5
    // when any of measurement data is read, be sure to read 
    // ST2 register at the end. 
    // auto lam2 = [&](){return read_reg(st2_reg.address, st2_reg);};
    // using Fn = std::decay_t<decltype(lam2)>;
    // using Ret = function_traits<Fn>::return_type;
    // using t = std::invoke_result_t<decltype(lam2)>>;
    data_valid_ = this->read_burst(mag_x_reg.address, &mag_x_reg, 3).is_ok();
    if(const auto res = read_reg(st2_reg.address, st2_reg.as_ref());
        res.is_err()) return Err(res.unwrap_err());
    AK8963_ASSERT(!st2_reg.hofl, "data overflow");
    data_valid_ &= !st2_reg.hofl;
    return Ok();
}
IResult<Vector3_t<q24>> AK8963::read_mag(){
    return Ok(Vector3_t<q24>{
        conv_data_to_ut(mag_x_reg.as_val(), data_is_16_bits_) * adj_scale_.x,
        conv_data_to_ut(mag_y_reg.as_val(), data_is_16_bits_) * adj_scale_.y,
        conv_data_to_ut(mag_z_reg.as_val(), data_is_16_bits_) * adj_scale_.z}
    );
}

Result<void, Error> AK8963::set_data_width(const uint8_t bits){
    auto & reg = st2_reg;
    data_is_16_bits_ = [](const uint8_t bits_){
        switch(bits_){
            default: AK8963_PANIC(bits_, "bits is not allowed");
            case 14: return 0;
            case 16: return 1;
        }
    }(bits);

    reg.bitm = data_is_16_bits_;
    return write_reg(reg.address, reg);
}

Result<void, Error> AK8963::set_mode(const AK8963::Mode mode){
    cntl1_reg.mode = uint8_t(mode);
    return write_reg(cntl1_reg.address, cntl1_reg);
}