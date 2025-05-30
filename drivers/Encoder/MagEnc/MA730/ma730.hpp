#pragma once

#include "core/io/regs.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct MA730_Collections{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class Width:uint8_t{
        W90, W180, W270, W360
    };

    enum class Phase:uint8_t{
        P0, P90, P180, P270
    };

    enum class MagThreshold:uint8_t{
        mT23, mT38, mT53, mT67, mT81, mT95, mT109, mT123
    };

    enum class RegAddress:uint8_t{
        ZeroDataLow,
        ZeroDataHigh,
        Trim,
        TrimConfig,
        ZParameters,
        PulsePerTurn,
        Threshold,
        Direction = 9,
        Magnitude = 27
    };

};

struct MA730_Regs:public MA730_Collections{
    struct TrimConfigReg:public Reg8<>{
        uint8_t enableX:1;
        uint8_t enableY:1;
        uint8_t :6;
    };

    struct ZParametersReg:public Reg8<>{
        uint8_t :2;
        uint8_t zPhase :2;
        uint8_t zWidth :2;
        uint8_t ppt:2;
    };

    struct ThresholdReg:public Reg8<>{
        uint8_t :2;
        uint8_t thresholdHigh :3;
        uint8_t thresholdLow :3;
    };

    struct DirectionReg:public Reg8<>{
        uint8_t :7;
        uint8_t direction :1;
    };

    struct MagnitudeReg:public Reg8<>{
        uint8_t :2;
        uint8_t mgl1:1;
        uint8_t mgl2:1;
        uint8_t :2;
        uint8_t magnitudeLow :1;
        uint8_t magnitudeHigh :1;
    };

    uint16_t zeroDataReg = {};
    uint8_t trimReg = {};
    TrimConfigReg trimConfigReg = {};
    ZParametersReg zParametersReg = {};
    uint8_t pulsePerTurnReg = {};
    ThresholdReg thresholdReg = {};
    DirectionReg directionReg = {};
    MagnitudeReg magnitudeReg = {};
};

class MA730 final:
    public MagEncoderIntf,
    public MA730_Regs{
public:
    MA730(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    MA730(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    MA730(hal::Spi & spi, const hal::SpiSlaveIndex index):
        spi_drv_(hal::SpiDrv(spi, index)){;}


    [[nodiscard]] IResult<> init();
    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_zero_position(const real_t position);
    [[nodiscard]] IResult<real_t> get_lap_position(){
        return Ok(lap_position_);
    }

    [[nodiscard]] IResult<> set_trim_x(const real_t k);

    [[nodiscard]] IResult<> set_trim_y(const real_t k);

    [[nodiscard]] IResult<> set_trim(const real_t am, const real_t e);

    [[nodiscard]] IResult<> set_mag_threshold(
        const MagThreshold low, const MagThreshold high);

    [[nodiscard]] IResult<> set_direction(const bool direction);
    [[nodiscard]] IResult<MagStatus> get_mag_status();

    [[nodiscard]]
    IResult<> set_zparameters(const Width width, const Phase phase);

    [[nodiscard]]
    IResult<> set_pulse_per_turn(const uint16_t ppt);
private:
    hal::SpiDrv spi_drv_;
    real_t lap_position_ = {};

    [[nodiscard]]
    IResult<> write_reg(const RegAddress addr, uint8_t data);

    [[nodiscard]]
    IResult<> read_reg(const RegAddress addr, uint8_t & reg);

    [[nodiscard]]
    IResult<uint16_t> direct_read();
    
    [[nodiscard]]
    IResult<uint16_t> get_raw_data();
    
    [[nodiscard]]
    IResult<> set_zero_data(const uint16_t data);
};

};