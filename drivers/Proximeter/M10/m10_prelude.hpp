//这个驱动尚未完成
//这个驱动尚未测试

//LD19是一款2D激光雷达 半径12M

// https://ws.dfrobot.com.cn/Fr1DgkvHXX4PYVgKpZgQIsL_lkEn

#pragma once

#include "hal/bus/uart/uart.hpp"
#include "core/math/real.hpp"
#include "core/utils/sumtype.hpp"
#include "types/vectors/polar.hpp"


namespace ymd::drivers{


struct M10_Prelude{

    static constexpr size_t DEFAULT_UART_BAUD = 460800;

    static constexpr uint8_t HEADER1_TOKEN = 0xa5;
    static constexpr uint8_t HEADER2_TOKEN = 0x5a;
    static constexpr size_t POINTS_PER_FRAME = 42;


    struct [[nodiscard]] LidarSpinSpeed{
    public:
        constexpr explicit LidarSpinSpeed(uint16_t bits):bits_(bits){;}

        [[nodiscard]] constexpr iq16 to_turns_per_secs() const{
            constexpr iq16 RATIO = iq16(1.0 / 360);
            return RATIO * bits_;
        }
    private:
        uint16_t bits_;
    };

    struct [[nodiscard]] LidarAngle{
    public:
        constexpr explicit LidarAngle(uint16_t bits):bits_(bits){;}

        [[nodiscard]] constexpr uq24 to_turns() const{
            constexpr auto RATIO = uq24(1.0 / 360 * 0.01);
            return RATIO * bits_;
        }

    private:
        uint16_t bits_;
    };

    struct [[nodiscard]] LidarDistance{
    public:
        constexpr explicit LidarDistance(uint16_t bits):bits_(bits){;}

        [[nodiscard]] constexpr uq24 to_meters() const{
            constexpr auto RATIO = uq24(0.001);
            return RATIO * bits_;
        }

    private:
        uint16_t bits_;
    };

    struct [[nodiscard]] LidarFrame final{
        LidarAngle start_angle;
        LidarSpinSpeed spin_speed;
        std::array<LidarDistance, POINTS_PER_FRAME> distances;

        struct Iterator{
            static constexpr uq24 DELTA_TURNS = uq16(15.0 / 360 / POINTS_PER_FRAME);
            constexpr Iterator(const LidarFrame & frame):frame_(frame){
                current_turns_ = frame_.start_angle.to_turns();
            }

            constexpr bool has_next() const{
                return index_ < frame_.distances.size();
            }

            constexpr Polar<uq24> next(){
                const auto ret = Polar<uq24>{
                    .amplitude = frame_.distances[index_].to_meters(),
                    .phase = Angle<uq24>::from_turns(current_turns_)
                };
                index_ += 1;
                current_turns_ += DELTA_TURNS;
                return ret;
            };
        private:

            const LidarFrame & frame_;
            size_t index_ = 0;
            uq24 current_turns_ = 0;
        };
    };

    static constexpr size_t FRAME_SIZE = sizeof(LidarFrame);

    struct Events{
        struct [[nodiscard]] DataReady{
            const LidarFrame & frame;
        };
        struct [[nodiscard]] InvalidCrc{
            uint8_t expected;
            uint8_t actual;
        };
    };

    struct Event:public Sumtype<Events::DataReady>{

    };

    using Callback = std::function<void(Event)>;
};


class M10_StreamParser final:public M10_Prelude{
    explicit M10_StreamParser(Callback callback):
        callback_(callback)
    {
        reset();
    }

    void push_byte(const uint8_t byte){
        switch(state_){
            case State::WaitingHeader1:
                if(byte != HEADER1_TOKEN){reset(); break;}
                state_ = State::WaitingHeader2;
                break;
            case State::WaitingHeader2:
                if(byte != HEADER2_TOKEN){reset(); break;}
                state_ = State::Remaining;
                break;
            case State::Remaining:
                bytes_[bytes_count_] = byte;
                bytes_count_ += 1;
                if(bytes_count_ == FRAME_SIZE){
                    flush();
                    reset();
                }
                break;
        
        }
    }

    void push_bytes(const std::span<const uint8_t> bytes){
        for(const auto byte : bytes){
            push_byte(byte);
        }
    }

    void flush(){
        if(callback_ == nullptr) __builtin_abort();
    }

    void reset(){
        state_ = State::WaitingHeader1;
        bytes_count_ = 0;
    }
private:

    Callback callback_;
    union{
        LidarFrame frame_;
        std::array<uint8_t, sizeof(LidarFrame)> bytes_;
    };
    size_t bytes_count_ = 0;

    enum class State:uint8_t{
        WaitingHeader1,
        WaitingHeader2,
        Remaining
    };

    State state_ = State::WaitingHeader1;

};

}