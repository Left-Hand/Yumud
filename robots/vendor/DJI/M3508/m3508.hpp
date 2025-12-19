#pragma once

#include "hal/bus/can/can.hpp"

#include "dsp/controller/pid_ctrl.hpp"
#include "dsp/motor_ctrl/position_filter.hpp"

#include <bitset>

#include "drivers/Encoder/Encoder.hpp"

namespace ymd::robots::dji::m3508{
static constexpr uint16_t NUM_HIGHER_QUAD_CAN_ID = 0x200;
static constexpr uint16_t NUM_LOWER_QUAD_CAN_ID = 0x1ff;
static constexpr size_t NUM_MAX_MOTORS = 8;

struct [[nodiscard]] CurrentCode{
    using Self = CurrentCode;
    uint16_t bits;
    static constexpr Self from_amps(const iq16 amps){
        int16_t temp = int16_t((amps / 20)* 16384);
        return Self{
            .bits = std::bit_cast<uint16_t>(BSWAP_16(temp))
        };
    }
    constexpr iq16 to_amps() const {
        return (iq16(bits) / 16384) * 20;
    }
};

struct [[nodiscard]] AngleCode{
    uint16_t bits;

    constexpr Angular<uq32> to_angle() const {
        const auto angle_u13 = uint16_t(BSWAP_16(bits));
        const auto turns = uq32::from_bits(angle_u13 << (32u - 13u));
        return Angular<uq32>::from_turns(turns);
    }
};

struct [[nodiscard]] SpeedCode{
    uint16_t bits;

    constexpr iq16 to_tps() const {
        return iq16(int16_t(BSWAP_16(bits))) / 60;
    }
};


struct [[nodiscard]] TxContext{
    std::array<CurrentCode, 4> current_codes;  
};

static_assert(sizeof(TxContext) == 8);

struct [[nodiscard]] RxContext{
    AngleCode angle_code;
    CurrentCode current_code;
    SpeedCode speed_code;
    uint8_t temp;
    uint8_t __resv__;
};

static_assert(sizeof(RxContext) == 8);


#if 0
class M3508Port{
public:
class M3508{
public:
    class M3508Encoder final: public drivers::EncoderIntf{
    public:
        using Error = drivers::EncoderError;

        M3508Encoder(M3508 & owner):owner_(owner){;}
        Result<void, Error> update(){
            //pass
            return Ok();
        }
        Result<Angular<uq32>, Error> read_lap_angle() {
            return Ok(Angular<uq32>::from_turns(owner_.lap_turns_));
        }

    private:
        M3508 & owner_;
    };

    M3508Encoder enc_{*this};
    dsp::PositionFilter position_filter_ = dsp::PositionFilter{
        dsp::PositionFilter::Config{
            .fs = 500,
            .r = 1
        }
    };
    
    M3508(M3508Port & _port, const size_t _index):
        port(_port), index(_index){reset();}

    auto & operator = (const M3508 & other) = delete;
    auto & operator = (M3508 && other) = delete;


    friend class M3508Port;
    friend class M3508Encoder;

    void apply_target_current(const iq16 _curr){port.set_target_current(_curr, index);}

    void update_measurements(
        const uq32 lap_position, const iq16 curr, const iq16 spd, const iq16 temp);
public:
    void init();
    void tick();
    void reset();
    void set_target_current(const iq16 curr_);
    void set_target_speed(const iq16 spd_);
    void set_target_position(const iq16 pos_);

    iq16 get_position() {
        // return  / reduction_ratio;
        return position_filter_.accumulated_angle().to_turns() / reduction_ratio;
        // return 0;
    }
    iq16 get_current() const {return curr_;}
    iq16 get_speed() const {return speed_ / reduction_ratio * iq16(2.5);}
    iq16 read_temp() const {return temperature_;}
    auto delta(){return micros_delta;}
    auto & enc() {return enc_;}
private:
    M3508Port & port;
    const size_t index;
    static constexpr int reduction_ratio = 19;
    
    uq32 lap_turns_ = 0;
    iq16 curr_ = 0;
    iq16 speed_ = 0;
    iq16 temperature_ = 0;
    iq16 curr_limit_ = 10;


    PID<iq16> spd_pid = {3, 0, 0};
    PID<iq16> pos_pid = {3, 0, 0};

    enum class CtrlMethod:uint8_t{
        NONE,
        CURR,
        SPD,
        POS
    };


    CtrlMethod ctrl_method = CtrlMethod::NONE;
    
    iq16 targ_curr = 0;
    iq16 targ_spd = 0;
    iq16 targ_pos = 0;

    iq16 curr_delta = iq16(0.02);
    iq16 curr_setpoint = 0;

    iq16 last_t = 0;

    Microseconds last_micros;
    Microseconds micros_delta;

};

    M3508Port(hal::Can & can):can_(can){reset();}
    
    void init();

    void reset();

    void tick();

    M3508 & operator[](const size_t index);
    auto & slaves(){
        return slaves_;
    }


private:


    
    hal::Can & can_;
    size_t size_ = MAX_SLAVES_COUNT;
    std::bitset<MAX_SLAVES_COUNT> connected_flags_;




    std::array<M3508, MAX_SLAVES_COUNT> slaves_ = {
        M3508{*this, 1},
        M3508{*this, 2},
        M3508{*this, 3},
        M3508{*this, 4},
        M3508{*this, 5},
        M3508{*this, 6},
        M3508{*this, 7},
        M3508{*this, 8},
    };
    
    union{
        uint16_t curr_cache[8];
        struct{
            TxContext tx_datum[2];
        };
    };
    

    void set_target_current(const iq16 curr_, const size_t index);

    void update_slave(const hal::BxCanFrame & frame, const size_t index);

};
#endif
};