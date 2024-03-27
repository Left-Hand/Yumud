#ifndef __FWWB_SIMPLE_MODEM_HPP__
#define __FWWB_SIMPLE_MODEM_HPP__

namespace FWWB{
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
            }else{
                idle();
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

    void init(){
        pwm_out.init();
        idle();
    }

    void idle(){
        setOut(true);
        cnt = -1;
    }
};
};
#endif