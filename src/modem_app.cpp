#include "apps.h"

#include "src/device/CommonIO/Led/rgbLed.hpp"
#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"
#include "fwwb_compents/fwwb_inc.h"

using Color = Color_t<real_t>;
Printer & logger = uart2;

namespace FWWB{
class TargetStation:public CanStation{
protected:
    using Command = CanStation::Command;
    PanelUnit & unit0;
    PanelUnit & unit1;

    bool Shotted(){
        return unit0.isShotted() || unit1.isShotted();
    }

    bool Outline(){
        return unit0.isOutline() || unit1.isOutline();
    }

    bool Died(){
        return unit0.isDied() && unit1.isDied();
    }

    void OutLineNotify(){
        sendCommand(Command::OUTBOUND);
    }

    void HpNotify(){
        uint8_t buf[2] = {unit0.hp, unit1.hp};
        sendCommand(Command::HP, VAR_AND_SIZE(buf));
    }

    void HpNotified(const CanMsg & msg){
        unit0.hp = msg.getData()[0];
        unit1.hp = msg.getData()[1];
    }

    void runMachine() override{
        CanStation::runMachine();
        switch(sm){
        case StateMachine::ACTIVE:

            if(Outline()){
                OutLineNotify();
                sm = StateMachine::INACTIVE;
            }else if(Shotted()){
                HpNotify();
                if(Died()){
                    sm = StateMachine::INACTIVE;
                }
            }

            unit0.run();
            unit1.run();
            break;
        default:
            break;
        }
    }
    void parseCommand(const CanCommand & cmd, const CanMsg & msg) override{
        CanStation::parseCommand(cmd, msg);
        switch(cmd){
        case CanCommand::HP:
            if(msg.isRemote())
                HpNotify();
            else
                HpNotified(msg);
            break;

        default:
            break;
        }
    }
public:

    virtual void init(){
        unit0.init();
        unit1.init();
    }

    TargetStation(Can & _can, PanelUnit & _unit0, PanelUnit & _unit1) : CanStation(_can), unit0(_unit0), unit1(_unit1){;}
};
};


//Laser PB8 TM4CH3
//TrigIn PA4
void modem_app(){
    using namespace FWWB;

    uart2.init(115200*4);
    logger.setEps(4);

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

    auto trigGpioA = Gpio(GPIOA, Pin::_4);
    auto trigExtiCHA = ExtiChannel(trigGpioA, 1, 0, ExtiChannel::Trigger::RisingFalling);
    auto capA = CaptureChannelExti(trigExtiCHA, trigGpioA);
    auto demodemA = SimpleDeModem(capA, mo_freq);
    auto panelTargetA = PanelTarget(demodemA, 2, 0);

    auto ledA = WS2812(Gpio(GPIOD, Pin::_0));
    auto panelLedA = PanelLed(ledA);


    auto trigGpioB = Gpio(GPIOA, Pin::_0);
    auto trigExtiCHB = ExtiChannel(trigGpioB, 1, 0, ExtiChannel::Trigger::RisingFalling);
    auto capB = CaptureChannelExti(trigExtiCHB, trigGpioB);
    auto demodemB = SimpleDeModem(capB, mo_freq);
    auto panelTargetB = PanelTarget(demodemB, 2, 1);

    auto ledB = WS2812(Gpio(GPIOD, Pin::_1));
    auto panelLedB = PanelLed(ledB);

    auto panelUnitA = PanelUnit(panelTargetA, panelLedA);
    auto panelUnitB = PanelUnit(panelTargetB, panelLedB);

    auto panel = TargetStation(can1, panelUnitA, panelUnitB);
    panel.init();
    // auto led0 = AW9523::RgbLed(aw, Pin::_2, Pin::_3, Pin::_4);
    // led0.init();
    // led0.setBrightness(real_t(0));
    // led0 = Color(real_t(1), real_t(1), real_t(1));
    // auto led = AW9523::RgbLed(aw, Pin::_5, Pin::_6, Pin::_7);
    // led.setBrightness(real_t(0.2));

    can1.init(Can::BaudRate::Mbps1);
    can1.enableHwReTransmit(false);

    if(Sys::getChipId() == 6002098052848856013)
    while(true){
        // awLed = 0.1 + 0.1 * sin(t * 6);
        // delay(40);
        // log.println();
        // delay(1);
        // awio = !awio;
        // log.println(bool(awio));
        panel.run();
        if(modem.isIdle()) modem.sendCode(2);
        Sys::reCalculateTime();
        // log.println(demodem.getCode());
        // log.println(targ1.isShotted());
        // log.println(targ1.isShotted());
    }else{
        while(true){
            can1.write(CanMsg((uint16_t)((uint8_t)CanCommand::RST << 4 | 0), false));
            // can1.write(CanMsg((uint16_t)((uint8_t)CanCommand::HP << 4 | 0), {2}));
            ledA = Color(real_t(0.01), real_t(1), real_t(1));
            delay(2000);
        }
    }
}