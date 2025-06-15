#pragma once

#include "config.hpp"
#include "calibrate_utils.hpp"


#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/SecondOrderLpf.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"
#include "dsp/controller/adrc/tracking_differentiator.hpp"
#include "dsp/controller/adrc/command_shaper.hpp"
#include "dsp/controller/adrc/leso.hpp"
#include "dsp/controller/adrc/utils.hpp"


namespace ymd{



struct PositionSensor final{

    constexpr void update(const real_t next_raw_lap_position){
        const auto corrected_lap_position = correct_raw_position(next_raw_lap_position);
        const auto delta_position = map_lap_postion_to_delta(lap_position_, corrected_lap_position);
        lap_position_ = corrected_lap_position;

        cont_position_ += delta_position;
        td_.update(cont_position_);
    }

    constexpr q20 lap_position() const{
        return lap_position_;
    }

    constexpr q20 position() const{
        return td_.get().position;
    }

    constexpr q20 speed() const {
        return td_.get().speed;
    }

    CalibrateDataVector forward_cali_vec = {MOTOR_POLE_PAIRS};
    CalibrateDataVector backward_cali_vec = {MOTOR_POLE_PAIRS};

    constexpr q16 correct_raw_position(const q16 raw_lap_position) const {
        const auto corr1 = forward_cali_vec[raw_lap_position].to_inaccuracy();
        const auto corr2 = backward_cali_vec[raw_lap_position].to_inaccuracy();

        return raw_lap_position + mean(corr1, corr2);
    }

    struct CalibrateDataStorage{
        struct CompressedInaccuracy { 
            using Raw = uint16_t;

            constexpr CompressedInaccuracy ():
                raw_(0){;}

            explicit constexpr CompressedInaccuracy (const Raw raw):
                raw_(raw){;}

            static constexpr Option<CompressedInaccuracy> from(const q16 inaccuracy){
                if(is_input_valid(inaccuracy)) return None;
                return Some(CompressedInaccuracy(compress(inaccuracy)));
            }

            constexpr q16 to_real() const{
                return decompress(raw_);
            }

            static constexpr bool is_input_valid(const q16 inaccuracy){
                return ABS(inaccuracy) < 1;
            }

            static constexpr uint16_t compress(const q16 count){
                return uint16_t(count.to_i32());
            }

            static constexpr q16 decompress(const Raw raw){
                return q16::from_i32(raw);
            }

        private:
            Raw raw_;
        };

        std::array<CompressedInaccuracy, MOTOR_POLE_PAIRS> buf = {};

        constexpr Result<void, void> load_from_buf(std::span<const uint8_t> pbuf){
            if(pbuf.size() != sizeof(buf)) return Err();

            std::memcpy(buf.data(), pbuf.data(), sizeof(buf));
            return Ok();
        }
    };
private:

    static constexpr q16 map_lap_postion_to_delta(const q16 last_lap_position, const q16 lap_position){
        const auto delta = lap_position - last_lap_position;
        if(delta > 0.5_q16) return delta - 1;
        else if(delta < -0.5_q16) return delta + 1;
        return delta;
    }

    // dsp::Leso leso_{dsp::Leso::Config{
    //     .b0 = 1,
    //     .w = MC_W / 3,
    //     .fs = ISR_FREQ
    // }};
    // using Td = dsp::TrackingDifferentiatorByOrders<2>;
    class MotorTrackingDifferentiator{
    public:
        struct Config{
            q8 r;
            uint32_t fs;
        };

        struct State{
            q20 position;
            q20 speed;

            constexpr void reset(){
                position = 0;
                speed = 0;
            }
        };
        constexpr void reconf(const Config & cfg){
            r_ = cfg.r;
            dt_ = 1_q24 / cfg.fs;
        }

        constexpr MotorTrackingDifferentiator(const Config & cfg){
            reconf(cfg);
            reset();
        }

        constexpr void reset(){
            state_.reset();
        }


        constexpr void update(const q20 u){
            const auto r = r_;
            const auto r_2 = r * r;

            const auto x1 = state_.position;
            const auto x2 = state_.speed;

            state_.position += x2 * dt_; 
            state_.speed += (- 2 * x2 * r - (x1 - u) * r_2) * dt_;
        }

        constexpr const State & get() const {return state_;}
    private:
        q16 r_ = 0;
        q24 dt_ = 0;
        // using State = dsp::StateVector<q20, N>;

        State state_;
    };

    using Td = MotorTrackingDifferentiator;
    using TdConfig = typename Td::Config;

    static constexpr TdConfig CONFIG{
        .r = 50,
        .fs = ISR_FREQ
    };

    Td td_{
        CONFIG
    };

    q16 lap_position_;
    q16 cont_position_;
};

}