#pragma once

//这个驱动已经完成
//这个驱动已经通过测试

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"



namespace ymd::drivers{

struct ADS7830_Prelude{
    // 1 0 0 1 0 A1 A0
    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0b1001000);

    static constexpr size_t CHANNEL_COUNT = 8;

    enum class Error_Kind:uint8_t{
        WrongChipId,
        NoChannelCombination
    };

    FRIEND_DERIVE_DEBUG(Error_Kind)
    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class PowerDownSel:uint8_t{
        BetweenConv = 0b00,
        RefOff_AdcOn = 0b01,
        RefOn_AdcOff = 0b10,
        RefOn_AdcOn = 0b11,
    };

    struct ChannelIndex{
        enum class Kind:uint8_t{
            CH0 = 0,
            CH1,
            CH2,
            CH3,
            CH4,
            CH5,
            CH6,
            CH7,
            COM,
        };

        using enum Kind;

        constexpr ChannelIndex(Kind kind):kind_(kind){}

        static constexpr Option<ChannelIndex> from_index(uint8_t rank){
            if(rank >= 8) return None;
            return Some(ChannelIndex(std::bit_cast<Kind>(rank)));
        }
        constexpr Kind kind() const {
            return kind_;
        }

        constexpr bool operator == (ChannelIndex const & rhs) const{
            return kind_ == rhs.kind_;
        }

        constexpr bool operator == (Kind kind) const {
            return kind_ == kind;
        }
    private:
        Kind kind_;
    };

    class ChannelSelection{
    public:
        enum class Kind:uint8_t{
            P0N1 = 0b0000,
            P2N3 = 0b0001,
            P4N5 = 0b0010,
            P6N7 = 0b0011,

            P1N0 = 0b0100,
            P3N2 = 0b0101,
            P5N4 = 0b0110,
            P7N6 = 0b0111,

            P0NC = 0b1000,
            P2NC = 0b1001,
            P4NC = 0b1010,
            P6NC = 0b1011,

            P1NC  = 0b1100,
            P3NC  = 0b1101,
            P5NC  = 0b1110,
            P7NC  = 0b1111,
        };

        constexpr ChannelSelection(const Kind kind):
            kind_(kind){;}

        static constexpr Option<ChannelSelection> 
            from_pos(
            const ChannelIndex pos){
            return from_pos_and_neg(pos, ChannelIndex::COM);
        }

        static constexpr Option<ChannelSelection> 
            from_pos_and_neg(
            const ChannelIndex pos, 
            const ChannelIndex neg
        ){
            return posneg2kind(pos, ChannelIndex::COM).
                map([](const Kind kind){return ChannelSelection(kind);});
        }

        constexpr Kind kind() const {return kind_;};
    private:
        Kind kind_;

        static constexpr Option<Kind> posneg2kind(
            const ChannelIndex pos,
            const ChannelIndex neg
        ){
            constexpr auto map = std::to_array<
                std::pair<std::pair<ChannelIndex, ChannelIndex>, Kind>>({
                std::make_pair(std::make_pair(ChannelIndex::CH0, ChannelIndex::CH1), Kind::P0N1),
                std::make_pair(std::make_pair(ChannelIndex::CH2, ChannelIndex::CH3), Kind::P2N3),
                std::make_pair(std::make_pair(ChannelIndex::CH4, ChannelIndex::CH5), Kind::P4N5),
                std::make_pair(std::make_pair(ChannelIndex::CH6, ChannelIndex::CH7), Kind::P6N7),
                std::make_pair(std::make_pair(ChannelIndex::CH1, ChannelIndex::CH0), Kind::P1N0),
                std::make_pair(std::make_pair(ChannelIndex::CH3, ChannelIndex::CH2), Kind::P3N2),
                std::make_pair(std::make_pair(ChannelIndex::CH5, ChannelIndex::CH4), Kind::P5N4),
                std::make_pair(std::make_pair(ChannelIndex::CH7, ChannelIndex::CH6), Kind::P7N6),
                std::make_pair(std::make_pair(ChannelIndex::CH0, ChannelIndex::COM), Kind::P0NC),
                std::make_pair(std::make_pair(ChannelIndex::CH2, ChannelIndex::COM), Kind::P2NC),
                std::make_pair(std::make_pair(ChannelIndex::CH4, ChannelIndex::COM), Kind::P4NC),
                std::make_pair(std::make_pair(ChannelIndex::CH6, ChannelIndex::COM), Kind::P6NC),
                std::make_pair(std::make_pair(ChannelIndex::CH1, ChannelIndex::COM), Kind::P1NC),
                std::make_pair(std::make_pair(ChannelIndex::CH3, ChannelIndex::COM), Kind::P3NC),
                std::make_pair(std::make_pair(ChannelIndex::CH5, ChannelIndex::COM), Kind::P5NC),
                std::make_pair(std::make_pair(ChannelIndex::CH7, ChannelIndex::COM), Kind::P7NC),
            });

            for(const auto & [key, value] : map){
                const auto [p, n] = key;
                if((p == pos) and (n == neg)) return Some(value);
            }
            return None;
        }
    };

    struct CommandByte{
        const uint8_t __resv__:2 = 0;
        const PowerDownSel pd:2;
        const ChannelSelection::Kind sel:4;

        constexpr uint8_t as_u8() const {
            return std::bit_cast<uint8_t>(*this);
        }
    };

    CHECK_R8(CommandByte)

    using ConvData = uint8_t;
};

class ADS7830_Phy final:
    public ADS7830_Prelude{
public:
    ADS7830_Phy(const hal::I2cDrv & i2c_drv):
        i2c_drv_(i2c_drv){;}

    ADS7830_Phy(hal::I2cDrv && i2c_drv):
        i2c_drv_(std::move(i2c_drv)){;}

    IResult<ConvData> fs_read(const CommandByte cmd){
        ConvData ret;
        if(const auto res = i2c_drv_.read_reg(
            cmd.as_u8(),
            ret 
        ); res.is_err()) return Err(res.unwrap_err());

        return Ok(ret);
    }
private:
    hal::I2cDrv i2c_drv_;
};


struct ADS7830 final:
    public ADS7830_Prelude{
public:
    using Phy = ADS7830_Phy;

    ADS7830(
        Some<hal::I2c *> i2c, 
        const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR
    ):
        phy_(hal::I2cDrv(i2c, addr)){;}

    IResult<> init();

    IResult<> validate();

    IResult<ConvData> read_pos_channel(const ChannelIndex ch);
    IResult<ConvData> read_channel(const ChannelSelection ch);

    void set_pwdn_sel(const PowerDownSel sel){
        pwdn_sel_ = sel;
    }
private:
    Phy phy_;
    PowerDownSel pwdn_sel_ = PowerDownSel::RefOn_AdcOn;
};

}