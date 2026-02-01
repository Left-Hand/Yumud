#pragma once


#include "core/io/regs.hpp"
#include "hal/bus/spi/spidrv.hpp"
#include "algebra/vectors/vec2.hpp"

#include "core/utils/result.hpp"
#include "core/utils/Errno.hpp"

// https://blog.csdn.net/weixin_44793491/article/details/105781595

namespace ymd::drivers{
struct Ps2Joystick_Prelude{
enum class Error_Kind:uint8_t{
    CantParseAtDigitMode,
    Unreachable
};

DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)
DEF_FRIEND_DERIVE_DEBUG(Error_Kind)

template<typename T = void>
using IResult = Result<T, Error>;

enum class [[nodiscard]] JoyStickChannelSelection:uint8_t{
    Select,
    L3,
    R3,
    Start,
    Up,
    Right,
    Down,
    Left,
    L2,
    R2,
    L1,
    R1,
    Delta,
    Circle,
    Cross,
    Square,
    RX,
    RY,
    LX,
    LY
};


enum class [[nodiscard]] DevId:uint8_t{
    Uninit = 0,
    None = 0x01,
    Digit = 0x41,
    NegCon = 0x23,
    AnalogRed = 0x73,
    AnalogGreen = 0x53
};

friend OutputStream & operator<<(OutputStream & os, const DevId dev_id){
    const auto pstr = [=] -> const char * {
        switch(dev_id){
            case DevId::Uninit:
                return "Uninit";
            case DevId::None:
                return "None";
            case DevId::Digit:
                return "Digit";
            case DevId::NegCon:
                return "NegCon";
            case DevId::AnalogRed:
                return "AnalogRed";
            case DevId::AnalogGreen:
                return "AnalogGreen";
            default:
                return nullptr;
        }
    }();

    if(pstr == nullptr){
        return os << os.field("Unknown")(static_cast<uint8_t>(dev_id));
    }
    return os << pstr;
}

enum class [[nodiscard]] PressLevel:uint8_t{
    Pressed = 0,
    UnPress = 1
};

struct [[nodiscard]] Modifiers final{
    PressLevel select:1;
    PressLevel l3:1;
    PressLevel r3:1;
    PressLevel start:1;
    PressLevel up:1;
    PressLevel right:1;
    PressLevel down:1;
    PressLevel left:1;


    PressLevel l2:1;
    PressLevel r2:1;
    PressLevel l1:1;
    PressLevel r1:1;
    PressLevel delta:1;
    PressLevel circ:1;
    PressLevel cross:1;
    PressLevel squ:1;

    [[nodiscard]] constexpr math::Vec2i left_direction() const{
        auto & self =  *this;
        return math::Vec2i(
            int(PressLevel::Pressed == self.right) - int(PressLevel::Pressed == self.left),
            int(PressLevel::Pressed == self.up) - int(PressLevel::Pressed == self.down)
        );
    }

    [[nodiscard]] constexpr std::bitset<16> to_bitset() const {
        return std::bitset<16>(std::bit_cast<uint16_t>(*this));
    }
};

static_assert(sizeof(Modifiers) == 2);
struct alignas(2) [[nodiscard]] JoystickPosition final{
    uint8_t x;
    uint8_t y;

    constexpr math::Vec2<real_t> to_vec2() const{
        constexpr auto SCALE = real_t(1.0/510);
        return {SCALE * (int(x * 4)) - 1, SCALE * (-int(y * 4)) + 1};
    }
};

struct [[nodiscard]] RxPacket{
    #pragma pack(push, 1)
    alignas(2) Modifiers modifiers;

    alignas(2) JoystickPosition right_joystick;
    alignas(2) JoystickPosition left_joystick;

    #pragma pack(pop)

    [[nodiscard]] constexpr uint8_t query_channel(const JoyStickChannelSelection sel) const {
        switch(sel){
            case JoyStickChannelSelection::Select:
                return (PressLevel::Pressed == modifiers.select) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::L3:
                return (PressLevel::Pressed == modifiers.l3) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::R3:
                return (PressLevel::Pressed == modifiers.r3) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::Start:
                return (PressLevel::Pressed == modifiers.start) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::Up:
                return (PressLevel::Pressed == modifiers.up) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::Right:
                return (PressLevel::Pressed == modifiers.right) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::Down:
                return (PressLevel::Pressed == modifiers.down) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::Left:
                return (PressLevel::Pressed == modifiers.left) ? uint8_t(255) : uint8_t(0);


            case JoyStickChannelSelection::L2:
                return (PressLevel::Pressed == modifiers.l2) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::R2:
                return (PressLevel::Pressed == modifiers.r2) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::L1:
                return (PressLevel::Pressed == modifiers.l1) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::R1:
                return (PressLevel::Pressed == modifiers.r1) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::Delta:
                return (PressLevel::Pressed == modifiers.delta) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::Circle:
                return (PressLevel::Pressed == modifiers.circ) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::Cross:
                return (PressLevel::Pressed == modifiers.cross) ? uint8_t(255) : uint8_t(0);
            case JoyStickChannelSelection::Square:
                return (PressLevel::Pressed == modifiers.squ) ? uint8_t(255) : uint8_t(0);


            case JoyStickChannelSelection::RX:
                return right_joystick.x;
            case JoyStickChannelSelection::RY:
                return right_joystick.y;
            case JoyStickChannelSelection::LX:
                return left_joystick.x;
            case JoyStickChannelSelection::LY:
                return left_joystick.y;
        }
        //unreachable
        __builtin_trap();
    }

    [[nodiscard]] imconstexpr std::span<uint8_t, 6> as_bytes_mut() {
        return std::span<uint8_t, 6>{reinterpret_cast<uint8_t *>(this), sizeof(*this)};
    }

};

static constexpr size_t PACKET_SIZE = 6;

static_assert(sizeof(RxPacket) == PACKET_SIZE);

struct TxPacket{
    uint8_t right_vibration_strength;
    uint8_t left_vibration_strength;
    uint8_t __padding__[4];

    static constexpr TxPacket from_parts(const uint8_t left, const uint8_t right){
        return {left, right, {}};
    }

    std::span<const uint8_t, 6> as_bytes() const {
        return std::span<const uint8_t, 6>{
            reinterpret_cast<const uint8_t*>(this), sizeof(*this)
        };
    }
};

static_assert(sizeof(TxPacket) == PACKET_SIZE);


};


class Ps2Joystick final:public Ps2Joystick_Prelude{
public:
    explicit Ps2Joystick(const hal::SpiDrv & spi_drv):
        spi_drv_(spi_drv){;}
    explicit Ps2Joystick(hal::SpiDrv && spi_drv):
        spi_drv_(std::move(spi_drv)){;}

    IResult<> init();
    IResult<> update();
    IResult<DevId> devid() const {
        return Ok(dev_id_);
    }

    IResult<RxPacket> read_info() const;
private:
    hal::SpiDrv spi_drv_;
    DevId dev_id_ = DevId::None;
    RxPacket rx_packet_;
};

}
