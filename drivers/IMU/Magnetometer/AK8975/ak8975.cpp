#include "ak8975.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define AK8975_DEBUG_EN

#ifdef AK8975_DEBUG_EN
#define AK8975_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define AK8975_PANIC(...) PANIC(__VA_ARGS__)
#define AK8975_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define AK8975_DEBUG(...)
#define AK8975_PANIC(...)  PANIC()
#define AK8975_ASSERT(cond, ...) ASSERT(cond)
#endif

using Error = AK8975::Error;

template<typename T = void>
using IResult = Result<T, Error>;

IResult<> AK8975::init(){
    if(const auto res = validate(); 
        res.is_err()) return res;
    if(const auto res = update_adj();
        res.is_err()) return res;
    return update();
}


IResult<> AK8975::update_adj(){
    return phy_.read_burst(0x10, std::span(&x_adj, 3));
}


IResult<> AK8975::update(){
    return phy_.read_burst(0x03, &x, 2 * 3);
}


IResult<> AK8975::validate(){
    // <Self-test Sequence> 
    // (1) Set Power-down mode 
    // (2) Write “1” to SELF bit of ASTC register 
    // (3) Set Self-test Mode 
    // (4) Check Data Ready or not by any of the following method. 
    // - Polling DRDY bit of ST1 register 
    // - Monitor DRDY pin 
    // When Data Ready, proceed to the next step. 
    // (5) Read measurement data (HXL to HZH) 
    // (6) Write “0” to SELF bit of ASTC register

    scexpr auto TIME_OUT_MS = 10ms;

    
    {
        uint8_t id = 0;
        if(const auto res = phy_.read_reg(0x00, id);
            res.is_err()) return Err(res.unwrap_err());
        if(id != 0x48) return Err(Error::WrongWhoAmI);
        //id not correct
    }

    //1
    if(const auto res = set_mode(Mode::SelfTest);
        res.is_err()) return res;

    //2
    if(const auto res = phy_.write_reg(0x0c, 0x40);
        res.is_err()) return res;

    //3
    if(const auto res = set_mode(Mode::PowerDown);
        res.is_err()) return res;

    //4
    auto ms = clock::millis();
    bool readed = false;

    while(clock::millis() - ms < TIME_OUT_MS){
        const auto res = this->is_busy();
        if(res.is_err()) return Err(res.unwrap_err());

        if(res.unwrap() == false){
            readed = true;
            break;
        }
    }

    if(readed == false) return Err(Error::MagCantSetup); 

    //5
    if(const auto res = update();
        res.is_err()) return res;

    //6
    return phy_.write_reg(0x0c, 0x00);
}

IResult<bool> AK8975::is_busy(){
    uint8_t stat;
    if(const auto res = phy_.read_reg(0x00, stat);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(stat == 0);
}

IResult<bool> AK8975::is_stable(){
    uint8_t stat;
    if(const auto res = phy_.read_reg(0x09, stat);
        res.is_err()) return Err(res.unwrap_err());

    if(stat != 0) return Ok(false);
    
    if(const auto res = update();
        res.is_err()) return Err(res.unwrap_err());

    const auto mag = read_mag();
    if(mag.is_ok()) return Ok(false);

    auto [a, b, c] = mag.unwrap();
    if(ABS(a) + ABS(b) + ABS(c) > real_t(2.4)) return Ok(false);

    return Ok(true);
}


IResult<> AK8975::set_mode(const Mode mode){
    return phy_.write_reg(0x0A, (uint8_t)mode);
}

IResult<> AK8975::disable_i2c(){
    return phy_.write_reg(0x0F, 0x01);
}

IResult<Vec3<q24>> AK8975::read_mag(){
    scexpr real_t max_mT = real_t(1.229);
    #define CONV(n) ((n * max_mT) / 4095) * ((real_t(n##_adj - 128) >> 8) + 1)
    return Ok{Vec3<q24>{CONV(x), CONV(y), CONV(z)}};
    #undef CONV
}
