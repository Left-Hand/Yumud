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

    explicit MT6816(Some<hal::Spi *> spi, const hal::SpiSlaveRank index):
        spi_drv_(hal::SpiDrv{spi, index}){;}

    IResult<> init(const Config & cfg);
    IResult<> reconf(const Config & cfg);
    IResult<> update();

    IResult<Angle<q31>> read_lap_angle() { 
        return Ok(Angle<q31>::from_turns(lap_position_));
    }

    uint32_t get_err_cnt() const {return err_cnt_;}

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

        // constexpr Packet() : pc(0), no_mag(0), data_14bit(0) {}

        Packet(uint16_t data){
            reinterpret_cast<uint16_t &>(*this) = data;
        }

        q16 to_position() const{
            return q16::from_i32(data_14bit << 2);
        }
    };

    static_assert(sizeof(Packet) == 2);

    hal::SpiDrv spi_drv_;

    real_t lap_position_ = 0;
    size_t err_cnt_ = 0;
    bool fast_mode_ = false;
    Packet last_packet_ = {0};

    IResult<uint16_t> get_position_data();
};

};