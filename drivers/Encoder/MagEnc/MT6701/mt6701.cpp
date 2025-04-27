#include "mt6701.hpp"
#include "core/debug/debug.hpp"



#ifdef MT6701_DEBUG
#undef MT6701_DEBUG
#define MT6701_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__)
#else
#define MT6701_DEBUG(...)
#endif


using namespace ymd::drivers;
using namespace ymd;

using Error = MT6701::Error;

template<typename T = void>
using IResult = typename MT6701::IResult<T>;


#define MT6701_NO_I2C_FAULT\
    MT6701_DEBUG("NO I2C!!");\
    PANIC()\



IResult<> MT6701_Phy::write_reg(const RegAddress addr, const uint16_t data){
    if(i2c_drv_) return Err(Error::BusError(i2c_drv_->write_reg(uint8_t(addr), data, MSB)));
    else{
        MT6701_NO_I2C_FAULT;
    }
}

IResult<> MT6701_Phy::read_reg(const RegAddress addr, uint16_t & data){
    if(i2c_drv_) return Err(Error::BusError(i2c_drv_->read_reg(uint8_t(addr), data, MSB)));
    else{
        MT6701_NO_I2C_FAULT;
    }
}

IResult<> MT6701_Phy::write_reg(const RegAddress addr, const uint8_t data){
    if(i2c_drv_) return Err(Error::BusError(i2c_drv_->write_reg(uint8_t(addr), data)));
    else{
        MT6701_NO_I2C_FAULT;
    }
}

IResult<> MT6701_Phy::read_reg(const RegAddress addr, uint8_t & data){
    if(i2c_drv_) return Err(Error::BusError(i2c_drv_->read_reg(uint8_t(addr), data)));
    else{
        MT6701_NO_I2C_FAULT;
    }
}
void MT6701::init(){
    enable_pwm();
    set_pwm_polarity(true);
    set_pwm_freq(PwmFreq::HZ497_2);
    update();
}

IResult<> MT6701::update(){
    const auto res = phy_.read_reg(RegAddress::RawAngle, rawAngleData);
    lap_position = u16_to_uni(rawAngleData);
    return res;
    // else if(spi_drv){

    //     uint16_t data16;
    //     spi_drv->read_single(data16).unwrap();

    //     uint8_t data8 = 0;
    //     if(fast_mode == false){
    //         spi_drv->read_single(data8).unwrap();
    //     }

    //     semantic = Semantic{data8, data16};
    //     if(semantic.valid(fast_mode)){
    //         lap_position = real_t(iq_t<16>(semantic.data_14bit << 2) >> 16);
    //     } 
    // }
    // else{
    //     MT6701_DEBUG("no drv!!");
    //     PANIC();
    // }
}

IResult<real_t> MT6701::get_lap_position(){
    return Ok(lap_position);
}

IResult<bool> MT6701::is_stable(){
    return Ok(semantic.valid(fast_mode));
}


IResult<> MT6701::enable_uvwmux(const bool enable){
    uvwMuxReg.uvwMux = enable;
    return phy_.write_reg(RegAddress::UVWMux, uint8_t(uvwMuxReg));
}

IResult<> MT6701::enable_abzmux(const bool enable){
    abzMuxReg.abzMux = enable;
    return phy_.write_reg(RegAddress::ABZMux, uint8_t(abzMuxReg));
}

IResult<> MT6701::set_direction(const bool clockwise){
    abzMuxReg.clockwise = clockwise;
    return phy_.write_reg(RegAddress::ABZMux, uint8_t(abzMuxReg));
}

IResult<> MT6701::set_poles(const uint8_t _poles){
    resolutionReg.poles = _poles;
    return phy_.write_reg(RegAddress::Resolution, uint16_t(resolutionReg));
}

IResult<> MT6701::set_abz_resolution(const uint16_t abzResolution){
    resolutionReg.abzResolution = abzResolution;
    return phy_.write_reg(RegAddress::Resolution, uint16_t(resolutionReg));
}

IResult<> MT6701::set_zero_position(const uint16_t zeroPosition){
    zeroConfigReg.zeroPosition = zeroPosition;
    return phy_.write_reg(RegAddress::ZeroConfig, uint16_t(zeroConfigReg));
}

IResult<> MT6701::set_zero_pulse_width(const ZeroPulseWidth zeroPulseWidth){
    zeroConfigReg.zeroPulseWidth = (uint8_t)zeroPulseWidth;
    return phy_.write_reg(RegAddress::ZeroConfig, uint16_t(zeroConfigReg));
}

IResult<> MT6701::set_hysteresis(const Hysteresis hysteresis){
    hystersisReg.hysteresis = (uint8_t)hysteresis & 0b11;
    zeroConfigReg.hysteresis = (uint8_t)hysteresis >> 2;
    return 
        phy_.write_reg(RegAddress::Hystersis, uint8_t(hystersisReg))
        | phy_.write_reg(RegAddress::ZeroConfig, uint16_t(zeroConfigReg))
    ;
}

IResult<> MT6701::enable_fast_mode(const bool en){
    fast_mode = en;
    return Ok();
}

IResult<> MT6701::enable_pwm(const bool enable){
    wireConfigReg.isPwm = enable;
    return phy_.write_reg(RegAddress::WireConfig, uint8_t(wireConfigReg));
}

IResult<> MT6701::set_pwm_polarity(const bool polarity){
    wireConfigReg.pwmPolarityLow = !polarity;
    return phy_.write_reg(RegAddress::WireConfig, uint8_t(wireConfigReg));
}

IResult<> MT6701::set_pwm_freq(const PwmFreq pwmFreq){
    wireConfigReg.pwmFreq = (uint8_t)pwmFreq;
    return phy_.write_reg(RegAddress::WireConfig, uint8_t(wireConfigReg));
}

IResult<> MT6701::set_start(const real_t start){
    uint16_t _startData = uni_to_u16(start);
    _startData >>= 4;
    startData = _startData;
    startStopReg.start = _startData >> 8;
    return 
        phy_.write_reg(RegAddress::Start, startData)
        | phy_.write_reg(RegAddress::StartStop, uint8_t(startStopReg))
        ;
}

IResult<> MT6701::set_stop(const real_t stop){
    uint16_t _stopData = uni_to_u16(stop);
    _stopData >>= 4;
    stopData = _stopData;
    startStopReg.stop = _stopData >> 8;
    return 
        phy_.write_reg(RegAddress::Stop, stopData)
        | phy_.write_reg(RegAddress::StartStop, uint8_t(startStopReg))
        ;
}