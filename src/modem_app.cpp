#include "apps.h"



class SimpleModem{
protected:
    PwmChannel & pwm_out;
    uint8_t cnt;
    uint8_t cvr;
    uint8_t arr;
public:
    SimpleModem(PwmChannel & _pwm_out):pwm_out(_pwm_out){;}
    void tick(){
        cnt++;
        if(cnt > cvr) pwm_out = real_t(0.1);
        else pwm_out = real_t(0);
        if(cnt == arr) cnt = 0;
    }

    void setCode(const uint8_t _cvr, const uint8_t _arr){
        cvr = _cvr;
        arr = _arr;
        cnt = 0;
    }
};

class SimpleDeModem{

};

class AwLed{
protected:
    AW9523 & aw9523;
    Pin pin;
public:
    AwLed(AW9523 & _aw9523, const Pin & _pin):aw9523(_aw9523), pin(_pin){;}
    void init(){
        aw9523.enableLedMode(pin);
    }
    AwLed & operator = (const real_t & _duty){
        aw9523.setLedCurrent(pin,int(255 * _duty));
        return *this;
    }
};

//Laser PB8 TM4CH3
//TrigIn PA4
void modem_app(){


    // auto canTxPin = Gpio(CAN1_Port, (Pin)CAN1_TX_Pin);
    // // auto canRxPin = Gpio(CAN1_Port, (Pin)CAN1_RX_Pin);
    // canTxPin.OutAfPP();
    // while(true){

    // }
    uart2.init(115200*4);
    Printer & log = uart2;
    log.setEps(4);


    auto i2c_scl = Gpio(GPIOD, Pin::_1);
    auto i2c_sda = Gpio(GPIOD, Pin::_0);
    auto i2csw = I2cSw(i2c_scl, i2c_sda);
    i2csw.init(0);

    auto aw_drv = I2cDrv(i2csw, 0b10110000);
    auto aw = AW9523(aw_drv);
    aw.init();

    auto led = AwLed(aw, Pin::_9);
    led.init();

    timer4.init(38000);
    auto tim4ch3 = timer4.getChannel(TimerOC::Channel::CH3);
    tim4ch3.init();

    auto pwm_out = PwmChannel(tim4ch3);
    pwm_out = real_t(0.1);

    auto io = Gpio(GPIOD, Pin::_1);
    io.OutPP();

    auto NecEnc = SimpleModem(pwm_out);

    timer1.init(7200);
    timer1.enableIt(Timer::IT::Update);
    timer1.bindCb(Timer::IT::Update, [&NecEnc](){
        NecEnc.tick();
    });

    auto TrigGpioA = Gpio(GPIOA, Pin::_4);
    auto TrigExtiCHA = ExtiChannel(TrigGpioA, 1, 2, ExtiChannel::Trigger::RisingFalling);
    auto CapA = CaptureChannelExti(TrigExtiCHA, TrigGpioA);
    CapA.init();

    CapA.bindCb([&CapA, &log](){
        static auto filter = BurrFilter_t<real_t>(real_t(0.9), real_t(0.1), 2);
        real_t duty = CapA.getDuty();
        can1.write(CanMsg(0x1, (uint8_t *)&duty, 4));
        log.println(filter.update(CapA.getDuty()));
        // log.println("cdscsd", real_t(3)/real_t(2));
    });

    can1.init(Can1::BaudRate::Mbps1);
    can1.enableHwReTransmit(false);

    while(true){
        led = 0.1 + 0.1 * sin(t * 6);
        io = !io;
        delay(10);
        NecEnc.setCode(int(9+8*sin(4*t)),18);
        reCalculateTime();
    }
}