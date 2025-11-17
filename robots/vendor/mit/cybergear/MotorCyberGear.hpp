#pragma once


#include "hal/bus/can/candrv.hpp"

#include "core/utils/Option.hpp"
#include "core/utils/Result.hpp"
#include "primitive/PerUnit.hpp"

#include "core/math/real.hpp"


namespace ymd::robots{

struct CyberGear_Fault{

    union{
        uint16_t raw;
        struct{
            uint16_t can_id:8;
            uint16_t under_voltage:1;
            uint16_t over_current:1;
            uint16_t over_temp:1;
            uint16_t mag_enc_err:1;
            uint16_t hall_enc_err:1;
            uint16_t uncalibrated:1;
            uint16_t mode:2;
        };
    };

    void update(const uint16_t data){raw = data;}
    bool is_running(){return mode == 2;}
    bool is_reset(){return mode == 0;}
    bool is_calibrating(){return mode == 1;}
};static_assert(sizeof(CyberGear_Fault) == 2);


enum class CyberGear_Error:uint8_t{
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

enum class CyberGear_Command:uint8_t{
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
using CyberGear_Result = Result<T, CyberGear_Error>;

namespace details{

    struct CyberGear_Temperature{
        uint16_t data;

        constexpr explicit operator real_t() const {
            return real_t(data) / 10;
        }
    };

    DEF_PER_UNIT(CyberGear_CmdRad, uint16_t, -2 * TAU, 2 * TAU)
    DEF_PER_UNIT(CyberGear_CmdOmega, uint16_t, -30 * TAU, 30 * TAU)
    DEF_PER_UNIT(CyberGear_CmdTorque, uint16_t, -12, 12)
    DEF_PER_UNIT(CyberGear_CmdKp, uint16_t, 0, 500)
    DEF_PER_UNIT(CyberGear_CmdKd, uint16_t, 0, 5)
    

    struct CyberGear_Feedback{
        real_t rad = {};
        real_t omega = {};
        real_t torque = {};
        real_t temperature = {};
    };
}



class CyberGear{
public:


    // using CyberGear_Error = details::CyberGear_Error;
    // using CyberGear_Result = details::CyberGear_Result;

    using CanMsg = hal::CanMsg; 

    using Feedback = details::CyberGear_Feedback;
    using Temperature = details::CyberGear_Temperature;
    using CmdRad = details::CyberGear_CmdRad;
    using CmdOmega = details::CyberGear_CmdOmega;
    using CmdTorque = details::CyberGear_CmdTorque;
    using CmdKp = details::CyberGear_CmdKp;
    using CmdKd = details::CyberGear_CmdKd;


    CyberGear(hal::Can & can, uint8_t host_id, uint8_t node_id):
        can_drv_(can), host_id_(host_id), node_id_(node_id){;}

    [[nodiscard]] CyberGear_Result<> init();

    [[nodiscard]] CyberGear_Result<> transmit(const CanMsg & msg);

    [[nodiscard]] CyberGear_Result<> transmit(const uint32_t id, const uint64_t payload, const uint8_t dlc);

    [[nodiscard]] CyberGear_Result<> request_mcu_id();

    struct MitParams{
        real_t torque;
        real_t rad;
        real_t omega; 
        real_t kp; 
        real_t kd;
    };

    [[nodiscard]] CyberGear_Result<> ctrl(const MitParams & params);

    
    [[nodiscard]] CyberGear_Result<> on_receive(const CanMsg & msg);

    [[nodiscard]] CyberGear_Result<> enable(const Enable en, const bool clear_fault = true);

    [[nodiscard]] CyberGear_Result<> set_current_as_machine_home();

    [[nodiscard]] CyberGear_Result<> change_can_id(const uint8_t id);

    [[nodiscard]] CyberGear_Result<> request_read_para(const uint16_t idx);

    [[nodiscard]] CyberGear_Result<> request_write_para(const uint16_t idx, const uint32_t data);

    [[nodiscard]] Option<uint64_t> get_device_mcu_id() const {return device_mcu_id_;}

private:
    hal::CanDrv can_drv_;
    const uint8_t host_id_;
    uint8_t node_id_;
    
    CyberGear_Fault fault_ = {};
    Option<uint64_t> device_mcu_id_ = None;


    Feedback feedback_ = {};

    [[nodiscard]] CyberGear_Result<> on_mcu_id_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
    [[nodiscard]] CyberGear_Result<> on_ctrl2_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
    [[nodiscard]] CyberGear_Result<> on_read_para_feed_back(const uint32_t id, const uint64_t data, const uint8_t dlc);
};

}