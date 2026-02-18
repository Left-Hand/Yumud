#include "m3508.hpp"
#include "core/math/realmath.hpp"

using namespace ymd::robots::dji::m3508;



#if 0
#define LIMIT 1000

#define M3508_CHECK_INDEX if(index > size_ or index == 0) PANIC(); 




void M3508::init(){

    port.connected_flags_.set(index - 1, 1);
}

void M3508::reset(){
    position_filter_.reset();
    lap_turns_ = 0;
    curr_ = 0;
    speed_ = 0;
    temperature_ = 0;

    spd_pid = {3, 0, 0};
    pos_pid = {3, 0, 0};

    ctrl_method = CtrlMethod::NONE;
    
    targ_curr = 0;
    targ_spd = 0;
    targ_pos = 0;

    curr_delta = iq16(0.04);
    curr_setpoint = 0;
}


void M3508::tick(){
    const auto this_micros = clock::micros();
    micros_delta =this_micros - last_micros; 
    last_micros = this_micros;
    iq16 expect_curr = 0;
    switch(ctrl_method){
        default:
        case CtrlMethod::NONE:
            break;
        case CtrlMethod::CURR:
            expect_curr = targ_curr;
            break;
        case CtrlMethod::SPD:
            // // expect_curr = spd_pid.update(targ_spd, getSpeed());
            //             // expect_curr = pos_pid.update(targ_pos, getPosition());
            // static constexpr iq16 targ_spd = 4;
            // iq16 e1 = targ_pos - getPosition();
            // iq16 e2 = targ_spd - getSpeed();

            // static constexpr iq16 kp = iq16(30); 
            // static constexpr iq16 kd = iq16(7);
            
            // expect_curr = sqrt(ABS(e1)) * sign(e1) * kp + e2 * kd;  
            // // expect_curr = (e1) * kp + e2 * kd;  
            // break;
            break;
        case CtrlMethod::POS:{
            targ_spd = position_filter_.speed();
            iq16 e1 = targ_pos - get_position();
            iq16 e2 = targ_spd - get_speed();

            // static constexpr iq16 kp = iq16(2.35); 
            // static constexpr iq16 kd = iq16(0.85);

            static constexpr iq16 kp = iq16(2.75); 
            static constexpr iq16 kd = iq16(1.45);
            
            // static constexpr iq16 kd = iq16(1.95);

            // static constexpr iq16 kp = iq16(4); 
            // static constexpr iq16 kd = iq16(2.75);
            
            // static constexpr iq16 kp = iq16(6); 
            // static constexpr iq16 kd = iq16(3.75);

            // static constexpr iq16 kp = iq16(8); 
            // static constexpr iq16 kd = iq16(7.45);

            // static constexpr iq16 kp = iq16(9); 
            // static constexpr iq16 kd = iq16(10.45);

            // static constexpr iq16 kp = iq16(10); 
            // static constexpr iq16 kd = iq16(13.45);

            // static constexpr iq16 kp = iq16(11); 
            // static constexpr iq16 kd = iq16(16.45);

            // static constexpr iq16 ki = iq16(0.066);
            // static constexpr iq16 ki = iq16(0.0);
            // static iq16 ci = 0;
            // ci += (ki * (delta()>>4) * e1) >> 12;
            // ci = CLAMP2(ci, curr_limit_ * 0.1);

            // if(SIGN_DIFF(e1, e2)) ci = SIGN_AS(curr_limit_, e1);
            // static constexpr iq16 kd = iq16(13);
            // static constexpr iq16 kd = iq16(-20);
            
            expect_curr = sqrt(ABS(e1)) * sign(e1) * kp + e2 * kd;  
            // expect_curr = (e1) * kp + e2 * kd;  
            break;
        }
    }
    
    expect_curr = CLAMP2(expect_curr, curr_limit_);
    curr_setpoint = STEP_TO(curr_setpoint, expect_curr, curr_delta);
    if(ctrl_method != CtrlMethod::NONE){
        apply_target_current(curr_setpoint);
    }
}

void M3508::set_target_current(const iq16 _curr){
    ctrl_method = CtrlMethod::CURR;
    targ_curr = _curr;
}


void M3508::set_target_speed(const iq16 _spd){
    ctrl_method = CtrlMethod::SPD;
    targ_spd = _spd;
}

void M3508::set_target_position(const iq16 _pos){
    ctrl_method = CtrlMethod::POS;
    targ_pos = _pos;
}

void M3508::update_measurements(
    const uq32 lap_position, 
    const iq16 current, 
    const iq16 spd, 
    const iq16 temp
){
    lap_turns_ = lap_position;
    curr_ = current;
    speed_ = spd; 
    temperature_ = temp;
}
        
void M3508Port::init(){
    for(auto & slave : slaves_){
        slave.init();
    }
}


void M3508Port::reset(){
    for(auto & slave : slaves_){
        slave.reset();
    }
}

void M3508Port::tick(){
    TODO();


    auto write_can_frame = [&](const hal::BxCanFrame & frame) {
        return can_.write(frame);
    };


    if((connected_flags_ & std::bitset<8>(0x0f)).any()){
        write_can_frame(hal::BxCanFrame(
            hal::CanStdId::from_bits(HIGHER_ADDRESS), 
            hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(tx_datum[0]))
        )).examine();
    }

    if((connected_flags_ & std::bitset<8>(0xf0)).any()){
        write_can_frame(hal::BxCanFrame(
            hal::CanStdId::from_bits(LOWER_ADDRESS), 
            hal::BxCanPayload::from_u64(std::bit_cast<uint64_t>(tx_datum[1]))
        )).examine();
    }

    for(size_t i = 0; i < size_; i++){
        if(connected_flags_.test(i)){
            slaves_[i].tick();
        }
    }
}

M3508 & M3508Port::operator[](const size_t index){
    M3508_CHECK_INDEX
    return slaves_[index - 1];
}

void M3508Port::set_target_current(const iq16 current, const size_t index){
    M3508_CHECK_INDEX
    curr_cache[index - 1] = curr_to_currdata(current);
}

void M3508Port::update_slave(const hal::BxCanFrame & frame, const size_t index){
    M3508_CHECK_INDEX
    const auto rx_context = std::bit_cast<RxContext>(frame.payload_u64());
    auto & slave = slaves_[index - 1];

    const auto angle_u13 = uint16_t(BSWAP_16(rx_context.angle_8192_be));
    slave.update_measurements(
        (uq32::from_bits(static_cast<uint32_t>(angle_u13) << (32 - 13))),
        currdata_to_curr(rx_context.current_be),
        iq16(int16_t(BSWAP_16(rx_context.speed_rpm_be))) / 60,
        rx_context.temp
    );
}
#endif