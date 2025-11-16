#include "m3508.hpp"
#include "core/math/realmath.hpp"

using namespace ymd::robots;

using M3508 = M3508Port::M3508;
#define LIMIT 1000

#define M3508_CHECK_INDEX if(index > size_ or index == 0) PANIC(); 

static constexpr uint16_t curr_to_currdata(const iq16 curr){
    int16_t temp = int16_t((curr / 20)* 16384);
    return BSWAP_16(temp);
}

static constexpr iq16 currdata_to_curr(const uint16_t currdata_msb){
    int16_t currdata = BSWAP_16(currdata_msb);
    return (iq16(currdata) / 16384) * 20;
};


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
            // iq16 pos_err = targ_pos - getPosition();
            // iq16 spd_err = targ_spd - getSpeed();

            // static constexpr iq16 kp = iq16(30); 
            // static constexpr iq16 kd = iq16(7);
            
            // expect_curr = sqrt(ABS(pos_err)) * sign(pos_err) * kp + spd_err * kd;  
            // // expect_curr = (pos_err) * kp + spd_err * kd;  
            // break;
            break;
        case CtrlMethod::POS:{
            targ_spd = position_filter_.speed();
            iq16 pos_err = targ_pos - get_position();
            iq16 spd_err = targ_spd - get_speed();

            // static constexpr iq16 kp = iq16(2.35); 
            // static constexpr iq16 kd = iq16(0.85);

            static constexpr iq16 kp = iq16(2.75); 
            static constexpr iq16 kd = real_t(1.45);
            
            // static constexpr real_t kd = real_t(1.95);

            // static constexpr real_t kp = real_t(4); 
            // static constexpr real_t kd = real_t(2.75);
            
            // static constexpr real_t kp = real_t(6); 
            // static constexpr real_t kd = real_t(3.75);

            // static constexpr real_t kp = real_t(8); 
            // static constexpr real_t kd = real_t(7.45);

            // static constexpr real_t kp = real_t(9); 
            // static constexpr real_t kd = real_t(10.45);

            // static constexpr real_t kp = real_t(10); 
            // static constexpr real_t kd = real_t(13.45);

            // static constexpr real_t kp = real_t(11); 
            // static constexpr real_t kd = real_t(16.45);

            // static constexpr real_t ki = real_t(0.066);
            // static constexpr real_t ki = real_t(0.0);
            // static real_t ci = 0;
            // ci += (ki * (delta()>>4) * pos_err) >> 12;
            // ci = CLAMP2(ci, curr_limit_ * 0.1);

            // if(SIGN_DIFF(pos_err, spd_err)) ci = SIGN_AS(curr_limit_, pos_err);
            // static constexpr real_t kd = real_t(13);
            // static constexpr real_t kd = real_t(-20);
            
            expect_curr = sqrt(ABS(pos_err)) * sign(pos_err) * kp + spd_err * kd;  
            // expect_curr = (pos_err) * kp + spd_err * kd;  
            break;
        }
    }
    
    expect_curr = CLAMP2(expect_curr, curr_limit_);
    curr_setpoint = STEP_TO(curr_setpoint, expect_curr, curr_delta);
    if(ctrl_method != CtrlMethod::NONE){
        apply_target_current(curr_setpoint);
    }
}

void M3508::set_target_current(const real_t _curr){
    ctrl_method = CtrlMethod::CURR;
    targ_curr = _curr;
}


void M3508::set_target_speed(const real_t _spd){
    ctrl_method = CtrlMethod::SPD;
    targ_spd = _spd;
}

void M3508::set_target_position(const real_t _pos){
    ctrl_method = CtrlMethod::POS;
    targ_pos = _pos;
}

void M3508::update_measurements(
    const uq32 lap_position, 
    const iq16 curr, 
    const iq16 spd, 
    const iq16 temp
){
    lap_turns_ = lap_position;
    curr_ = curr;
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

    static constexpr uint16_t HIGHER_ADDRESS = 0x200;
    static constexpr uint16_t LOWER_ADDRESS = 0x1ff;
    auto write_can_msg = [&](const hal::CanMsg & msg) {
        return can_.write(msg);
    };


    if((connected_flags_ & std::bitset<8>(0x0f)).any()){
        write_can_msg(hal::CanMsg::from_bytes(
            hal::CanStdId(HIGHER_ADDRESS), 
            std::bit_cast<std::array<uint8_t, 8>>(tx_datas[0])
        )).examine();
    }

    if((connected_flags_ & std::bitset<8>(0xf0)).any()){
        write_can_msg(hal::CanMsg::from_bytes(
            hal::CanStdId(LOWER_ADDRESS), 
            std::bit_cast<std::array<uint8_t, 8>>(tx_datas[1])
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

void M3508Port::set_target_current(const iq16 curr, const size_t index){
    M3508_CHECK_INDEX
    curr_cache[index - 1] = curr_to_currdata(curr);
}

void M3508Port::update_slave(const hal::CanMsg & msg, const size_t index){
    M3508_CHECK_INDEX
    const auto rx_data = std::bit_cast<RxData>(msg.payload_as_u64());
    auto & slave = slaves_[index - 1];

    const auto angle_u13 = uint16_t(BSWAP_16(rx_data.angle_8192_msb));
    slave.update_measurements(
        (uq32::from_bits(static_cast<uint32_t>(angle_u13) << (32 - 13))),
        currdata_to_curr(rx_data.curr_data_msb),
        iq16(int16_t(BSWAP_16(rx_data.speed_rpm_msb))) / 60,
        rx_data.temp
    );
}
