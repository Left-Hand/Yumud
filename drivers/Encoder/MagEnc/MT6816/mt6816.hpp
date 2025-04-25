#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Encoder/MagEncoder.hpp"

namespace ymd::drivers{

class MT6816:public MagEncoderIntf{
protected:
    class Error{
        enum Kind:uint8_t{
            BusError,
            Unspecified,
        };

        constexpr Error(const Kind kind):
            kind_(kind){;}

        constexpr Error(const hal::BusError):
            kind_(Kind::BusError){;}
        constexpr bool operator == (const Error other) const {
            return kind_ == other.kind_;
        }

        constexpr bool operator == (const Kind kind) const {
            return kind_ == kind;
        }
    private:
        Kind kind_;
    };

    struct Semantic:public Reg16<>{
        using Reg16::operator=;
        uint16_t pc:1;
        uint16_t no_mag:1;
        uint16_t data_14bit:14;
    };

    hal::SpiDrv spi_drv_;

    real_t lap_position_;
    size_t err_cnt_ = 0;
    bool fast_mode_ = true;
    Semantic last_semantic_;

    uint16_t get_position_data();
public:
    MT6816(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    MT6816(hal::SpiDrv && spi_drv):spi_drv_(spi_drv){;}
    MT6816(hal::Spi & _bus, const hal::SpiSlaveIndex index):spi_drv_(hal::SpiDrv{_bus, index}){;}

    void init() override;
    void update() override;
    Option<real_t> get_lap_position() override { return Some(lap_position_);}
    uint32_t get_err_cnt() const {return err_cnt_;}

    bool stable() override {return last_semantic_.no_mag == false;}
};

};