#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "drivers/VirtualIO/PCA9685/pca9685.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

using namespace ymd;

using namespace ymd::drivers;

#define SCL_GPIO hal::PD<2>()
#define SDA_GPIO hal::PC<12>()

class PCA9685Channel final:public hal::PwmIntf,  hal::GpioIntf{
public:
    PCA9685Channel(PCA9685 & _pca, const Nth nth):
        pca_(_pca), nth_(nth){;}
    
    PCA9685Channel(const PCA9685Channel & other) = delete;
    PCA9685Channel(PCA9685Channel && other) = delete;
    

    friend class PCA9685;

    void set_dutycycle(const real_t duty){
        pca_.set_pwm(nth_, 0, uint16_t(duty << 12)).unwrap();
    }
    __fast_inline void set() {this->set_dutycycle(real_t(1));}
    __fast_inline void clr() {this->set_dutycycle(real_t(0));}
    __fast_inline void write(const BoolLevel val){
        this->set_dutycycle(real_t(int(val.to_bool())));
    }

    BoolLevel read() const;

    __fast_inline Nth nth() const {return nth_;}

    void set_mode(const hal::GpioMode mode){}
private:
    PCA9685 & pca_;
    Nth nth_;

};

void pca_tb(OutputStream & logger){
    auto scl_gpio_ = SCL_GPIO;
    auto sda_gpio_ = SDA_GPIO;

    hal::I2cSw i2c{&scl_gpio_, &sda_gpio_};

    static constexpr int servo_freq = 50;
    
    i2c.init({100_KHz});
    PCA9685 pca{&i2c};
    pca.init({
        .freq = servo_freq, 
        .trim = 1.09_r
    }).unwrap();

    auto pwm1 = PCA9685Channel(pca, 0_nth);
    auto pwm2 = PCA9685Channel(pca, 1_nth);
    MG995 servo_left{pwm1};
    MG995 servo_right{pwm2};

    while(true){
        servo_left.set_angle(0);
        servo_right.set_angle(0);
    }
}

void pca_main(){
    DEBUGGER_INST.init({576000});
    DEBUGGER.retarget(&DEBUGGER_INST);

    pca_tb(DEBUGGER);
}