//这个驱动已经完成
//这个驱动已经测试

//PMW3901是原相科技的一款光流传感器

#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"


#include "hal/bus/spi/spidrv.hpp"
#include "algebra/vectors/vec2.hpp"


namespace ymd::drivers{

struct PMW3901_Prelude{
    struct MotionReg:public Reg8<>{

        uint8_t frame_from0:1;
        uint8_t run_mode:2;
        const uint8_t __resv1__:1 = 0;
        uint8_t raw_from0:1;
        uint8_t __resv2__:2;
        uint8_t occured:1;
    };

    struct DeltaReg:public Reg16<>{
        int16_t bits;
    };


    #pragma pack(push, 1)
    struct PMW3901_Packet {
        alignas(1) MotionReg motion = {};
        alignas(1) uint8_t observation = {};
        alignas(2) DeltaReg dx = {};
        alignas(2) DeltaReg dy = {};
    };
    #pragma pack(pop)
    static_assert(sizeof(PMW3901_Packet) == 1 + 1 + 2 + 2, "PMW3901_Packet size error");
    static_assert(__builtin_offsetof(PMW3901_Packet, PMW3901_Packet::motion) == 0);
    static_assert(__builtin_offsetof(PMW3901_Packet, PMW3901_Packet::observation) == 1);
    static_assert(__builtin_offsetof(PMW3901_Packet, PMW3901_Packet::dx) == 2);
    static_assert(__builtin_offsetof(PMW3901_Packet, PMW3901_Packet::dy) == 4);

    enum class Error_Kind:uint8_t{
        InvalidChipId
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
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

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> validate();

    [[nodiscard]] IResult<> update();

    [[nodiscard]] IResult<> set_led(bool on);
private:

    hal::SpiDrv spi_drv_;

    PMW3901_Packet packet_ = {};
    iq16 x_cm = 0;
    iq16 y_cm = 0;

    [[nodiscard]] IResult<> assert_reg(const uint8_t command, const uint8_t data, const Error & error);

    [[nodiscard]] IResult<> write_reg(const uint8_t command, const uint8_t data);
    [[nodiscard]] IResult<> read_reg(const uint8_t command, uint8_t & data);
    [[nodiscard]] IResult<> read_burst(const uint8_t commnad, std::span<uint8_t> pbuf);

    [[nodiscard]] IResult<> read_data_slow();
    [[nodiscard]] IResult<> read_data_burst();
    [[nodiscard]] IResult<> read_data();

    [[nodiscard]] IResult<> write_list(std::span<const std::pair<uint8_t, uint8_t>>);
};

}
