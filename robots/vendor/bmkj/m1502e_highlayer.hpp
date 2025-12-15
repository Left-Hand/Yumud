#pragma once 

#include "m1502e_msgs.hpp"
#include "middlewares/reactive/cellprobe.hpp"

namespace ymd::robots::bmkj::m1502e{

struct motor_recipes{
    struct [[nodiscard]] Timeout{};
    struct [[nodiscard]] ShutDown{};
    struct [[nodiscard]] SendFrame{const hal::BxCanFrame frame;};
    struct [[nodiscard]] PropagationException{};
};

struct [[nodiscard]] MotorHealthGuard{
    using CanFrame = m1502e::CanFrame;
    using Exception = m1502e::Exception;

    struct [[nodiscard]] Recipe:public Sumtype<
        motor_recipes::Timeout,
        motor_recipes::ShutDown,
        motor_recipes::SendFrame,
        motor_recipes::PropagationException
    >{

    };


    struct [[nodiscard]] Switches{
        struct{
            uint16_t shutdown_on_fault:1;
        } fault_protect;
    };


    struct [[nodiscard]] Config{
        Switches switches;
        m1502e::LoopMode loop_mode;
        uint8_t report_duration_ms;
        uint16_t timeout_ms;
    };

    explicit constexpr MotorHealthGuard(const Config & cfg): 
        cfg_(cfg){;}


    struct [[nodiscard]] FeedBack{
        m1502e::ExceptionCode exception_code;
        m1502e::LoopMode loop_mode;
    };

    constexpr Option<Recipe> poll(const Milliseconds now) {
        //使用卫语句优先处理更严重的问题

        if(health_.may_exception.is_none() == false){
            //电机发生了异常
            const auto exception = health_.may_exception.unwrap();
            return handle_exception(exception);
        }

        {//心跳检查
            if(health_.may_last_heartbeat_tick.is_none()){
                //没有心跳过 以第一次侦测开始计时
                health_.may_last_heartbeat_tick = Some(now);
            }else{
                const auto last_heartbeat_tick = health_.may_last_heartbeat_tick.unwrap();
            }
        }
    }
private:
    Config cfg_;

    struct [[nodiscard]] MotorHealth{
        using Self = MotorHealth;
        Option<m1502e::Exception> may_exception;
        Option<m1502e::LoopMode> loop_mode;
        Option<Milliseconds> may_last_heartbeat_tick;

        static constexpr Self from_default(){
            return Self{
                .may_exception = None,
                .loop_mode = None,
                .may_last_heartbeat_tick = None
            };
        }
    };


    MotorHealth health_ = MotorHealth::from_default();

    constexpr Option<Recipe> handle_exception(const Exception exception) {
        [[maybe_unused]] const auto & switches = cfg_.switches;
        switch(exception){
            case Exception::UnderVoltage2:
                // 电压小于17V
                break;
            case Exception::UnderVoltage1:
                // 17V - 22V
                break;
            case Exception::OverVoltage:
                // 36V +
                break;
            case Exception::OverCurrent:
                // 15A +
                break;
            case Exception::OverSpeed:
                //250RPM +
                break;
            case Exception::OverHeat2:
                // 120C +
                break;
            case Exception::OverHeat1:
                // 80C +
                break;
            case Exception::EncoderError:

                break;
            case Exception::EncoderSingalCorrupted:

                break;
            case Exception::CommunicationTimeout:

                break;
            case Exception::Stall:

                break;
        }
        return None;
    }
};



}