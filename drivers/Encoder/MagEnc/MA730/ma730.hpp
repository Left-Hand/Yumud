#pragma once

#include "core/io/regs.hpp"

#include "core/utils/Result.hpp"
#include "drivers/Encoder/MagEncoder.hpp"

#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{

struct MA730_Prelude{
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

struct MA730_Regs:public MA730_Prelude{

    struct ZeroDataLowReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::ZeroDataLow;
        uint8_t data;
    };

    struct ZeroDataHighReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::ZeroDataHigh;
        uint8_t data;
    };

    struct TrimReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Trim;
        uint8_t trim;
    };
    struct TrimConfigReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::TrimConfig;
        uint8_t enableX:1;
        uint8_t enableY:1;
        uint8_t :6;
    };

    struct ZParametersReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::ZParameters;
        uint8_t :2;
        Phase zPhase :2;
        Width zWidth :2;
        uint8_t ppt:2;
    };

    struct PulsePerTurnReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::PulsePerTurn;
        uint8_t data;
    };

    struct ThresholdReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Threshold;
        uint8_t :2;
        uint8_t thresholdHigh :3;
        uint8_t thresholdLow :3;
    };

    struct DirectionReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Direction;
        uint8_t :7;
        uint8_t direction :1;
    };

    struct MagnitudeReg:public Reg8<>{
        static constexpr auto ADDRESS = RegAddress::Magnitude;
        uint8_t :2;
        uint8_t mgl1:1;
        uint8_t mgl2:1;
        uint8_t :2;
        uint8_t magnitudeLow :1;
        uint8_t magnitudeHigh :1;
    };

    ZeroDataLowReg zero_data_low_reg = {};
    ZeroDataHighReg zero_data_high_reg = {};
    TrimReg trim_reg = {};

    TrimConfigReg trim_config_reg = {};
    ZParametersReg z_parameters_reg = {};
    PulsePerTurnReg pulse_per_turn_reg = {};
    ThresholdReg threshold_reg = {};
    DirectionReg direction_reg = {};
    MagnitudeReg magnitude_reg = {};
};

class MA730 final:
    public MagEncoderIntf,
    public MA730_Regs{
public:
    MA730(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    MA730(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
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

    template<typename T>
    [[nodiscard]] IResult<> write_reg(const RegCopy<T> & reg){
        const auto address = T::ADDRESS;
        const uint8_t data = reg.as_val();
        const auto tx = uint16_t(
            0x8000 | (std::bit_cast<uint8_t>(address) << 8) | data);
        if(const auto res = spi_drv_.write_single<uint16_t>(tx);
            res.is_err()) return Err(Error(res.unwrap_err()));
        reg.apply();
        return Ok();
    }


    template<typename T>
    [[nodiscard]] IResult<> read_reg(T & reg){
        uint16_t dummy;
        const auto addr = std::bit_cast<uint8_t>(T::ADDRESS);
        const auto tx = uint16_t(0x4000 | ((uint8_t)addr << 8));
        if(const auto res = spi_drv_.write_single<uint16_t>(tx); 
            res.is_err()) return Err(Error(res.unwrap_err()));
        if(const auto res = spi_drv_.read_single<uint16_t>(dummy);
            res.is_err()) return Err(Error(res.unwrap_err()));
        if((dummy & 0xff) != 0x00) 
            return Err(Error(Error::Kind::InvalidRxFormat));
        reg.as_ref() = (dummy >> 8);
        return Ok();
    }

    [[nodiscard]]
    IResult<uint16_t> direct_read();
    
    [[nodiscard]]
    IResult<uint16_t> get_raw_data();
    
    [[nodiscard]]
    IResult<> set_zero_data(const uint16_t data);
};

};