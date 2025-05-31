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

using Error = DRV832X::Error;

template<typename T = void>
using IResult = Result<T, Error>;

struct SpiFormat{
    uint16_t data:11;
    uint16_t addr:4;
    uint16_t write:1;

    uint16_t as_val() const {
        return std::bit_cast<uint16_t>(*this);
    }

    uint16_t & as_ref(){
        return *reinterpret_cast<uint16_t *>(this);
    }
};

IResult<> DRV832X::write_reg(const RegAddress addr, const uint16_t reg){
    const SpiFormat spi_format = {
        .data = reg,
        .addr = uint16_t(addr),
        .write = 0
    };

    if(const auto res = spi_drv_.write_single<uint16_t>((spi_format));
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

IResult<> DRV832X::read_reg(const RegAddress addr, uint16_t & reg){
    SpiFormat spi_format = {
        .data = 0,
        .addr = uint16_t(addr),
        .write = 1
    };

    const auto res = spi_drv_.read_single<uint16_t>((spi_format));
    if(res.is_err()) return Err(res.unwrap_err());
    reg = spi_format.data;

    return Ok();
}


IResult<> DRV832X::set_drive_hs(const IDriveP pdrive, const IDriveN ndrive){
    auto reg = RegCopy(gate_drv_hs_reg);
    reg.idrive_p_hs = pdrive;
    reg.idrive_n_hs = ndrive;

    return write_reg(reg);
}

IResult<> DRV832X::set_drive_ls(const IDriveP pdrive, const IDriveN ndrive){
    auto reg = RegCopy(gate_drv_ls_reg);
    reg.idrive_p_ls = pdrive;
    reg.idrive_n_ls = ndrive;

    return write_reg(reg);
}

IResult<> DRV832X::set_drive_time(const PeakDriveTime ptime){
    auto reg = RegCopy(gate_drv_ls_reg);
    reg.tdrive = ptime;

    return write_reg(reg);
}