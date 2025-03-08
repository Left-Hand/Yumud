#pragma once

#include "hal/bus/can/can.hpp"

#include "sys/utils/rustlike/Optional.hpp"
#include "sys/utils/rustlike/Result.hpp"

#include "sys/math/real.hpp"

namespace ymd::rmst{


// enum class CgFault:uint8_t{
//     COIL_A_OVERLOAD,
//     OVERLOAD,
//     ENC_NOT_CALI,
//     COIL_C_OVERLOAD,
//     COIL_B_OVERLOAD,
//     OVER_VOLTAGE,
//     UNDER_VOLTAGE,
//     DRIVER_FAULT,
//     OVER_TEMPERATURE,
// };

struct CgFault{

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
};static_assert(sizeof(CgFault) == 2);


enum class CgError:uint8_t{
    PRAGRAM_UNHANDLED,
    PRAGRAM_TODO,
    RET_DLC_SHORTER,
    RET_DLC_LONGER,
    RET_UNKOWN_CMD,
    CMD_LOWER_THAN_LIMIT,
    CMD_HIGHER_THAN_LIMIT,
};

enum class CgCommand:uint8_t{
    GET_DEVICE_ID = 0,
    SEND_CTRL1 = 1,
    FBK_CTRL1 = 2,
    EN_MOT = 3,
    DISEN_MOT = 4,
    SET_MACHINE_HOME = 6,
    SET_CAN_ID = 7,
    READ_PARA = 17,
    WRITE_PARA = 18,
    FBK_FAULT = 21
};


template<typename T = void>
using CgResult = Result<T, CgError>;

namespace details{
    // enum class CgError:uint8_t{

    // };

    // template<typename T = void>
    // using CgResult = Result<T, CgError>;

    template<typename T>
    struct PerUint{
        uint16_t data;
        constexpr PerUint(const real_t value):data(static_cast<uint16_t>(
            iq_t<8>(INVLERP(get_min(), get_max(), CLAMP(value, get_min(), get_max())) * 65535))){;}
    
        constexpr operator real_t() const {return LERP(get_min(), get_max(), real_t(data) / 65535);} 

        static constexpr CgResult<void> check(const real_t value){
            if(unlikely(value > get_max())) return Err{CgError::CMD_HIGHER_THAN_LIMIT};
            if(unlikely(value < get_min())) return Err{CgError::CMD_LOWER_THAN_LIMIT};
            return Ok{};
        }
    private:
        static constexpr real_t get_min() {return std::get<0>(T::get_range());}
        static constexpr real_t get_max() {return std::get<1>(T::get_range());}
    };
    
    #define DEF_PER_UNIT(name, min, max)\
    struct name:public PerUint<name>{\
        static constexpr std::tuple<real_t, real_t> get_range(){return {static_cast<real_t>(min),static_cast<real_t>(max)};}\
    };\
    
    DEF_PER_UNIT(CmdRad, -2 * TAU, 2 * TAU)
    DEF_PER_UNIT(CmdOmega, -30 * TAU, 30 * TAU)
    DEF_PER_UNIT(CmdTorque, -12, 12)
    DEF_PER_UNIT(CmdKp, 0, 500)
    DEF_PER_UNIT(CmdKd, 0, 5)
    
    struct Temperature{
        uint16_t data;
        
        operator real_t() const {
            return real_t(data) / 10;
        }
    };

    #undef DEF_PER_UINT


    struct Feedback{
        real_t rad = {};
        real_t omega = {};
        real_t torque = {};
        // real_t current = {};
        // real_t voltage = {};
        real_t temperature = {};
    };
}



class MotorCyberGear{
public:


    // using CgError = details::CgError;
    // using CgResult = details::CgResult;

    using CanMsg = hal::CanMsg; 

    using Feedback = details::Feedback;

    class CanDrv{
        public:
            using CanMsg = hal::CanMsg; 

            CanDrv(hal::Can & can):can_(can){;}
        
            void transmit(const CanMsg & msg){
                can_.write(msg);
            }
        
        private:
            hal::Can & can_; 
    };


    MotorCyberGear(hal::Can & can, uint8_t host_id, uint8_t node_id):
        can_drv_(can), host_id_(host_id), node_id_(node_id){;}

    [[nodiscard]]CgResult<void> init();

    [[nodiscard]] CgResult<void> transmit(const CanMsg & msg);

    [[nodiscard]] CgResult<void> transmit(const uint32_t id, const uint64_t payload, const uint8_t dlc);

    [[nodiscard]] CgResult<void> requestMcuId();

    [[nodiscard]] CgResult<void> ctrl(const real_t cmd_torque, const real_t cmd_rad, const real_t cmd_omega, const real_t cmd_kp, const real_t cmd_kd);

    
    [[nodiscard]] CgResult<void> onReceive(const CanMsg & msg);

    [[nodiscard]] CgResult<void> enable(const bool en = true, const bool clear_fault = true);

    [[nodiscard]] CgResult<void> setCurrentAsMachineHome();

    [[nodiscard]] CgResult<void> changeCanId(const uint8_t id);

    [[nodiscard]] CgResult<void> requestReadPara(const uint16_t idx);

    [[nodiscard]] CgResult<void> requestWritePara(const uint16_t idx, const uint32_t data);

    Option<uint64_t> getDeviceMcuId() const {return device_mcu_id_;}

    private:
    CanDrv can_drv_;
    const uint8_t host_id_;
    uint8_t node_id_;
    
    CgFault fault_ = {};
    Option<uint64_t> device_mcu_id_ = None;


    Feedback feedback_ = {};

    [[nodiscard]] CgResult<void> onMcuIdFeedBack(const uint32_t id, const uint64_t data, const uint8_t dlc);
    [[nodiscard]] CgResult<void> onCtrl2FeedBack(const uint32_t id, const uint64_t data, const uint8_t dlc);
    [[nodiscard]] CgResult<void> onReadParaFeedBack(const uint32_t id, const uint64_t data, const uint8_t dlc);
};

}