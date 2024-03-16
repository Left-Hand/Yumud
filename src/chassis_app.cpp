#include "apps.h"

class GM25{
protected:
    PwmChannel & pwm_channel;
    Gpio & dir_pin;
    CaptureChannelConcept & cap_channel;
    bool rsv;
    static constexpr int speed_scaler = 400;
public:
    GM25(PwmChannel & _pwm_channel,Gpio & _dir_pin,CaptureChannelConcept & _cap_channel,const bool & _rsv = false):
            pwm_channel(_pwm_channel), dir_pin(_dir_pin), cap_channel(_cap_channel), rsv(_rsv){;}
    void setSpeed(const real_t & speed){
        pwm_channel.setDuty(abs(speed / speed_scaler));
        dir_pin = (speed > 0) ^ rsv;
    }

    void init(){
        dir_pin.OutPP();
    }
};


class Motion{
protected:
    GM25 & motorLeft;
    GM25 & motorRight;
public:

    Motion(GM25 & _motorLeft,GM25 & _motorRight): motorLeft(_motorLeft), motorRight(_motorRight){;}
    void setVelocity(const Vector2 & vel){
        motorLeft.setSpeed(vel.x);
        motorRight.setSpeed(vel.y);
    }

    void init(){
        motorLeft.init();
        motorRight.init();
    }
};

void chassis_app(){

    uart2.init(115200*4, Uart::Mode::TxOnly);


    Printer & log = uart2;
    log.setEps(4);

    auto TrigGpioA = Gpio(GPIOA, Pin::_5);
    auto TrigGpioB = Gpio(GPIOB, Pin::_1);

    auto TrigExtiCHA = ExtiChannel(TrigGpioA, 1, 2, ExtiChannel::Trigger::Rising);
    auto TrigExtiCHB = ExtiChannel(TrigGpioB, 1, 2, ExtiChannel::Trigger::Rising);
    auto CapA = CaptureChannelExti(TrigExtiCHA, TrigGpioA);
    auto CapB = CaptureChannelExti(TrigExtiCHB, TrigGpioB);
    CapA.init();
    CapB.init();

    timer3.init(3000);
    auto tim3ch1 = timer3.getChannel(TimerOC::Channel::CH1);

    auto tim3ch2 = timer3.getChannel(TimerOC::Channel::CH2);
    auto PwmA = PwmChannel(tim3ch1);
    auto PwmB = PwmChannel(tim3ch2);
    PwmA.init();
    PwmB.init();

    tim3ch1.setPolarity(false);
    tim3ch2.setPolarity(false);

    PwmA = real_t(0.1);
    PwmB = real_t(0.2);

    auto dirPinA = Gpio(GPIOA, Pin::_0);
    auto dirPinB = Gpio(GPIOA, Pin::_1);

    auto motorX = GM25(PwmA, dirPinA, CapA, false);
    auto motorY = GM25(PwmB, dirPinB, CapB, true);
    auto motion = Motion(motorX, motorY);
    motion.init();
    // auto filterA = BurrFilter_t<real_t>(real_t(0.7), real_t(40), 2);
    // auto filterB = BurrFilter_t<real_t>(real_t(0.7), real_t(40), 2);
    // auto speed_pid = PID_t<real_t>(real_t(0.0007), real_t(0), real_t(0));
    // real_t dutyA = real_t(1);
    while(1){
        // real_t targetA = real_t(348) + (439-348)*sign(cos(8*t));
        // real_t rpmA = filterA.update(CapA.getFreq());
        // real_t rawA = rpmA;
        // if(rpmA > 1000) rpmA = real_t(70);
        // dutyA = CLAMP(dutyA + speed_pid.update(targetA, rpmA), real_t(0), real_t(1));

        // real_t dutyB = real_t(0.2);
        // log.println(rpmA, filterB.update(CapB.getFreq()), dutyA, dutyB, rawA);
        // PwmA = dutyA; PwmB = dutyB;

        motion.setVelocity(Vector2(400,0).rotate(t));
        delay(10);
        reCalculateTime();
    }
};