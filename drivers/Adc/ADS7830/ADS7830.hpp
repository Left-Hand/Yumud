#pragma once

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

#include "concept/analog_channel.hpp"


namespace ymd::drivers{

struct ADS7830_Prelude{
    // 1 0 0 1 0 A1 A0
    static constexpr auto DEFAULT_I2C_ADDR = 
        hal::I2cSlaveAddr<7>::from_u7(0b1001000);

    enum class Error_Kind{
        WrongChipId,
        NoChannelComb
    };

    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;

    enum class PowerDownSel:uint8_t{
        BetweenConv = 0b00,
        RefOff_AdcOn = 0b01,
        RefOn_AdcOff = 0b10,
        RefOn_AdcOn = 0b11,
    };

    enum class ChannelIndex{
        CH0,
        CH2,
        CH1,
        CH4,

        CH3,
        CH5,
        CH6,
        CH7,
        COM,
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

        constexpr auto kind() const {return kind_;};
    private:
        Kind kind_;

        static constexpr Option<Kind> posneg2kind(
            const ChannelIndex pos,
            const ChannelIndex neg
        ){
            using enum ChannelIndex;
            constexpr auto map = std::to_array<
                    std::pair<
                        std::pair<ChannelIndex, ChannelIndex>,
                        Kind>
                    >({
                std::make_pair(std::make_pair(CH0, CH1), Kind::P0N1),
                std::make_pair(std::make_pair(CH2, CH3), Kind::P2N3),
                std::make_pair(std::make_pair(CH4, CH5), Kind::P4N5),
                std::make_pair(std::make_pair(CH6, CH7), Kind::P6N7),
                std::make_pair(std::make_pair(CH1, CH0), Kind::P1N0),
                std::make_pair(std::make_pair(CH3, CH2), Kind::P3N2),
                std::make_pair(std::make_pair(CH5, CH4), Kind::P5N4),
                std::make_pair(std::make_pair(CH7, CH6), Kind::P7N6),
                std::make_pair(std::make_pair(CH0, COM), Kind::P0NC),
                std::make_pair(std::make_pair(CH2, COM), Kind::P2NC),
                std::make_pair(std::make_pair(CH4, COM), Kind::P4NC),
                std::make_pair(std::make_pair(CH6, COM), Kind::P6NC),
                std::make_pair(std::make_pair(CH1, COM), Kind::P1NC),
                std::make_pair(std::make_pair(CH3, COM), Kind::P3NC),
                std::make_pair(std::make_pair(CH5, COM), Kind::P5NC),
                std::make_pair(std::make_pair(CH7, COM), Kind::P7NC),
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
        if(const auto res = i2c_drv_.write_reg(
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

    ADS7830(Phy && phy):
        phy_(std::move(phy)){;}

    IResult<> init();

    IResult<> validate();

    IResult<ConvData> read_channel(const ChannelIndex ch){
        const auto sel = ({
            const auto s = ChannelSelection::from_pos(ch);
            if(s.is_none()) return Err(Error::NoChannelComb);
            s.unwrap().kind();
        });

        const auto cmd = CommandByte{
            .pd = PowerDownSel::RefOn_AdcOn,
            .sel = sel 
        };

        return phy_.fs_read(cmd);
    }
private:
    Phy phy_;
};

}