#include "apps.h"

#include "src/device/CommonIO/Led/rgbLed.hpp"

using Color = Color_t<real_t>;


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
        capturer.init();
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
    volatile bool outline = false;

    void update(const uint8_t & code){
        if(!shotted){
            if(code == species){
                combo_cnt++;
                if(combo_cnt >= combo_thr){
                    shotted = true;
                    combo_cnt = 0;
                }
            }else{
                if(code == 3){
                    outline = true;
                }
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

    bool isOutline(){
        return outline;
    }

    void bindCb(const std::function<void(void)>& _cb){
        cb = _cb;
    }
};

class PanelLed{
public:
    enum class Method:uint8_t{
        Sine = 0,
        Saw,
    };
protected:
    RgbLedConcept<true> & led;

    Color color_a;
    Color color_b;
    uint16_t period;

    Method method;
public:
    PanelLed(RgbLedConcept<true> & _led) : led(_led){;}

    void init(){
        led.init();
    }

    void setPeriod(const uint16_t & _period){
        period = _period;
    }

    void setTranstit(const Color & _color_a, const Color & _color_b, const Method & _method){
        color_a = _color_a;
        color_b = _color_b;
        method = _method;
    }

    void run(){
        real_t ratio;
        real_t _t = t / (real_t(period) / 1000);
        switch(method){
        case Method::Saw:
            ratio = frac(_t);
            break;
        case Method::Sine:
            ratio = 0.5 * sin(_t * TAU) + 0.5;
        }

        Color color_mux = color_a.linear_interpolate(color_b, ratio);
        led = color_mux;
    }

    PanelLed & operator = (const Color & color){
        led = color;
        return *this;
    }
};

enum class CanCommand:uint8_t{
    INACTIVE = 0,
    ACTIVE,
    HP,
    MP,
    WEIGHT,

    RST = 0x70,
    POWER_ON = 0x7e,
    OUTBOUND = 0x7f
};


class PanelUnit{
protected:
    enum class StateMachine:uint8_t{
        NONE, IDLE, SHOTTED, RECOVER, DIED
    };
    StateMachine sm = StateMachine::NONE;
public:
    PanelTarget & target;
    PanelLed & led;

    int32_t shot_tick = -10000;
    static constexpr uint32_t besta_time = 1600;
    static constexpr uint32_t transtit_time = 2400;
    static constexpr uint8_t max_hp = 2;
    bool shot = false;

    Color from_hp_to_color(const uint8_t & _hp){
        return Color::from_hsv(real_t(MAX(_hp, 0)) / max_hp * real_t(0.66), real_t(1), real_t(1));
    }

    void sw(const StateMachine & _sm){
        switch(_sm){
        case StateMachine::IDLE:
            led.setPeriod(1200);
            {
                Color hp_color = from_hp_to_color(hp);
                led.setTranstit(hp_color, Color(), PanelLed::Method::Sine);
            }
            break;
        case StateMachine::RECOVER:
            led.setPeriod(400);
            goto saw_transtit;
        case StateMachine::SHOTTED:
            led.setPeriod(200);

        saw_transtit:
            led.setTranstit(from_hp_to_color(hp + 1), from_hp_to_color(hp), PanelLed::Method::Saw);
            sm = StateMachine::SHOTTED;
            break;
        case StateMachine::DIED:
            {
                Color hp_color = from_hp_to_color(0);
                led.setTranstit(hp_color, hp_color, PanelLed::Method::Sine);
            }
        default:
            break;
        }

        sm = _sm;
    }
public:
    uint8_t hp = max_hp;

    PanelUnit(PanelTarget & _target, PanelLed & _led) : target(_target), led(_led){;}

    void init(){
        target.init();
        led.init();
    }

    void run(){
        switch(sm){
        case StateMachine::NONE:
            sw(StateMachine::IDLE);
        case StateMachine::IDLE:
            if(target.isOutline()){
                hp = 0;
                sw(StateMachine::DIED);
            }else if(target.isShotted()){
                hp --;
                if(hp > 0) sw(StateMachine::SHOTTED);
                else sw(StateMachine::DIED);
                shot = true;
                shot_tick = millis();
            }
            break;
        case StateMachine::SHOTTED:
            if(millis() - shot_tick > besta_time){
                sw(StateMachine::RECOVER);
            }
            break;
        case StateMachine::RECOVER:
            if(millis() - shot_tick > transtit_time + besta_time){
                target.isShotted();
                sw(StateMachine::IDLE);
            }
        case StateMachine::DIED:
            break;
        }

        led.run();
    }

    bool isShotted(){
        auto _shot = shot;
        shot = false;
        return _shot;
    }

    bool isOutline(){
        return target.isOutline();
    }
};

#define VAR_AND_SIZE(x) x,sizeof(x)

class Panel{
protected:

    PanelUnit & unit;
    Can & can;

    uint8_t node_id = 0;
    using Command = CanCommand;

    volatile void sendCommand(const Command & command, const uint8_t *buf, const uint8_t len){
        can.write(CanMsg((uint16_t)((uint8_t)command << 4 | node_id), buf, len));
    }

    volatile void sendCommand(const Command & command){
        can.write(CanMsg((uint16_t)((uint8_t)command << 4 | node_id), {0}));
    }

    void OutLineNotify(){
        sendCommand(Command::OUTBOUND);
    }
    void HpNotify(const uint8_t & hp){
        uint8_t buf[1] = {hp};
        sendCommand(Command::HP, VAR_AND_SIZE(buf));
        uart2.println("HP", hp);
    }

    void PowerOnNotify(){
        sendCommand(Command::POWER_ON);
        uart2.println("Pw On");
    }


    enum class StateMachine:uint8_t{
        POWER_ON,
        INACTIVE,
        ACTIVE
    };

    volatile StateMachine sm = StateMachine::POWER_ON;

    bool Shotted(){
        return unit.isShotted();
    }

    bool Outline(){
        return unit.isOutline();
    }

    uint8_t getSubHp(){
        return unit.hp;
    }
public:
    Panel(PanelUnit & _unit, Can & _can) : unit(_unit), can(_can){;}

    void init(){
        unit.init();
    }

    void run(){
        if(can.available()){
            const CanMsg & msg = can.read();
            uint8_t id = msg.getId() & 0b1111;
            if(id == node_id){
                CanCommand cmd = (CanCommand)(msg.getId() >> 4);
                switch(cmd){
                    case CanCommand::POWER_ON:
                        sm = StateMachine::POWER_ON;
                        break;
                    case CanCommand::INACTIVE:
                        sm = StateMachine::INACTIVE;
                        break;
                    case CanCommand::ACTIVE:
                        sm = StateMachine::ACTIVE;
                        break;
                    case CanCommand::HP:
                        if(msg.isRemote()){
                            HpNotify(unit.hp);
                        }else{
                            unit.hp = msg.getData()[0];
                        }
                        break;
                    case CanCommand::RST:
                        Sys::Reset();
                    default:
                        break;
                }
            }
        }

        switch(sm){
        case StateMachine::POWER_ON:
            PowerOnNotify();
            sm = StateMachine::ACTIVE;
            break;

        case StateMachine::INACTIVE:
            break;

        case StateMachine::ACTIVE:

            if(Outline()){
                OutLineNotify();
                sm = StateMachine::INACTIVE;
            }else if(Shotted()){
                uint8_t subHp = getSubHp();
                HpNotify(subHp);
                if(subHp == 0){
                    sm = StateMachine::INACTIVE;
                }
            }

            unit.run();


            break;
        }
    }
};



//Laser PB8 TM4CH3
//TrigIn PA4
void modem_app(){

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

    auto demodem = SimpleDeModem(CapA, mo_freq);
    auto unit1_targ = PanelTarget(demodem, 2, 0);

    auto led = AW9523::RgbLed(aw, Pin::_2, Pin::_3, Pin::_4);
    led.setBrightness(real_t(0.1));
    auto unit1_led = PanelLed(led);

    auto unit1 = PanelUnit(unit1_targ, unit1_led);

    can1.init(Can::BaudRate::Mbps1);
    can1.enableHwReTransmit(false);

    auto panel = Panel(unit1, can1);
    panel.init();

    while(true){
        // awLed = 0.1 + 0.1 * sin(t * 6);
        // delay(40);
        // log.println();
        // delay(1);
        // awio = !awio;
        // log.println(bool(awio));
        panel.run();
        if(modem.isIdle()) modem.sendCode(3);
        reCalculateTime();
        // log.println(demodem.getCode());
        // log.println(targ1.isShotted());
        // log.println(targ1.isShotted());
    }
}