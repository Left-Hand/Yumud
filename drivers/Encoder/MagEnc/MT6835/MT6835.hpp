#pragma once

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"
#include "core/io/regs.hpp"

namespace ymd::drivers{

struct MT6835_Prelude{
    using Error = EncoderError;

    template<typename T = void>
    using IResult = Result<T, Error>;

    using RegAddress = uint8_t;

    enum class UVWPoles:uint8_t{
        _1 = 0,
        _2, _3, _4, _5, _6, _7, _8, _9, 
        _10, _11, _12, _13, _14, _15, _16
    };

};

class MT6835_Regs:public MT6835_Prelude{
    struct UserIdReg:public Reg8<>{
        static constexpr RegAddress address = 0x001;
        uint8_t id:8;
    };

    struct AngleHReg:public Reg8<>{
        static constexpr RegAddress address = 0x003;
        uint8_t angle:8;
    };

    struct AngleMReg:public Reg8<>{
        static constexpr RegAddress address = 0x004;
        uint8_t angle:8;
    };

    struct AngleLReg:public Reg8<>{
        static constexpr RegAddress address = 0x005;
        uint8_t over_speed:1;
        uint8_t mag_weak:1;
        uint8_t under_voltage:1;
        uint8_t angle:5;
    };

    struct CrcReg:public Reg8<>{
        static constexpr RegAddress address = 0x006;
        uint8_t crc:8;
    };

    struct AbzResHReg:public Reg8<>{
        static constexpr RegAddress address = 0x007;
        uint8_t resolution:8;
    };

    struct AbzResLReg:public Reg8<>{
        static constexpr RegAddress address = 0x008;
        uint8_t ab_swap:1;
        uint8_t abz_off:1;
        uint8_t resolution:6;
    };

    struct ZeroPosHReg:public Reg8<>{
        static constexpr RegAddress address = 0x009;
        uint8_t zero_pos:8;
    };

    struct ZeroPosLReg:public Reg8<>{
        static constexpr RegAddress address = 0x00A;
        uint8_t z_pul_wid:3;
        uint8_t z_falling_on_0edge:1;
        uint8_t zero_pos:4;
    };


    struct UvwReg:public Reg8<>{
        static constexpr RegAddress address = 0x00B;
        uint8_t uvw_res:4;
        uint8_t uvw_off:1;
        uint8_t uvw_mux:1;
        uint8_t z_phase:2;
    };


    struct PwmReg:public Reg8<>{
        static constexpr RegAddress address = 0x00C;
        uint8_t pwm_sel:3;
        uint8_t pwm_pol:1;
        uint8_t pwm_fq:1;
        uint8_t nlc_en:1;
    };

    struct RoatationReg:public Reg8<>{
        static constexpr RegAddress address = 0x00D;
        uint8_t hyst:3;
        uint8_t rot_dir:1;
    };

    struct CaliReg:public Reg8<>{
        static constexpr RegAddress address = 0x00E;

        uint8_t :4;
        uint8_t autocal_freq:3;
        uint8_t gpio_ds:1;
    };

    struct BandWidthReg:public Reg8<>{
        static constexpr RegAddress address = 0x011;
        uint8_t bw:3;
        uint8_t :5;
    };

    struct NlcReg:public Reg8<>{
        static constexpr RegAddress address = 0x013;
        static constexpr RegAddress address_end = 0x0D2;

        uint8_t :8;
    };
};

class MT6835 final:
    public MagEncoderIntf,
    public MT6835_Regs{
public:
    MT6835(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    MT6835(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    MT6835(Some<hal::Spi *> spi, const hal::SpiSlaveIndex index):
        spi_drv_(hal::SpiDrv{spi, index}){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> update();
    [[nodiscard]] IResult<Angle<q31>> read_lap_angle(){
        return Ok(Angle<q31>::from_turns(lap_position_));
    }
    [[nodiscard]] uint32_t get_err_cnt() const {return errcnt_;}
private:
    hal::SpiDrv spi_drv_;

    real_t lap_position_ = 0;
    size_t errcnt_ = 0;
    bool fast_mode_ = true;

    uint16_t get_position_data();

    [[nodiscard]] IResult<> write_reg(const RegAddress addr, const uint8_t data);
    [[nodiscard]] IResult<> read_reg(const RegAddress addr, uint8_t & data);
};

};