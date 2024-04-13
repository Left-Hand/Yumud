#ifndef __FWWB_SIMPLE_DEMODEM_HPP__
#define __FWWB_SIMPLE_DEMODEM_HPP__

namespace FWWB{
class SimpleDeModem{
protected:
    CaptureChannelExti & capturer;
    std::function<void(void)> cb;
    Range_t<uint16_t> bit_period;
    static constexpr uint16_t bit_tolerance = 80;
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
            if(capturer.getPeriodUs() > bit_period.end){
                prog = Progs::BIT0;
            }
            break;
        case Progs::BIT0:
            if(!bit_period.has_value(capturer.getPeriodUs())){
                prog = Progs::IDLE;
                break;
            }
            code.bit0 = (capturer.getPulseUs() > capturer.getPeriodUs() / 2);
            prog = Progs::BIT1;
            break;
        case Progs::BIT1:
            if(!bit_period.has_value(capturer.getPeriodUs())){
                prog = Progs::IDLE;
                break;
            }
            code.bit1 = (capturer.getPulseUs() > capturer.getPeriodUs() / 2);
            prog = Progs::IDLE;

        default:
            last_code = code;
            cb();
            break;
        }
    };
public:
    SimpleDeModem(CaptureChannelExti & _capturer,const uint16_t & _freq):capturer(_capturer),
            bit_period(1000000 / _freq * 4 - bit_tolerance, 1000000 / _freq * 4 + bit_tolerance){;}

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