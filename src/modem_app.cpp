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
void modem_app(){
    // while(1);
    uart2.init(115200*4, Uart::Mode::TxOnly);

    Printer & log = uart2;
    log.setEps(4);
    timer4.init(38000);

    auto tim4ch3 = timer4.getChannel(TimerOC::Channel::CH3);
    tim4ch3.init();

    auto pwm_out = PwmChannel(tim4ch3);
    pwm_out = real_t(0.1);

    auto NecEnc = SimpleModem(pwm_out);
    NecEnc.setCode(1,2);
    timer1.init(1600);
    timer1.enableIt(Timer::IT::Update);
    timer1.bindCb(Timer::IT::Update, [&NecEnc](){
        NecEnc.tick();
    });


    while(true){

        delay(100);
        log.println(",");
        // log.println(NecEnc.isIdle());
        // NecEnc.emit(0x55, 0xAA);
    }
}