#pragma once

#include "core/math/realmath.hpp"
#include "core/stream/BufStream.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include "core/magic/enum_traits.hpp"

#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"

#include "types/regions/range2.hpp"



namespace ymd::robots{

struct YahboomQuatMotorDriver_Prelude{
    enum class Error_Kind{

    };


    DEF_ERROR_SUMWITH_HALERROR(Error, Error_Kind)

    template<typename T = void>
    using IResult = Result<T, Error>;
};

struct YahboomQuatMotorDriver_Uart final:
    public YahboomQuatMotorDriver_Prelude{
    static constexpr size_t MAX_BUF_SIZE = 32;

    YahboomQuatMotorDriver_Uart(Some<hal::Uart *> && uart) : 
        uart_(uart.deref())
    {
        // reconf(cfg);
    }

    enum class MotorType:uint8_t{
        _520,
        _310,
        TT_Encoder,
        TT_Without_Encoder
    };

    void set_motor_type(MotorType type){
        send_var("mtype", std::bit_cast<uint8_t>(type));
    }

    void set_dead_zone(const uint16_t dead_zone){
        send_var("deadzone", dead_zone);
    }

    void set_deduction(const uint16_t deduction){
        send_var("mline", deduction);
    }

    void set_wheel_radius_mm(const uint16_t radius_mm){
        send_var("wdiameter", radius_mm);
    }

    void set_pid(const real_t kp, const real_t ki, const real_t kd){
        send_var("MPID", kp, ki, kd);
    }

    void refactory(){
        send_var("flash_reset");
    }

    void set_speed(const std::array<int16_t, 4> speed){
        send_var("spd", speed[0], speed[1], speed[2], speed[3]);
    }

    void set_pwm(const std::array<uint16_t, 4> pwm){
        send_var("pwm", pwm[0], pwm[1], pwm[2], pwm[3]);
    }
private:
    template<typename ... Args>
    void send_var(StringView name, Args && ... args){    
        std::array<char, MAX_BUF_SIZE> buf;
        BufStream bs{buf};
        bs.set_splitter(',');
        bs.set_eps(2);

        bs << '$' << name; 
        
        if constexpr (sizeof...(args)) {
            bs << ':';
            bs.prints(std::forward<Args>(args)...);
        }

        bs << '#';

        send_line(StringView(bs));
    }

    void send_line(const StringView line){
        uart_.writeN(line.data(), line.size());
    }

    hal::Uart & uart_;

};


}