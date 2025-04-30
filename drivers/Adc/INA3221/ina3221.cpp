#include "ina3221.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;

#define INA3221_DEBUG_EN

#ifdef INA3221_DEBUG_EN
#define INA3221_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA3221_PANIC(...) PANIC{__VA_ARGS__}
#define INA3221_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define INA3221_DEBUG(...)
#define INA3221_PANIC(...)  PANIC_NSRC()
#define INA3221_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

template<typename T = void>
using IResult = INA3221::IResult<T>;

using Error = INA3221::Error;

IResult<> INA3221::init(){
    INA3221_ASSERT(verify().is_ok(), "INA3221 verify failed");

    if(const auto res = this->enable_channel(ChannelIndex::CH1);
        res.is_err()) return res;
    if(const auto res = this->set_shunt_conversion_time(INA3221::ConversionTime::_140us);
        res.is_err()) return res;
    if(const auto res = this->set_bus_conversion_time(INA3221::ConversionTime::_140us);
        res.is_err()) return res;
    if(const auto res = this->set_average_times(INA3221::AverageTimes::_1);
        res.is_err()) return res;
    if(const auto res = this->enable_continuous();
        res.is_err()) return res;
    if(const auto res = this->enable_measure_bus();
        res.is_err()) return res;
    if(const auto res = this->enable_measure_shunt();
        res.is_err()) return res;
    // while(true){
    //     INA3221_DEBUG(config_reg);
    // }
    return Ok();
}

IResult<bool> INA3221::is_ready(){
    TODO();
    return Ok(true);
}


IResult<void> INA3221::verify(){

    INA3221_ASSERT(i2c_drv.verify().is_ok(), "INA3221 drv lost");

    if(const auto res = read_reg(chip_id_reg); res.is_err()) return res;
    if(const auto res = read_reg(manu_id_reg); res.is_err()) return res;

    if(chip_id_reg.key != chip_id_reg.as_val()) return Err(Error::WrongChipId);
    if(manu_id_reg.key == manu_id_reg.as_val()) return Err(Error::WrongManuId);

    return Ok();
}

IResult<> INA3221::update(){
    return read_burst(shuntvolt1_reg.address, &shuntvolt1_reg, 6);
}

IResult<> INA3221::update(const ChannelIndex index){
    return read_burst(
        shuntvolt1_reg.address + ((uint8_t(index) - 1) * 2), 
        &shuntvolt1_reg + ((uint8_t(index) - 1) * 2), 2);
    // read_reg(config_reg);
    // INA3221_DEBUG(config_reg.as_val());
    // switch(index){
    //     case 1: read_reg(shuntvolt1_reg); read_reg(busvolt1_reg); break;
    //     case 2: read_reg(shuntvolt2_reg); read_reg(busvolt2_reg); break;
    //     case 3: read_reg(shuntvolt3_reg); read_reg(busvolt3_reg); break;
    // }
    // return *this;
} 

IResult<> INA3221::set_average_times(const uint16_t times){
    uint8_t temp = CTZ(times);
    uint8_t temp2;

    if(times <= 64){
        temp2 = temp >> 1;
    }else{
        temp2 = 4 + (temp - 7); 
    }

    config_reg.average_times = temp2;
    return write_reg(config_reg);
}

IResult<> INA3221::set_average_times(const AverageTimes times){
    config_reg.average_times = uint8_t(times);
    return write_reg(config_reg);
}

IResult<> INA3221::enable_channel(const ChannelIndex index, const bool en){
    switch(index){
        default: INA3221_PANIC();
        case ChannelIndex::CH1:
            config_reg.ch1_en = en;
            break;
        case ChannelIndex::CH2:
            config_reg.ch2_en = en;
            break;
        case ChannelIndex::CH3:
            config_reg.ch3_en = en;
            break;
    }
    return write_reg(config_reg);
}


IResult<> INA3221::set_bus_conversion_time(const ConversionTime time){
    config_reg.bus_conv_time = uint8_t(time);
    return write_reg(config_reg);
}


IResult<> INA3221::set_shunt_conversion_time(const ConversionTime time){
    config_reg.shunt_conv_time = uint8_t(time);
    return write_reg(config_reg);
}


IResult<> INA3221::reset(){
    config_reg.rst = true;
    const auto res = write_reg(config_reg);
    config_reg.rst = false;
    return res;
}


IResult<int> INA3221::get_shunt_volt_uv(const ChannelIndex index){

    RegAddress addr;
    ShuntVoltReg & reg = [&]() -> ShuntVoltReg &{
        switch(index){
        default: INA3221_PANIC();
        case ChannelIndex::CH1:
            addr = shuntvolt1_reg.address;
            return shuntvolt1_reg;
        case ChannelIndex::CH2:
            addr = shuntvolt2_reg.address;
            return shuntvolt2_reg;
        case ChannelIndex::CH3:
            addr = shuntvolt3_reg.address;
            return shuntvolt3_reg;
        }
    }();

    const auto res = read_reg(addr, reg.as_ref());
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.to_uv());
}



IResult<int> INA3221::get_bus_volt_mv(const ChannelIndex index){
    RegAddress addr;
    BusVoltReg & reg = [&]() -> BusVoltReg &{
        switch(index){
        default: INA3221_PANIC();
        case ChannelIndex::CH1:
            addr = busvolt1_reg.address;
            return busvolt1_reg;
        case ChannelIndex::CH2:
            addr = busvolt2_reg.address;
            return busvolt2_reg;
        case ChannelIndex::CH3:
            addr = busvolt3_reg.address;
            return busvolt3_reg;
        }
    }();

    if(const auto res = read_reg(addr, reg.as_ref()); res.is_err())
        return Err(res.unwrap_err());

    return Ok(reg.to_mv());
}


IResult<real_t> INA3221::get_shunt_volt(const ChannelIndex index){
    const auto res = get_shunt_volt_uv(index);
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok(iq_t<16>(iq_t<8>(res.unwrap()) / 100) / 10000);
}

IResult<real_t> INA3221::get_bus_volt(const ChannelIndex index){
    const auto res = get_bus_volt_mv(index);
    if(res.is_err()) return Err(res.unwrap_err());
    return Ok(real_t(res.unwrap()) / 1000);
}


IResult<> INA3221::set_instant_ovc(const ChannelIndex index, const real_t volt){
    const RegAddress addr = [&]{
        switch(index){
            case ChannelIndex::CH1: return instant_ovc1_reg.address1; 
            case ChannelIndex::CH2: return instant_ovc1_reg.address2; 
            case ChannelIndex::CH3: return instant_ovc1_reg.address3; 
            default: __builtin_unreachable();
        }
    }();

    return write_reg(addr, ShuntVoltReg::to_i16(volt));
}


IResult<> INA3221::set_constant_ovc(const ChannelIndex index, const real_t volt){
    const RegAddress addr = [&]{
        switch(index){
            case ChannelIndex::CH1: return constant_ovc1_reg.address1; 
            case ChannelIndex::CH2: return constant_ovc1_reg.address2; 
            case ChannelIndex::CH3: return constant_ovc1_reg.address3; 
            default: __builtin_unreachable();
        }
    }();

    return write_reg(addr, ShuntVoltReg::to_i16(volt));
}

IResult<> INA3221::enable_measure_bus(const bool en){
    config_reg.bus_measure_en = en;
    return write_reg(config_reg);
}


IResult<> INA3221::enable_measure_shunt(const bool en){
    config_reg.shunt_measure_en = en;
    return write_reg(config_reg);
}

IResult<> INA3221::enable_continuous(const bool en){
    config_reg.continuos = en;
    return write_reg(config_reg);
}