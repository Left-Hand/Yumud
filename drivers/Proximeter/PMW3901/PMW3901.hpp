#pragma once

#include "core/io/regs.hpp"
#include "drivers/Proximeter/FlowSensor.hpp"
// #include "types/image/image.hpp"

#include "hal/bus/spi/spidrv.hpp"

#include "core/utils/Result.hpp"

namespace ymd::drivers{


class PMW3901 final:public FlowSensorIntf{
public:
    enum class Error{
        Bus,
        Unspecified = 0xff,
    };
private:
    struct MotionReg:public Reg8<>{
        using Reg8::operator=;

        uint8_t frameFrom0:1;
        uint8_t runMode:2;
        const uint8_t __resv1__:1 = 0;
        uint8_t rawFrom0:1;
        uint8_t __resv2__:2;
        uint8_t occured:1;
    };

    struct DeltaReg:public Reg16<>{
        using Reg16::operator =;

        uint16_t :16;
    };


    #pragma pack(push, 1)
    struct PMW3901_Data {
    MotionReg motion = {};
    uint8_t observation = {};
    DeltaReg dx = {};
    DeltaReg dy = {};
    };static_assert(sizeof(PMW3901_Data) == 1 + 1 + 2 + 2, "PMW3901_Data size error");
    #pragma pack(pop)

    hal::SpiDrv spi_drv_;

    PMW3901_Data data_ = {};
    real_t x_cm = {};
    real_t y_cm = {};

    [[nodiscard]] Result<bool, Error> assert_reg(const uint8_t command, const uint8_t data);
    [[nodiscard]] Result<void, Error> write_reg(const uint8_t command, const uint8_t data);
    [[nodiscard]] Result<void, Error> read_reg(const uint8_t command, uint8_t & data);
    [[nodiscard]] Result<void, Error> read_burst(const uint8_t commnad, uint8_t * data, const size_t len);

    [[nodiscard]] Result<void, Error> read_data_slow();
    [[nodiscard]] Result<void, Error> read_data_burst();
    [[nodiscard]] Result<void, Error> read_data();

    [[nodiscard]] Result<void, Error> write_list(std::span<const std::pair<uint8_t, uint8_t>>);
public:
    PMW3901(const PMW3901 & other) = delete;
    PMW3901(PMW3901 && other) = delete;

    PMW3901(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    PMW3901(hal::SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    PMW3901(hal::Spi & spi, const hal::SpiSlaveIndex index):spi_drv_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] Result<bool, Error> verify();
    [[nodiscard]] Result<void, Error> init();

    [[nodiscard]] Result<void, Error> update();
    [[nodiscard]] Result<void, Error> update(const real_t rad);

    [[nodiscard]] Vector2_t<real_t> get_position(){
        return {x_cm * real_t(0.01), y_cm * real_t(0.01)};
    }

    [[nodiscard]] Result<void, Error> set_led(bool on);
    // [[nodiscard]] Result<void, Error> read_image(ImageWritable<Grayscale> & img);
};

}

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::PMW3901::Error, BusError> {
        static Result<T, drivers::PMW3901::Error> convert(const BusError berr){
            using Error = drivers::PMW3901::Error;
            using BusError = BusError;
            
            if(berr.ok()) return Ok();

            Error err = [](const BusError berr_){
                switch(berr_.type){
                    // case BusError::NO_ACK : return Error::I2C_NOT_ACK;

                    // case BusError::I2C_NOT_READY: return PMW3901::Error::I2C_NOT_READY;
                    default: return Error::Unspecified;
                }
            }(berr);

            return Err(err); 
        }
    };
}