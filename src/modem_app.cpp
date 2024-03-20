#include "apps.h"



class SimpleModem{
protected:
    PwmChannel & pwm_out;
    uint8_t code;
    int8_t cnt = -1;

    void setOut(const bool & val){
        if(val) pwm_out = real_t(0.1);
        else pwm_out = real_t(0);
    }

    void sendBit(const uint8_t cnt, const bool & bit){
        if(bit){
            setOut(cnt < 3);
        }else{
            setOut(cnt < 1);
        }
    }
public:
    SimpleModem(PwmChannel & _pwm_out):pwm_out(_pwm_out){;}
    void tick(){
        if(cnt >= 0){
            if(cnt < 4) setOut(true);
            else if(cnt < 8) setOut(false);
            else if(cnt < 12){
                sendBit(cnt - 8, code & 0b1);
            }else if(cnt < 16){
                sendBit(cnt - 12, code & 0b10);
            }else if(cnt < 20){
                setOut(true);
            }
            else{
                cnt = -1;
                return;
            }
            cnt++;
        }
    }

    bool isIdle(){
        return (cnt == -1);
    }

    void sendCode(const uint8_t & _code){
        code = _code;
        cnt = 0;
    }
};

class SimpleDeModem{
protected:
    CaptureChannelExti & capturer;
    std::function<void(void)> cb;
    uint16_t max_period;
    struct Code{
        union{
            struct{
                uint8_t bit0:1;
                uint8_t bit1:1;
            };
            uint8_t data = 0;
        };
    };
    
    Code code;
    Code last_code;


    enum class Progs:uint8_t{
        IDLE = 0,
        BIT0,
        BIT1,
    };

    Progs prog = Progs::IDLE;

    void update(){
        switch (prog){
        case Progs::IDLE:
            if(capturer.getPeriodUs() > max_period){
                prog = Progs::BIT0;
            }
            break;
        case Progs::BIT0:
            if(capturer.getPeriodUs() > max_period){
                prog = Progs::IDLE;
                break;
            }
            code.bit0 = (capturer.getDuty() > real_t(0.5));
            prog = Progs::BIT1;
            break;
        case Progs::BIT1:
            if(capturer.getPeriodUs() > max_period){
                prog = Progs::IDLE;
                break;
            }
            code.bit1 = (capturer.getDuty() > real_t(0.5));
            prog = Progs::IDLE;

        default:
            last_code = code;
            cb();
            break;
        }
    };
public:
    SimpleDeModem(CaptureChannelExti & _capturer,const uint16_t & _freq):capturer(_capturer), max_period(1000000 / _freq * 3 / 2 * 4){;}

    void init(){
        capturer.bindCb([this](){this->update();});
    }

    void bindCb(const std::function<void(void)>& _cb){
        cb = _cb;
    }

    uint8_t getCode(){
        return last_code.data;
    }

};

class PanelTarget{
protected:
// public:
    SimpleDeModem & demodem;
    std::function<void(void)> cb;
    const uint8_t species;
    const uint8_t index;
    volatile uint8_t combo_cnt = 0;
    static constexpr uint8_t combo_thr = 3;
    volatile bool shotted = false;

    void update(const uint8_t & code){
        if(!shotted){
            if(code == species){
                combo_cnt++;
                if(combo_cnt >= combo_thr){
                    shotted = true;
                    combo_cnt = 0;
                }
            }else{
                combo_cnt = 0;
            }
        }
    }
public:
    PanelTarget(SimpleDeModem & _demodem, const uint8_t & _species, const uint8_t & _index):demodem(_demodem), species(_species), index(_index){;}

    void init(){
        demodem.init();
        demodem.bindCb([this](){
            this->update(demodem.getCode());
        });
    }

    bool isShotted(){
        volatile bool _shotted = shotted;
        shotted = false;
        return _shotted;
    }

    void bindCb(const std::function<void(void)>& _cb){
        cb = _cb;
    }
};

class PanelLed{
protected:
    AwLed & led;
public:
    PanelLed(AwLed & _led) : led(_led){;}


    void init(){
        led.init();
    }

    PanelLed & operator = (const real_t & _duty){
        led = _duty;
        return *this;
    }
};

class PanelProtocol{
    // Printer
};

class PanelUnit{
protected:
    enum class StateMachine:uint8_t{
        IDLE, SHOTTED, RECOVER
    };
    StateMachine sm = StateMachine::IDLE;
public:
    PanelTarget & target;
    PanelLed & led;

    int32_t shot_tick = -10000;
    uint32_t besta_time = 1600;
    uint32_t transtit_time = 3400;

public:
    PanelUnit(PanelTarget & _target, PanelLed & _led) : target(_target), led(_led){;}

    void init(){
        target.init();
        led.init();
    }

    void run(){
        switch(sm){
            case StateMachine::IDLE:
                led = 0.1 + 0.1 * sin(4 * t);
                if(target.isShotted()){
                    sm = StateMachine::SHOTTED;
                    shot_tick = millis();
                }
                break;
            case StateMachine::SHOTTED:
                led = 0.1 + 0.1 * sign(sin(48*t));
                if(millis() - shot_tick > besta_time){
                    sm = StateMachine::RECOVER;
                }
                break;
            case StateMachine::RECOVER:
                led = 0.1 + 0.1 * sign(sin(24*t));
                if(millis() - shot_tick > transtit_time + besta_time){
                    target.isShotted();
                    sm = StateMachine::IDLE;
                }
                break;
        }
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
    aw.setLedCurrentLimit(AW9523::CurrentLimit::Low);
    // aw.enableLedMode(Pin::_8, false);
    // aw.setModeByIndex(8, PinMode::OutPP);

    auto awio = GpioVirtual(&aw, Pin::_0);
    awio.OutPP();

    auto awLed = AwLed(aw, Pin::_0);
    awLed.init();

    timer4.init(38000);
    auto tim4ch3 = timer4.getChannel(TimerOC::Channel::CH3);
    tim4ch3.init();

    auto pwm_out = PwmChannel(tim4ch3);

    auto modem = SimpleModem(pwm_out);

    constexpr int mo_freq = 3600;
    timer1.init(mo_freq);
    timer1.enableIt(Timer::IT::Update);
    timer1.bindCb(Timer::IT::Update, [&modem](){
        modem.tick();
    });

    auto TrigGpioA = Gpio(GPIOA, Pin::_4);
    auto TrigExtiCHA = ExtiChannel(TrigGpioA, 1, 2, ExtiChannel::Trigger::RisingFalling);
    auto CapA = CaptureChannelExti(TrigExtiCHA, TrigGpioA);
    CapA.init();

    auto demodem = SimpleDeModem(CapA, mo_freq);
    auto panel1_targ = PanelTarget(demodem, 2, 0);
    auto panel1_led = PanelLed(awLed);

    auto panel1 = PanelUnit(panel1_targ, panel1_led);
    panel1.init();
    // targ1.init();
    
    // targ1.bindCb([&log, &targ1](){
    // });

    can1.init(Can1::BaudRate::Mbps1);
    can1.enableHwReTransmit(false);

    uint16_t cnt = 0;
    while(true){
        // awLed = 0.1 + 0.1 * sin(t * 6);
        // delay(40);
        // log.println();
        // delay(1);
        // awio = !awio;
        // log.println(bool(awio));
        panel1.run();
        log.println(panel1.shot_tick);
        if(modem.isIdle()) modem.sendCode(2);
        reCalculateTime();
        // log.println(demodem.getCode());
        // log.println(targ1.isShotted());
        // log.println(targ1.isShotted());
    }
}