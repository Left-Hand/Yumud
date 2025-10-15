#pragma once


#include "core/io/regs.hpp"
#include "hal/bus/spi/spidrv.hpp"
#include "types/vectors/vector2.hpp"

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

    enum class JoyStickChannel:uint8_t{
        SELECT,
        L3,
        R3,
        START,
        UP,
        RIGHT,
        DOWN,
        LEFT,
        L2, 
        R2, 
        L1, 
        R1, 
        DELTA, 
        CIRC, 
        CROSS, 
        SQU,
        RX, 
        RY, 
        LX, 
        LY
    };


    enum class DevId:uint8_t{
        UNINIT = 0,
        NONE = 0x01,
        DIGIT = 0x41,
        NEGCON = 0x23,
        ANARED = 0x73,
        ANAGREEN = 0x53
    };

    friend OutputStream & operator<<(OutputStream & os, const DevId dev_id){
        const auto pstr = [=] -> const char * {
            switch(dev_id){
                case DevId::UNINIT:
                    return "UNINIT";
                case DevId::NONE:
                    return "NONE";
                case DevId::DIGIT:
                    return "DIGIT";
                case DevId::NEGCON:
                    return "NEGCON";
                case DevId::ANARED:
                    return "ANARED";
                case DevId::ANAGREEN:
                    return "ANAGREEN";
                default:
                    return nullptr;
            }
        }();

        if(pstr == nullptr){
            return os << "UNKNOWN(" << static_cast<uint8_t>(dev_id) << ")";
        }
        return os << pstr;
    }

    enum class PressLevel:uint8_t{
        Pressed = 0,
        UnPress = 1
    };

    struct RxPayload{
        #pragma pack(push, 1)

        struct Modifiers{
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

            constexpr std::bitset<16> to_bitset() const {
                return std::bitset<16>(std::bit_cast<uint16_t>(*this));
            }
        };

        static_assert(sizeof(Modifiers) == 2);

        Modifiers modifiers;

        // uint8_t rx;
        // uint8_t ry;
        // uint8_t lx;
        // uint8_t ly;

        struct JoystickPosition{
            uint8_t x;
            uint8_t y;

            constexpr Vec2<real_t> to_position() const{
                constexpr auto SCALE = real_t(1.0/510);
                return {SCALE * (int(x * 4)) - 1, SCALE * (-int(y * 4)) + 1};
                // return {x,y};
            }
        };

        JoystickPosition right_joystick;
        JoystickPosition left_joystick;

        #pragma pack(pop)

        constexpr uint8_t query_channel(const JoyStickChannel event) const {
            switch(event){
                case JoyStickChannel::SELECT:
                    return bool(PressLevel::Pressed == modifiers.select);
                case JoyStickChannel::L3:
                    return bool(PressLevel::Pressed == modifiers.l3);
                case JoyStickChannel::R3:
                    return bool(PressLevel::Pressed == modifiers.r3);
                case JoyStickChannel::START:
                    return bool(PressLevel::Pressed == modifiers.start);
                case JoyStickChannel::UP:
                    return bool(PressLevel::Pressed == modifiers.up);
                case JoyStickChannel::RIGHT:
                    return bool(PressLevel::Pressed == modifiers.right);
                case JoyStickChannel::DOWN:
                    return bool(PressLevel::Pressed == modifiers.down);
                case JoyStickChannel::LEFT:
                    return bool(PressLevel::Pressed == modifiers.left);


                case JoyStickChannel::L2:
                    return bool(PressLevel::Pressed == modifiers.l2);
                case JoyStickChannel::R2:
                    return bool(PressLevel::Pressed == modifiers.r2);
                case JoyStickChannel::L1:
                    return bool(PressLevel::Pressed == modifiers.l1);
                case JoyStickChannel::R1:
                    return bool(PressLevel::Pressed == modifiers.r1);
                case JoyStickChannel::DELTA:
                    return bool(PressLevel::Pressed == modifiers.delta);
                case JoyStickChannel::CIRC:
                    return bool(PressLevel::Pressed == modifiers.circ);
                case JoyStickChannel::CROSS:
                    return bool(PressLevel::Pressed == modifiers.cross);
                case JoyStickChannel::SQU:
                    return bool(PressLevel::Pressed == modifiers.squ);


                case JoyStickChannel::RX:
                    return right_joystick.x;
                case JoyStickChannel::RY:
                    return right_joystick.y;
                case JoyStickChannel::LX:
                    return left_joystick.x;
                case JoyStickChannel::LY:
                    return left_joystick.y;

                default:
                    __builtin_unreachable();
            }
        }

        constexpr Vec2i left_direction() const{
            Vec2i dir = Vec2i(0, 0);

            if(PressLevel::Pressed == modifiers.left) 
                dir += Vec2i::LEFT;
            if(PressLevel::Pressed == modifiers.right) 
                dir += Vec2i::RIGHT;
            if(PressLevel::Pressed == modifiers.up) 
                dir += Vec2i::UP;
            if(PressLevel::Pressed == modifiers.down) 
                dir += Vec2i::DOWN;

            return dir;
        }

        std::span<uint8_t> as_bytes() {
            return std::span<uint8_t>(reinterpret_cast<uint8_t *>(this), sizeof(*this));
        }

    };



    static constexpr size_t FRAME_SIZE = sizeof(RxPayload);
    static_assert(FRAME_SIZE == 6);
};

struct Ps2Joystick_Phy:public Ps2Joystick_Prelude{
    #if 0

    #endif
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

    IResult<RxPayload> read_info() const;
private:
    hal::SpiDrv spi_drv_;
    DevId dev_id_ = DevId::NONE;
    RxPayload rx_payload_;
};

}