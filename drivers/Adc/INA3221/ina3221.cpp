#include "ina3221.hpp"
#include "core/debug/debug.hpp"
#include "core/utils/scope_guard.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define INA3221_DEBUG_EN 0

#if INA3221_DEBUG_EN == 1
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

#define CHECKRES(x, ...) (x)
#define CHECKERR(x, ...) (x)
#endif

using Self = INA3221;
template<typename T = void>
using IResult = Self::IResult<T>;

using Error = Self::Error;
using Regs = INA3221_Regs;

IResult<> INA3221::set_average_times(const AverageTimes average_times){
    auto reg = RegCopy(regs_.config_reg);
    reg.average_times = average_times.kind();
    return write_reg(reg);
}

IResult<> INA3221::enable_channel(const ChannelSelection ch_sel, const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    switch(ch_sel){
        case ChannelSelection::CH1:
            reg.ch1_en = (en == EN);
            break;
        case ChannelSelection::CH2:
            reg.ch2_en = (en == EN);
            break;
        case ChannelSelection::CH3:
            reg.ch3_en = (en == EN);
            break;
    }
    return write_reg(reg);
}


IResult<> INA3221::set_bus_conversion_time(const ConversionTime time){
    auto reg = RegCopy(regs_.config_reg);
    reg.bus_conv_time = time;
    return write_reg(reg);
}


IResult<> INA3221::set_shunt_conversion_time(const ConversionTime time){
    auto reg = RegCopy(regs_.config_reg);
    reg.shunt_conv_time = time;
    return write_reg(reg);
}


IResult<> INA3221::reset(){
    auto reg = RegCopy(regs_.config_reg);
    auto gaurd = make_scope_guard([&](){
        reg.rst = false;
        reg.apply();
    });

    reg.rst = true;
    return write_reg(reg);
}


IResult<Self::ShuntVoltCode> INA3221::get_shunt_volt_code(const ChannelSelection ch_sel){
    switch(ch_sel){
        case ChannelSelection::CH1:return Ok(regs_.shuntvolt1_reg.code);
        case ChannelSelection::CH2:return Ok(regs_.shuntvolt2_reg.code);
        case ChannelSelection::CH3:return Ok(regs_.shuntvolt3_reg.code);
    }
    __builtin_unreachable();
}

IResult<Self::BusVoltCode> INA3221::get_bus_volt_code(const ChannelSelection ch_sel){
    switch(ch_sel){
        case ChannelSelection::CH1:return Ok(regs_.busvolt1_reg.code);
        case ChannelSelection::CH2:return Ok(regs_.busvolt2_reg.code);
        case ChannelSelection::CH3:return Ok(regs_.busvolt3_reg.code);
    }
    __builtin_unreachable();
}


IResult<> INA3221::set_instant_ovc_threshold(const ChannelSelection ch_sel, const ShuntVoltCode volt_code){
    const RegAddr reg_addr = [&]{
        switch(ch_sel){
            case ChannelSelection::CH1: return Regs::R16_InstantOVC1::REG_ADDR; 
            case ChannelSelection::CH2: return Regs::R16_InstantOVC2::REG_ADDR; 
            case ChannelSelection::CH3: return Regs::R16_InstantOVC3::REG_ADDR; 
        }
        __builtin_unreachable();
    }();

    return write_reg(reg_addr, volt_code.bits);
}


IResult<> INA3221::set_constant_ovc_threshold(const ChannelSelection ch_sel, const ShuntVoltCode volt_code){
    const RegAddr reg_addr = [&]{
        switch(ch_sel){
            case ChannelSelection::CH1: return Regs::R16_ConstantOVC1::REG_ADDR; 
            case ChannelSelection::CH2: return Regs::R16_ConstantOVC2::REG_ADDR; 
            case ChannelSelection::CH3: return Regs::R16_ConstantOVC3::REG_ADDR; 
        }
        __builtin_unreachable();
    }();

    return write_reg(reg_addr, volt_code.bits);
}

IResult<> INA3221::enable_measure_bus(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.bus_measure_en = (en == EN);
    return write_reg(reg);
}


IResult<> INA3221::enable_measure_shunt(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.shunt_measure_en = (en == EN);
    return write_reg(reg);
}

IResult<> INA3221::enable_continuous(const Enable en){
    auto reg = RegCopy(regs_.config_reg);
    reg.continuous_en = (en == EN);
    return write_reg(reg);
}

IResult<> INA3221::init(const Config & cfg){
    if(const auto res = this->validate(); 
        res.is_err()) return CHECKRES(res, "INA3221 verify failed");

    if(const auto res = this->enable_channel(ChannelSelection::CH1, EN);
        res.is_err()) return res;
    if(const auto res = this->enable_channel(ChannelSelection::CH2, EN);
        res.is_err()) return res;
    if(const auto res = this->enable_channel(ChannelSelection::CH3, EN);
        res.is_err()) return res;
    if(const auto res = this->reconf(cfg);
        res.is_err()) return res;
    if(const auto res = this->enable_continuous(EN);
        res.is_err()) return res;
    if(const auto res = this->enable_measure_bus(EN);
        res.is_err()) return res;
    if(const auto res = this->enable_measure_shunt(EN);
        res.is_err()) return res;

    return Ok();
}

IResult<> INA3221::reconf(const Config & cfg){
    auto reg = RegCopy(regs_.config_reg);

    reg.bus_conv_time = cfg.bus_conv_time;
    reg.shunt_conv_time = cfg.shunt_conv_time;
    reg.average_times = cfg.average_times.kind();
    
    return write_reg(reg);
}

IResult<bool> INA3221::is_ready(){
    TODO();
    return Ok(true);
}


IResult<> INA3221::validate(){
    {
        Regs::R16_ChipId chip_id_reg = {};
        
        if(const auto res = read_reg(chip_id_reg); 
            res.is_err()) return CHECKRES(res);
        
        if(chip_id_reg.KEY != chip_id_reg.to_bits()) 
            return CHECKERR(Err(Error::ChipIdMismatch));
    }
    
    {
        Regs::R16_ManuId manu_id_reg = {};
    
        if(const auto res = read_reg(manu_id_reg); 
            res.is_err()) return CHECKRES(res);
    
        if(manu_id_reg.KEY != manu_id_reg.to_bits()) 
            return CHECKERR(Err(Error::ManuIdMisMatch));
    }

    return Ok();
}

IResult<> INA3221::update(const ChannelSelection ch_sel){
    #define READ_DUAL_REG(r1, r2)\
        if(const auto res = read_reg(r1); \
            res.is_err()) return CHECKRES(res);\
        if(const auto res = read_reg(r2); \
            res.is_err()) return CHECKRES(res);\
        return Ok();\

    // update bus and shunt
    switch(ch_sel){
        case ChannelSelection::CH1: 
            READ_DUAL_REG(regs_.shuntvolt1_reg, regs_.busvolt1_reg);
        case ChannelSelection::CH2: 
            READ_DUAL_REG(regs_.shuntvolt2_reg, regs_.busvolt2_reg);
        case ChannelSelection::CH3: 
            READ_DUAL_REG(regs_.shuntvolt3_reg, regs_.busvolt3_reg);
    }
    __builtin_unreachable();

    #undef READ_DUAL_REG
} 