#pragma once

#include "hal/bus/can/can.hpp"
#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"
#include "primitive/PerUnit.hpp"

#include "core/math/real.hpp"


namespace ymd::robots::cybergear{

struct [[nodiscard]] Fault{
    using Self = Fault;
    uint8_t can_id;
    uint8_t under_voltage:1;
    uint8_t over_current:1;
    uint8_t over_temp:1;
    uint8_t mag_enc_err:1;
    uint8_t hall_enc_err:1;
    uint8_t uncalibrated:1;
    uint8_t mode:2;

    void update(const uint16_t bits){*this = std::bit_cast<Self>(bits);}
    [[nodiscard]] constexpr bool is_running() const {return mode == 2;}
    [[nodiscard]] constexpr bool is_reset() const {return mode == 0;}
    [[nodiscard]] constexpr bool is_calibrating() const {return mode == 1;}
};
static_assert(sizeof(Fault) == 2);


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

struct Temperature{
    uint16_t data;

    constexpr explicit operator real_t() const {
        return real_t(data) / 10;
    }
};

DEF_PER_UNIT(CmdRad, uint16_t, -2 * TAU, 2 * TAU)
DEF_PER_UNIT(CmdOmega, uint16_t, -30 * TAU, 30 * TAU)
DEF_PER_UNIT(CmdTorque, uint16_t, -12, 12)
DEF_PER_UNIT(CmdKp, uint16_t, 0, 500)
DEF_PER_UNIT(CmdKd, uint16_t, 0, 5)


struct Feedback{
    real_t radians = {};
    real_t omega = {};
    real_t torque = {};
    real_t temperature = {};
};
}

class CyberGear{
public:


    // using Error = details::Error;
    // using IResult = details::IResult;

    using CanFrame = hal::BxCanFrame; 

    using Feedback = details::Feedback;
    using Temperature = details::Temperature;
    using CmdRad = details::CmdRad;
    using CmdOmega = details::CmdOmega;
    using CmdTorque = details::CmdTorque;
    using CmdKp = details::CmdKp;
    using CmdKd = details::CmdKd;


    CyberGear(hal::Can & can, uint8_t host_id, uint8_t node_id):
        can_(can), host_id_(host_id), node_id_(node_id){;}

    [[nodiscard]] IResult<> init();

    [[nodiscard]] IResult<> transmit(const CanFrame & frame);

    [[nodiscard]] IResult<> transmit(const uint32_t id, const uint64_t payload, const uint8_t dlc);

    [[nodiscard]] IResult<> request_mcu_id();

    struct MitParams{
        real_t torque;
        real_t radians;
        real_t omega; 
        real_t kp; 
        real_t kd;
    };

    [[nodiscard]] IResult<> ctrl(const MitParams & params);

    
    [[nodiscard]] IResult<> on_receive(const CanFrame & frame);

    [[nodiscard]] IResult<> enable(const Enable en, const bool clear_fault = true);

    [[nodiscard]] IResult<> set_current_as_machine_home();

    [[nodiscard]] IResult<> change_node_id(const uint8_t id);

    [[nodiscard]] IResult<> request_read_para(const uint16_t idx);

    [[nodiscard]] IResult<> request_write_para(const uint16_t idx, const uint32_t data);

    [[nodiscard]] Option<uint64_t> get_device_mcu_id() const {return device_mcu_id_;}

private:
    hal::Can & can_;
    const uint8_t host_id_;
    uint8_t node_id_;
    
    Fault fault_ = {};
    Option<uint64_t> device_mcu_id_ = None;


    Feedback feedback_ = {};

    [[nodiscard]] IResult<> on_mcu_id_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
    [[nodiscard]] IResult<> on_ctrl2_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
    [[nodiscard]] IResult<> on_read_para_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
};

}