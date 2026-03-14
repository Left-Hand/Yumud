#pragma once

#include "primitive/can/bxcan_frame.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/PerUnit.hpp"

#include "core/math/real.hpp"


namespace ymd::robots::cybergear{

struct [[nodiscard]] StatusBitFields final{
    using Self = StatusBitFields;
    uint8_t can_id;
    uint8_t under_voltage:1;
    uint8_t over_current:1;
    uint8_t over_temp:1;
    uint8_t mag_enc_err:1;

    uint8_t hall_enc_err:1;
    uint8_t uncalibrated:1;
    uint8_t mode:2;

    [[nodiscard]] constexpr bool is_running() const {return mode == 2;}
    [[nodiscard]] constexpr bool is_reset() const {return mode == 0;}
    [[nodiscard]] constexpr bool is_calibrating() const {return mode == 1;}
};
static_assert(sizeof(StatusBitFields) == 2);


enum class [[nodiscard]] Error:uint8_t{
    PRAGRAM_UNHANDLED,
    PRAGRAM_TODO,
    RET_DLC_SHORTER,
    RET_DLC_LONGER,
    RET_UNKOWN_CMD,
    INPUT_OUT_OF_RANGE,
    INPUT_LOWER_THAN_LIMIT,
    INPUT_HIGHER_THAN_LIMIT,
    MOTOR_NOT_INITED,
};

enum class [[nodiscard]] Command:uint8_t{
    GET_DEVICE_ID = 0,
    SEND_CTRL1 = 1,
    FBK_CTRL1 = 2,
    EN_MOT = 3,
    DISEN_MOT = 4,
    SET_MACHINE_HOME = 6,
    SET_CAN_ID = 7,
    READ_PARA = 17,
    WRITE_PARA = 18,
    FBK_FAULT = 21,
};


template<typename T = void>
using IResult = Result<T, Error>;

namespace details{

struct [[nodiscard]] TemperatureCode final{
    uint16_t bits;

    constexpr iq16 to_celsius() const {
        return iq16(bits) / 10;
    }
};

DEF_PER_UNIT(RadCode, uint16_t, -2 * (2 * M_PI), 2 * (2 * M_PI))
DEF_PER_UNIT(OmegaCode, uint16_t, -30 * (2 * M_PI), 30 * (2 * M_PI))
DEF_PER_UNIT(TorqueCode, uint16_t, -12, 12)
DEF_PER_UNIT(KpCode, uint16_t, 0, 500)
DEF_PER_UNIT(KdCode, uint16_t, 0, 5)


struct [[nodiscard]] Feedback final{
    iq16 radians;
    iq16 omega;
    iq16 torque;
    iq16 celsius;
};
}


#if 0
class CyberGear{
public:


    // using Error = details::Error;
    // using IResult = details::IResult;

    using CanFrame = hal::BxCanFrame; 

    using Feedback = details::Feedback;
    using TemperatureCode = details::TemperatureCode;
    using RadCode = details::RadCode;
    using OmegaCode = details::OmegaCode;
    using TorqueCode = details::TorqueCode;
    using KpCode = details::KpCode;
    using KdCode = details::KdCode;


    explicit CyberGear(hal::Can & can, uint8_t host_id, uint8_t node_id):
        can_(can), host_id_(host_id), node_id_(node_id){;}

    IResult<> init();

    IResult<> transmit(const CanFrame & frame);

    IResult<> transmit(const uint32_t id, const uint64_t payload, const uint8_t dlc);

    IResult<> request_mcu_id();

    struct MitParams{
        iq16 torque;
        iq16 radians;
        iq16 omega; 
        iq16 kp; 
        iq16 kd;
    };

    IResult<> ctrl(const MitParams & params);

    
    IResult<> on_receive(const CanFrame & frame);

    IResult<> enable(const Enable en, const bool clear_fault = true);

    IResult<> set_current_as_machine_home();

    IResult<> change_node_id(const uint8_t id);

    IResult<> request_read_para(const uint16_t idx);

    IResult<> request_write_para(const uint16_t idx, const uint32_t data);

    [[nodiscard]] Option<uint64_t> get_device_mcu_id() const {return device_mcu_id_;}

private:
    hal::Can & can_;
    const uint8_t host_id_;
    uint8_t node_id_;
    
    StatusBitFields fault_ = {};
    Option<uint64_t> device_mcu_id_ = None;


    Feedback feedback_ = {};

    IResult<> on_mcu_id_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
    IResult<> on_ctrl2_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
    IResult<> on_read_para_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
};

#endif
}