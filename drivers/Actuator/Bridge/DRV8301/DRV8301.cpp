#include "DRV8301.hpp"


#define DRV8301_DEBUG

#ifdef DRV8301_DEBUG
#undef DRV8301_DEBUG
#define DRV8301_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define DRV8301_PANIC(...) PANIC(__VA_ARGS__)
#define DRV8301_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define DRV8301_DEBUG(...)
#define DRV8301_PANIC(...)  PANIC()
#define DRV8301_ASSERT(cond, ...) ASSERT(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

#define WRITE_REG(reg) write_reg(reg.address, reg.as_val());
#define READ_REG(reg) read_reg(reg.address, reg.as_ref());

void DRV8301::init(){

}

void DRV8301::setPeakCurrent(const PeakCurrent peak_current){
    auto & reg = ctrl1_reg;
    reg.gate_current = uint16_t(peak_current);
    WRITE_REG(reg);
}

void DRV8301::setOcpMode(const OcpMode ocp_mode){
    auto & reg = ctrl1_reg;
    reg.ocp_mode = uint16_t(ocp_mode);
    WRITE_REG(reg);
}


void DRV8301::setOctwMode(const OctwMode octw_mode){
    auto & reg = ctrl2_reg;
    reg.octw_mode = uint16_t(octw_mode);
    WRITE_REG(reg);
}

void DRV8301::setGain(const Gain gain){
    auto & reg = ctrl2_reg;
    reg.gain = uint16_t(gain);
    WRITE_REG(reg);
}

void DRV8301::setOcAdTable(const OcAdTable oc_ad_table){
    auto & reg = ctrl1_reg;
    reg.oc_adj_set = uint8_t(oc_ad_table);
    WRITE_REG(reg);
}

void DRV8301::enablePwm3(const bool en){
    auto & reg = ctrl1_reg;
    reg.pwm3_en = en;
    WRITE_REG(reg);
}

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

hal::BusError DRV8301::write_reg(const RegAddress addr, const uint16_t reg){
    const SpiFormat spi_format = {
        .data = reg,
        .addr = uint16_t(addr),
        .write = 0
    };

    return spi_drv_.write_single<uint16_t>((spi_format));
}

hal::BusError DRV8301::read_reg(const RegAddress addr, uint16_t & reg){
    SpiFormat spi_format = {
        .data = 0,
        .addr = uint16_t(addr),
        .write = 1
    };

    const auto err = spi_drv_.read_single<uint16_t>((spi_format));
    if(err.is_err()) return err;
    reg = spi_format.data;

    return hal::BusError::Ok();
}