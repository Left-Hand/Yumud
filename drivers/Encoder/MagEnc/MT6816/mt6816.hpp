#pragma once

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

public:
    MT6816(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}

    MT6816(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}

    MT6816(hal::Spi & spi, const hal::SpiSlaveIndex index):
        spi_drv_(hal::SpiDrv{spi, index}){;}

    IResult<> init();
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
    struct Semantic:public Reg16<>{
        using Reg16::operator=;
        #pragma pack(push, 1)
        uint16_t pc:1;
        uint16_t no_mag:1;
        uint16_t data_14bit:14;
        #pragma pack(pop)
    };

    static_assert(sizeof(Semantic) == 2);

    static constexpr size_t MAX_INIT_RETRY_TIMES = 32;

    hal::SpiDrv spi_drv_;

    real_t lap_position_ = 0;
    size_t err_cnt_ = 0;
    bool fast_mode_ = true;
    Semantic last_sema_ = {};

    Result<uint16_t, hal::HalResult> get_position_data();
};

};