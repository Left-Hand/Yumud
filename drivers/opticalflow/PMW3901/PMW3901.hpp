//这个驱动已经完成
//这个驱动已经测试

//PMW3901是原相科技的一款光流传感器

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"


#include "hal/conn/spi/spidrv.hpp"
#include "middlewares/algebra/vectors/vec2.hpp"


namespace ymd::drivers{

struct PMW3901_Prelude{
    struct [[nodiscard]] MotionCode final{
        uint8_t frame_from0:1;
        uint8_t run_mode:2;
        uint8_t __resv1__:1;
        uint8_t raw_from0:1;
        uint8_t __resv2__:2;
        uint8_t occured:1;
    };

    #pragma pack(push, 1)
    struct [[nodiscard]] Packet final{
        alignas(1) MotionCode motion;
        alignas(1) uint8_t observation;
        alignas(2) int16_t dx;
        alignas(2) int16_t dy;
    };
    #pragma pack(pop)
    static_assert(sizeof(Packet) == 1 + 1 + 2 + 2, "Packet size error");
    static_assert(__builtin_offsetof(Packet, Packet::motion) == 0);
    static_assert(__builtin_offsetof(Packet, Packet::observation) == 1);
    static_assert(__builtin_offsetof(Packet, Packet::dx) == 2);
    static_assert(__builtin_offsetof(Packet, Packet::dy) == 4);

    enum class Error_Kind:uint8_t{
        InvalidChipId
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class [[nodiscard]] RegAddr:uint8_t{
        Product_ID              = 0x00,
        Revision_ID             = 0x01,
        Motion                  = 0x02,
        Delta_X_L               = 0x03,
        Delta_X_H               = 0x04,
        Delta_Y_L               = 0x05,
        Delta_Y_H               = 0x06,
        Squal                   = 0x07,
        RawData_Sum             = 0x08,
        RawData_Max             = 0x09,
        RawData_Min_            = 0x0A,
        Shutter_Lower           = 0x0B,
        Shutter_Upper           = 0x0C,
        Observation             = 0x15,
        Motion_Burst            = 0x16,
        Power_Up_Reset          = 0x3A,
        Shutdown                = 0x3B,
        RawData_Grab            = 0x58,
        RawData_Grab_Status     = 0x59,
        Inverse_Product_ID      = 0x5F,
    };
};

class PMW3901 final:public PMW3901_Prelude{
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

    IResult<> init();

    IResult<> validate();

    IResult<Packet> update();

    IResult<> set_led(bool on);
private:

    hal::SpiDrv spi_drv_;

    IResult<> assert_reg(const uint8_t command, const uint8_t data, const Error & error);

    IResult<> write_reg(const uint8_t command, const uint8_t data);
    IResult<> read_reg(const uint8_t command, uint8_t & data);
    IResult<> read_bulk(const uint8_t commnad, std::span<uint8_t> pbuf);

    IResult<> read_data_slow();
    IResult<> read_data_burst();
    IResult<> read_data();

    IResult<> write_list(std::span<const std::pair<uint8_t, uint8_t>>);
};

}
