#include "DRV832X.hpp"


#define DRV832X_DEBUG_EN

#ifdef DRV832X_DEBUG_EN
#define DRV832X_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define DRV832X_PANIC(...) PANIC(__VA_ARGS__)
#define DRV832X_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define DRV832X_DEBUG(...)
#define DRV832X_PANIC(...)  PANIC()
#define DRV832X_ASSERT(cond, ...) ASSERT(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

using Error = DRV8323R::Error;

template<typename T = void>
using IResult = Result<T, Error>;



IResult<> DRV8323R::init(const Config & cfg){

    TODO();
    return Ok();
}

IResult<> DRV8323R::reconf(const Config & cfg){
    TODO();
    return Ok();
}

IResult<> DRV8323R::set_peak_current(const PeakCurrent peak_current){
    TODO();
    return Ok();
}

IResult<> DRV8323R::set_ocp_mode(const OcpMode ocp_mode){
    TODO();
    return Ok();
}

IResult<> DRV8323R::set_gain(const Gain gain){
    TODO();
    return Ok();
}

IResult<> DRV8323R::enable_pwm3(const Enable en){
    TODO();
    return Ok();
}

IResult<> DRV8323R::set_drive_hs(const IDriveP pdrive, const IDriveN ndrive){
    auto reg = RegCopy(gate_drv_hs_reg);
    reg.idrive_p_hs = pdrive;
    reg.idrive_n_hs = ndrive;

    return write_reg(reg);
}

IResult<> DRV8323R::set_drive_ls(const IDriveP pdrive, const IDriveN ndrive){
    auto reg = RegCopy(gate_drv_ls_reg);
    reg.idrive_p_ls = pdrive;
    reg.idrive_n_ls = ndrive;

    return write_reg(reg);
}

IResult<> DRV8323R::set_drive_time(const PeakDriveTime ptime){
    auto reg = RegCopy(gate_drv_ls_reg);
    reg.tdrive = ptime;

    return write_reg(reg);
}

IResult<DRV8323R::R16_Status1> DRV8323R::get_status1(){
    if(const auto res = read_reg(status1_reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(status1_reg);
}

IResult<DRV8323R::R16_Status2> DRV8323R::get_status2(){
    if(const auto res = read_reg(status2_reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(status2_reg);
}


struct SpiFormat{
    uint16_t data:11;
    uint16_t addr:4;
    uint16_t is_write:1;

    uint16_t as_val() const {
        return std::bit_cast<uint16_t>(*this);
    }

    uint16_t & as_ref(){
        return *reinterpret_cast<uint16_t *>(this);
    }
};

static_assert(sizeof(SpiFormat) == sizeof(uint16_t));

IResult<> DRV8323R_Phy::write_reg(const RegAddress addr, const uint16_t reg){
    const SpiFormat spi_format = {
        .data = reg,
        .addr = uint16_t(addr),
        .is_write = 0
    };

    if(const auto res = spi_drv_.write_single<uint16_t>((spi_format));
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> DRV8323R_Phy::read_reg(const RegAddress addr, uint16_t & reg){
    SpiFormat spi_format = {
        .data = 0,
        .addr = uint16_t(addr),
        .is_write = 1
    };

    const auto res = spi_drv_.read_single<uint16_t>((spi_format));
    if(res.is_err()) return Err(res.unwrap_err());
    reg = spi_format.data;

    return Ok();
}
