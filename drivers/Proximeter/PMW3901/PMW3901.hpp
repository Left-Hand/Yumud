//这个驱动已经完成
//这个驱动已经测试

//PMW3901是原相科技的一款光流传感器

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"


#include "hal/bus/spi/spidrv.hpp"
#include "types/vectors/vector2.hpp"


namespace ymd::drivers{

struct PMW3901_Prelude{
    struct MotionReg:public Reg8<>{
        using Reg8::operator=;

        uint8_t frame_from0:1;
        uint8_t run_mode:2;
        const uint8_t __resv1__:1 = 0;
        uint8_t raw_from0:1;
        uint8_t __resv2__:2;
        uint8_t occured:1;
    };

    struct DeltaReg:public Reg16i<>{
        int16_t data;
    };


    #pragma pack(push, 1)
    struct PMW3901_Data {
    MotionReg motion = {};
    uint8_t observation = {};
    DeltaReg dx = {};
    DeltaReg dy = {};
    };
    #pragma pack(pop)
    static_assert(sizeof(PMW3901_Data) == 1 + 1 + 2 + 2, "PMW3901_Data size error");

};

class PMW3901 final:public PMW3901_Prelude{
public:
    enum class Error_Kind:uint8_t{
        WrongChipId
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
public:

    explicit PMW3901(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit PMW3901(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}
    explicit PMW3901(Some<hal::Spi *> spi, const hal::SpiSlaveRank rank):
        spi_drv_(hal::SpiDrv(spi, rank)){;}

    PMW3901(const PMW3901 & other) = delete;
    PMW3901(PMW3901 && other) = delete;

    ~PMW3901() = default;
    [[nodiscard]] IResult<> validate();
    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> update();

    [[nodiscard]] Vec2<q16> get_position(){
        return {x_cm * q16(0.01), y_cm * q16(0.01)};
    }

    [[nodiscard]] IResult<> set_led(bool on);
private:

    hal::SpiDrv spi_drv_;

    PMW3901_Data data_ = {};
    q16 x_cm = 0;
    q16 y_cm = 0;

    [[nodiscard]] IResult<bool> assert_reg(const uint8_t command, const uint8_t data);
    [[nodiscard]] IResult<> write_reg(const uint8_t command, const uint8_t data);
    [[nodiscard]] IResult<> read_reg(const uint8_t command, uint8_t & data);
    [[nodiscard]] IResult<> read_burst(const uint8_t commnad, std::span<uint8_t> pbuf);

    [[nodiscard]] IResult<> read_data_slow();
    [[nodiscard]] IResult<> read_data_burst();
    [[nodiscard]] IResult<> read_data();

    [[nodiscard]] IResult<> write_list(std::span<const std::pair<uint8_t, uint8_t>>);
};

}
