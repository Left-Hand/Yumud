#include "IST8310.hpp"

using namespace ymd;
using namespace ymd::drivers;

#ifdef IST8310_DEBUG
#undef IST8310_DEBUG
#define IST8310_DEBUG(...) DEBUG_PRINTLN(__VA_ARGS__);
#define IST8310_PANIC(...) PANIC(__VA_ARGS__)
#define IST8310_ASSERT(cond, ...) ASSERT(cond, __VA_ARGS__)
#else
#define IST8310_DEBUG(...)
#define IST8310_PANIC(...)  PANIC()
#define IST8310_ASSERT(cond, ...) ASSERT(cond)
#endif

#define WRITE_REG(reg) write_reg(reg.address, reg);
#define READ_REG(reg) read_reg(reg.address, reg);


void IST8310::init(){
    reset();
    delay(10);

    if(validate() == false) HALT;

    enableContious(false);
    setXAverageTimes(AverageTimes::_4);
    setYAverageTimes(AverageTimes::_4);

    update();
}
void IST8310::update(){
    read_burst(axis_x_reg.address, &axis_x_reg, 3);
}

bool IST8310::validate(){
    auto & reg = whoami_reg;
    READ_REG(reg);
    return reg == reg.expected_value;
}

void IST8310::reset(){
    auto & reg = ctrl2_reg;
    reg.reset = 1;
    WRITE_REG(reg)
    reg.reset = 0;
}

void IST8310::enableContious(const bool en){
    auto & reg = ctrl1_reg;
    reg.cont = en;
    WRITE_REG(reg);
}


void IST8310::setXAverageTimes(const AverageTimes times){
    setAverageTimes(true, times);
}


void IST8310::setYAverageTimes(const AverageTimes times){
    setAverageTimes(false, times);
}

void IST8310::setAverageTimes(bool is_x, AverageTimes times){
    auto & reg = average_reg;
    if(is_x){
        reg.x_times = uint8_t(times);
    }else{
        reg.y_times = uint8_t(times);
    }

    WRITE_REG(reg)
}

Option<Vector3_t<q24>> IST8310::read_mag(){
    auto conv = [](const int16_t data) -> real_t{
        return data * real_t(0.3);
    };

    return Some{Vector3_t<q24>{
        conv(axis_x_reg),
        conv(axis_y_reg),
        conv(axis_z_reg)
    }};
}

int IST8310::getTemperature(){
    return temp_reg;
}

bool IST8310::busy(){
    auto & reg = status1_reg;
    READ_REG(reg);
    return reg.drdy == false;
}

void IST8310::enableInterrupt(const bool en){
    auto & reg = ctrl2_reg;
    reg.int_en = en;
    WRITE_REG(reg);
}

void IST8310::setInterruptLevel(const BoolLevel lv){
    auto & reg = ctrl2_reg;
    reg.drdy_level = bool(lv);
    WRITE_REG(reg);
}

void IST8310::sleep(const bool en){
    auto & reg = ctrl1_reg;
    reg.awake = !en;
    WRITE_REG(reg);
}

bool IST8310::getInterruptStatus(){
    auto & reg = status2_reg;
    READ_REG(reg);
    return reg.on_int;
}