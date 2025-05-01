#include "ina3221.hpp"
#include "core/debug/debug.hpp"

using namespace ymd::drivers;

#define INA3221_DEBUG_EN

#ifdef INA3221_DEBUG_EN
#define INA3221_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define INA3221_PANIC(...) PANIC{__VA_ARGS__}
#define INA3221_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}

#define CHECKRES(x, ...) ({\
    const auto __res = (x);\
    ASSERT{__res.is_ok(), ##__VA_ARGS__};\
    __res;\
})\


#define CHECKERR(x, ...) ({\
    const auto && __err = x;\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err;\
})\


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
    if(const auto res = read_reg(chip_id_reg); res.is_err()) return CHECKRES(res);
    if(const auto res = read_reg(manu_id_reg); res.is_err()) return CHECKRES(res);

    if(chip_id_reg.key != chip_id_reg.as_val()) return CHECKERR(Err(Error::WrongChipId));
    if(manu_id_reg.key != manu_id_reg.as_val()) return CHECKERR(Err(Error::WrongManuId));

    return Ok();
}

IResult<> INA3221::update(const ChannelIndex index){

    // update bus and shunt
    switch(index){
        default: __builtin_unreachable();
        case ChannelIndex::CH1: 
            return read_reg(shuntvolt1_reg) | read_reg(busvolt1_reg);
        case ChannelIndex::CH2: 
            return read_reg(shuntvolt2_reg) | read_reg(busvolt2_reg);
        case ChannelIndex::CH3: 
            return read_reg(shuntvolt3_reg) | read_reg(busvolt3_reg);
    }
} 

IResult<> INA3221::set_average_times(const AverageTimes times){
    config_reg.average_times = times.as_raw();
    return write_reg(config_reg);
}

IResult<> INA3221::enable_channel(const ChannelIndex index, const Enable en){
    switch(index){
        default: __builtin_unreachable();
        case ChannelIndex::CH1:
            config_reg.ch1_en = bool(en);
            break;
        case ChannelIndex::CH2:
            config_reg.ch2_en = bool(en);
            break;
        case ChannelIndex::CH3:
            config_reg.ch3_en = bool(en);
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

    // RegAddress addr;
    const R16_ShuntVolt & reg = [&]() -> const R16_ShuntVolt &{
    switch(index){
        default: __builtin_unreachable();
        case ChannelIndex::CH1:return shuntvolt1_reg;
        case ChannelIndex::CH2:return shuntvolt2_reg;
        case ChannelIndex::CH3:return shuntvolt3_reg;
        }
    }();

    // const auto res = read_reg(addr, reg.as_ref());
    // if(res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.to_uv());
}



IResult<int> INA3221::get_bus_volt_mv(const ChannelIndex index){
    // RegAddress addr;
    const R16_BusVolt & reg = [&]() -> const R16_BusVolt &{
        switch(index){
            default: __builtin_unreachable();
            case ChannelIndex::CH1:return busvolt1_reg;
            case ChannelIndex::CH2:return busvolt2_reg;
            case ChannelIndex::CH3:return busvolt3_reg;
        }
    }();

    // if(const auto res = read_reg(addr, reg.as_ref()); res.is_err())
    //     return Err(res.unwrap_err());

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
            case ChannelIndex::CH1: return instant_ovc1_reg.address; 
            case ChannelIndex::CH2: return instant_ovc1_reg.address; 
            case ChannelIndex::CH3: return instant_ovc1_reg.address; 
            default: __builtin_unreachable();
        }
    }();

    return write_reg(addr, R16_ShuntVolt::to_i16(volt));
}


IResult<> INA3221::set_constant_ovc(const ChannelIndex index, const real_t volt){
    const RegAddress addr = [&]{
        switch(index){
            case ChannelIndex::CH1: return constant_ovc1_reg.address; 
            case ChannelIndex::CH2: return constant_ovc1_reg.address; 
            case ChannelIndex::CH3: return constant_ovc1_reg.address; 
            default: __builtin_unreachable();
        }
    }();

    return write_reg(addr, R16_ShuntVolt::to_i16(volt));
}

IResult<> INA3221::enable_measure_bus(const Enable en){
    config_reg.bus_measure_en = bool(en);
    return write_reg(config_reg);
}


IResult<> INA3221::enable_measure_shunt(const Enable en){
    config_reg.shunt_measure_en = bool(en);
    return write_reg(config_reg);
}

IResult<> INA3221::enable_continuous(const Enable en){
    config_reg.continuos = bool(en);
    return write_reg(config_reg);
}