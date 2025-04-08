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

struct SpiFormat{
    uint16_t data:11;
    uint16_t addr:4;
    uint16_t write:1;

    operator uint16_t() const{
        return std::bit_cast<uint16_t>(*this);
    }

    operator uint16_t &(){
        return *reinterpret_cast<uint16_t *>(this);
    }
};

BusError DRV832X::write_reg(const RegAddress addr, const uint16_t reg){
    const SpiFormat spi_format = {
        .data = reg,
        .addr = uint16_t(addr),
        .write = 0
    };

    return spi_drv_.write_single<uint16_t>((spi_format));
}

BusError DRV832X::read_reg(const RegAddress addr, uint16_t & reg){
    SpiFormat spi_format = {
        .data = 0,
        .addr = uint16_t(addr),
        .write = 1
    };

    const auto err = spi_drv_.read_single<uint16_t>((spi_format)).unwrap();

    reg = spi_format.data;

    return err;
}


Result<void, Error> DRV832X::set_drive_hs(const IDriveP pdrive, const IDriveN ndrive){
    auto & reg = regs_.gate_drv_hs;
    reg.idrive_p_hs = uint8_t(pdrive);
    reg.idrive_n_hs = uint8_t(ndrive);

    return write_reg(reg);
}

Result<void, Error> DRV832X::set_drive_ls(const IDriveP pdrive, const IDriveN ndrive){
    auto & reg = regs_.gate_drv_ls;
    reg.idrive_p_ls = uint8_t(pdrive);
    reg.idrive_n_ls = uint8_t(ndrive);

    return write_reg(reg);
}

Result<void, Error> DRV832X::set_drive_time(const PeakDriveTime ptime){
    auto & reg = regs_.gate_drv_ls;
    reg.tdrive = uint8_t(ptime);

    return write_reg(reg);
}