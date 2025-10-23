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
IResult<> AK8963::write_reg(const uint8_t addr, const uint8_t data){
    auto res = phy_.write_reg(std::bit_cast<uint8_t>(addr), data);
    return res;
}

IResult<> AK8963::read_reg(const uint8_t addr, uint8_t & data){
    auto res = phy_.read_reg(std::bit_cast<uint8_t>(addr), data);
    return res;
}

IResult<> AK8963::read_burst(const uint8_t reg_addr, std::span<int16_t> pbuf){
    auto res = phy_.read_burst(reg_addr, pbuf);
    return res;
}

IResult<> AK8963::init(){
    // AK8963_DEBUG("AK8963 init begin");
    if(const auto res = validate();
        res.is_err()) return Err(res.unwrap_err());

    clock::delay(2ms);
    if(const auto res = reset();
        res.is_err()) return Err(res.unwrap_err());
    clock::delay(2ms);

    {
        const auto coeff = ({
            const auto res = get_coeff();
            if(res.is_err()) return Err(res.unwrap_err());
            res.unwrap();
        });

        auto coeff2adj = [&](const uint8_t _coeff) -> q16{
            return ((q16(_coeff - 128) >> 8) + 1);
        };

        adj_scale_ = Vec3<q24>(
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


IResult<> AK8963::validate(){

    if (const auto res = phy_.validate();
        res.is_err()) return Err(res.unwrap_err());

    auto & reg = regs_.wia_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());

    if (reg.data != reg.correct){
        if(reg.data == 63)
            AK8963_DEBUG("it is normal to read 63 when comm speed too high");
        AK8963_DEBUG("AK8963 verify failed", reg.data);
        return Err{Error::InvalidChipId};
    }
    return Ok();
}


Result<Vec3<uint8_t>, Error> AK8963::get_coeff(){
    if(const auto res = write_reg(0x0a, 0x0f);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(regs_.asax_reg);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(regs_.asay_reg);
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(regs_.asaz_reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(Vec3<uint8_t>{
        regs_.asax_reg.data, 
        regs_.asay_reg.data, 
        regs_.asaz_reg.data
    });
}

IResult<> AK8963::reset(){
    auto reg = RegCopy(regs_.cntl2_reg);
    reg.srst = 1;
    const auto res = write_reg(reg);
    clock::delay(1ms);
    reg.srst = 0;
    reg.apply();
    return res;
}

IResult<bool> AK8963::is_busy(){
    TODO();
    return Ok(true);
}

IResult<bool> AK8963::is_stable(){
    TODO();
    return Ok(true);
}


IResult<> AK8963::disable_i2c(){
    TODO();
    return Ok();
}


IResult<> AK8963::update(){
    // ak8963c-datasheet 8.3.5
    // when any of measurement data is read, be sure to read 
    // ST2 register at the end. 
    if(const auto res = this->read_burst(regs_.mag_x_reg.address, std::span(&regs_.mag_x_reg.as_ref(), 3));
        res.is_err()) return Err(res.unwrap_err());

    auto & reg = regs_.st2_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    AK8963_ASSERT(!reg.hofl, "data overflow");
    data_valid_ &= !reg.hofl;
    return Ok();
}
IResult<Vec3<q24>> AK8963::read_mag(){
    return Ok(Vec3<q24>{
        conv_data_to_ut(regs_.mag_x_reg.as_val(), data_is_16_bits_) * adj_scale_.x,
        conv_data_to_ut(regs_.mag_y_reg.as_val(), data_is_16_bits_) * adj_scale_.y,
        conv_data_to_ut(regs_.mag_z_reg.as_val(), data_is_16_bits_) * adj_scale_.z}
    );
}

IResult<> AK8963::set_data_width(const uint8_t bits){
    auto reg = RegCopy(regs_.st2_reg);
    data_is_16_bits_ = [](const uint8_t bits_){
        switch(bits_){
            default: AK8963_PANIC(bits_, "bits is not allowed");
            case 14: return 0;
            case 16: return 1;
        }
    }(bits);

    reg.bitm = data_is_16_bits_;
    return write_reg(reg);
}

IResult<> AK8963::set_mode(const AK8963::Mode mode){
    auto reg =  RegCopy(regs_.cntl1_reg);
    reg.mode = mode;
    return write_reg(reg);
}