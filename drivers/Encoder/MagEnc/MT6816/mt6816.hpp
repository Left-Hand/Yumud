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
    explicit MT6816(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}

    explicit MT6816(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}

    explicit MT6816(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv{spi, rank}){;}

    IResult<> init(const Config & cfg);
    IResult<> reconf(const Config & cfg);
    IResult<> update();

    IResult<Angular<uq32>> read_lap_angle() { 
        return last_packet_.parse();
    }

    IResult<MagStatus> get_mag_status() {
        if(last_packet_.no_mag){
            return Ok(MagStatus::from_low());
        }else{
            return Ok(MagStatus::from_proper());
        }
    }

private:
    struct Packet{
        uint16_t pc:1;
        uint16_t no_mag:1;
        uint16_t data_14bit:14;

        [[nodiscard]] IResult<Angular<uq32>> parse() const {
            if(not is_pc_valid()) [[unlikely]]
                return Err(EncoderError::InvalidPc);
            
            if(no_mag) [[unlikely]]
                return Err(EncoderError::MagnetLost);

            const auto turns = static_cast<uq32>(uq14::from_bits(data_14bit));
            return Ok(Angular<uq32>::from_turns(turns));
        }

        [[nodiscard]] bool is_pc_valid() const {
            //TODO;
            return true;
        }
    private:
        
    };

    static_assert(sizeof(Packet) == 2);

    hal::SpiDrv spi_drv_;

    Packet last_packet_ = {};

    IResult<Packet> get_packet();
};

};