#ifndef __FWWB_SIMPLE_DEMODEM_HPP__
#define __FWWB_SIMPLE_DEMODEM_HPP__

namespace FWWB{
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
};

#endif