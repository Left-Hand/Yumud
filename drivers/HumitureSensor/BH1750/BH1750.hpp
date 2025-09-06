#pragma once

#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/math/fraction.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers{

class BH1750 final{
public:
    enum class Mode:uint8_t{
        HMode = 0,
        HMode2 = 1,
        LMode = 3
    };

    enum class Error_Kind{

    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
public:
    explicit BH1750(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    explicit BH1750(hal::I2cDrv && i2c_drv):i2c_drv_(std::move(i2c_drv)){;}

    IResult<> power_down(){
        return send_command(Command::PowerDown);
    }

    IResult<> reset(){
        return send_command(Command::Reset);
    }

    void set_mode(const Mode mode){
        current_mode_ = mode;
    }

    void enable_continuous(const Enable en){
        cont_en_ = en == EN;
    }

    IResult<> start_conv();

    IResult<> change_measure_time(const uint16_t ms);

    IResult<uint32_t> get_lx();

private:
    hal::I2cDrv i2c_drv_;

    enum class Command:uint8_t{
        PowerDown = 0,
        PowerOn = 1,
        Reset = 7,
        ChangeMeasureTimeH = 0x40,
        ChangeMeasureTimeL = 0x60,
    };

    Fraction lsb = {
        .numerator = 69,
        .denominator = 69
    };

    Mode current_mode_ = Mode::LMode;
    bool cont_en_ = false;

    IResult<> send_command(const uint8_t cmd);
    IResult<> send_command(const Command cmd){
        return send_command(std::bit_cast<uint8_t>(cmd));
    }

};

};