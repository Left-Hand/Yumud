#ifndef __TJC_HPP__

#define __TJC_HPP__

#include "sys/string/string.hpp"

class HMI{

};

class TJC:public HMI{
public:
    Uart & uart;

protected:

public:
    TJC(Uart & _uart):uart(_uart){;}
    void init(){
        uart.init(115200);
        uart.setSpace("");
    }

    void test(){
        static uint8_t cnt = 0;
        uart.print("add 1,2,", cnt++, "\xff\xff\xff");
        // PrintfDebugUart("add 7,0,%d\xff\xff\xff",a[i]);
    }

    class Ctrl{
    protected:
        TJC & instance;
        uint8_t ctrl_id;

        Ctrl(TJC & _instance, const uint8_t & _ctrl_id):instance(_instance), ctrl_id(_ctrl_id){;}
    };

    class Waveform;

    class WaveWindow:public Ctrl{
    public:

        friend class Waveform;

        WaveWindow(TJC & _instance, const uint8_t & _ctrl_id):Ctrl(_instance, _ctrl_id){;}

        void addChData(const uint8_t & channel_id,const uint8_t & data){
            instance.uart.print("add ", ctrl_id, ',', channel_id, ',', data, "\xff\xff\xff");
        }

        Waveform getChannel(const uint8_t & channel_id){return Waveform(*this, channel_id);}
    };

    class Waveform{
    protected:
        WaveWindow & parent;
        uint8_t channel_id;


    public:
        Waveform(WaveWindow & _parent, const uint8_t & _channel_id):parent(_parent), channel_id(_channel_id){;}

        void addData(const uint8_t & data){
            parent.addChData(channel_id, data);
        }
    };

};


#endif