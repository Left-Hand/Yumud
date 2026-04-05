#include "drv8323r.hpp"


#define DRV832X_DEBUG_EN

#ifdef DRV832X_DEBUG_EN
#define DRV832X_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define DRV832X_PANIC(...) PANIC(__VA_ARGS__)
#define DRV832X_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define DRV832X_DEBUG(...)
#define DRV832X_PANIC(...)  PANIC_NSRC()
#define DRV832X_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

using Error = DRV8323R::Error;

template<typename T = void>
using IResult = Result<T, Error>;



IResult<> DRV8323R::init(const Config & cfg){
    (void)(cfg);
    TODO();
    return Ok();
}

IResult<> DRV8323R::reconf(const Config & cfg){
    (void)(cfg);
    TODO();
    return Ok();
}

IResult<> DRV8323R::set_peak_current(const PeakCurrent peak_current){
    (void)(peak_current);
    TODO();
    return Ok();
}

IResult<> DRV8323R::set_ocp_mode(const OcpMode ocp_mode){
    (void)(ocp_mode);
    TODO();
    return Ok();
}

IResult<> DRV8323R::set_gain(const Gain gain){
    (void)(gain);
    TODO();
    return Ok();
}

IResult<> DRV8323R::enable_pwm3(const Enable en){
    (void)(en);
    TODO();
    return Ok();
}

IResult<> DRV8323R::set_drive_hs(const IDriveP pdrive, const IDriveN ndrive){
    auto reg = RegCopy(regs_.gate_drv_hs_reg);
    reg.idrive_p_hs = pdrive;
    reg.idrive_n_hs = ndrive;

    return write_reg(reg);
}

IResult<> DRV8323R::set_drive_ls(const IDriveP pdrive, const IDriveN ndrive){
    auto reg = RegCopy(regs_.gate_drv_ls_reg);
    reg.idrive_p_ls = pdrive;
    reg.idrive_n_ls = ndrive;

    return write_reg(reg);
}

IResult<> DRV8323R::set_drive_time(const PeakDriveTime ptime){
    auto reg = RegCopy(regs_.gate_drv_ls_reg);
    reg.tdrive = ptime;

    return write_reg(reg);
}

IResult<DRV8323R::Status1> DRV8323R::get_status1(){
    auto & reg = regs_.status1_reg;
    if(const auto res = read_reg(reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.status);
}

IResult<DRV8323R::Status2> DRV8323R::get_status2(){
    auto & reg = regs_.status2_reg;
    if(const auto res = read_reg(reg); 
        res.is_err()) return Err(res.unwrap_err());
    return Ok(reg.status);
}


namespace {
struct [[nodiscard]] Packet final{
    uint16_t data:11;
    uint16_t reg_addr:4;
    uint16_t is_write:1;

    [[nodiscard]] constexpr uint16_t to_bits() const {
        return std::bit_cast<uint16_t>(*this);
    }

    [[nodiscard]] uint16_t & as_bits_mut(){
        return *reinterpret_cast<uint16_t *>(this);
    }
};

static_assert(sizeof(Packet) == sizeof(uint16_t));
}



IResult<> DRV8323R_Transport::write_reg(const RegAddr reg_addr, const uint16_t reg_val){
    const Packet packet = {
        .data = reg_val,
        .reg_addr = uint16_t(reg_addr),
        .is_write = 0
    };

    if(const auto res = spi_drv_.write_single<uint16_t>((packet.to_bits()));
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> DRV8323R_Transport::read_reg(const RegAddr reg_addr, uint16_t & reg_val){
    Packet packet = {
        .data = 0,
        .reg_addr = uint16_t(reg_addr),
        .is_write = 1
    };

    if(const auto res = spi_drv_.read_single<uint16_t>((packet.as_bits_mut()));
        res.is_err()) return Err(res.unwrap_err());
    reg_val = packet.data;

    return Ok();
}
