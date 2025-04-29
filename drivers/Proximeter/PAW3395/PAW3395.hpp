//这个驱动还未完成
//这个驱动还未测试

//PAW3395 是原相科技的一款游戏级顶级鼠标光流传感器

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"


#include "hal/bus/spi/spidrv.hpp"

#include "drivers/Proximeter/FlowSensor.hpp"



namespace ymd::drivers{


class PAW3395 final:public FlowSensorIntf{
public:
    enum class Error_Kind:uint8_t{
            
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    hal::SpiDrv spi_drv_;

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
    PAW3395(const PAW3395 & other) = delete;
    PAW3395(PAW3395 && other) = delete;

    PAW3395(const hal::SpiDrv & spi_drv):spi_drv_(spi_drv){;}
    PAW3395(hal::SpiDrv && spi_drv):spi_drv_(std::move(spi_drv)){;}
    PAW3395(hal::Spi & spi, const hal::SpiSlaveIndex index):spi_drv_(hal::SpiDrv(spi, index)){;}

    [[nodiscard]] Result<bool, Error> verify();
    [[nodiscard]] Result<void, Error> init();

    [[nodiscard]] Result<void, Error> update();
    [[nodiscard]] Result<void, Error> update(const real_t rad);

    [[nodiscard]] Vector2_t<real_t> get_position(){
        return {x_cm * real_t(0.01), y_cm * real_t(0.01)};
    }

    [[nodiscard]] Result<void, Error> set_led(bool on);
};

}

namespace ymd::custom{
    template<typename T>
    struct result_converter<T, drivers::PAW3395::Error, hal::HalResult> {
        static Result<T, drivers::PAW3395::Error> convert(const hal::HalResult res){
            if(res.is_ok()) return Ok();
            return Err(drivers::PAW3395::Error(res.unwrap_err())); 
        }
    };
}