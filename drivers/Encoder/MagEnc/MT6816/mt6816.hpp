#pragma once

//不推荐用于新设计 性价比不高 

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"

namespace ymd::drivers{

class MT6816 final:public MagEncoderIntf{
public:
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    struct Config{
        Enable fast_mode_en = EN;
    };

public:
    MT6816(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}

    MT6816(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}

    MT6816(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index):
        spi_drv_(hal::SpiDrv{spi, index}){;}

    IResult<> init(const Config & cfg);
    IResult<> reconf(const Config & cfg);
    IResult<> update();

    IResult<real_t> get_lap_position() { return Ok(lap_position_);}
    uint32_t get_err_cnt() const {return err_cnt_;}

    IResult<MagStatus> get_mag_status() {
        if(last_sema_.no_mag){
            return Ok(MagStatus::Low());
        }else{
            return Ok(MagStatus::Proper());
        }
    }

private:
    struct Semantic{
        uint16_t pc:1;
        uint16_t no_mag:1;
        uint16_t data_14bit:14;

        // constexpr Semantic() : pc(0), no_mag(0), data_14bit(0) {}

        Semantic(uint16_t data){
            reinterpret_cast<uint16_t &>(*this) = data;
        }

        q16 to_position() const{
            return q16::from_i32(data_14bit << 2);
        }
    };

    static_assert(sizeof(Semantic) == 2);

    hal::SpiDrv spi_drv_;

    real_t lap_position_ = 0;
    size_t err_cnt_ = 0;
    bool fast_mode_ = false;
    Semantic last_sema_ = {0};

    IResult<uint16_t> get_position_data();
};

};