#pragma once

//不推荐用于新设计 性价比不高 

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/conn/spi/spidrv.hpp"

#include "drivers/encoder/encoder.hpp"

namespace ymd::drivers{


struct MT6816_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    struct Config{
    };
};

class MT6816 final:public MT6816_Prelude{
public:


public:
    explicit MT6816(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}

    explicit MT6816(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}

    explicit MT6816(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv{spi, rank}){;}

    IResult<> init(const Config & cfg);


private:
    struct [[nodiscard]] Packet final{
        uint16_t pc:1;
        uint16_t no_mag:1;
        uint16_t data_14bit:14;

        Angular<uq32> angle() const noexcept {
            const auto turns = static_cast<uq32>(uq14::from_bits(data_14bit));
            return Angular<uq32>::from_turns(turns);
        }

        EncoderFaultBitFields fault() {
            EncoderFaultBitFields fault_flags = EncoderFaultBitFields::zero();
            if(this->no_mag){
                fault_flags.mag_strength = EncoderFaultBitFields::MagStrength::Lost;
            }else{
                fault_flags.mag_strength = EncoderFaultBitFields::MagStrength::Proper;
            }
            return fault_flags;
        }

        [[nodiscard]] bool is_pc_valid() const noexcept {
            //TODO;
            return true;
        }
    private:
        
    };

    static_assert(sizeof(Packet) == 2);

    hal::SpiDrv spi_drv_;

    IResult<Packet> get_packet();
};

};