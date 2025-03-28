#pragma once

namespace ymd::drivers{
class HMI{

};

class TJC:public HMI{
private:
    void end(){
        constexpr char buf[3] = {char(0xff), char(0xff), char(0xff)};
        uart.write(buf, 3);
    }
protected:
    Uart & uart;

public:
    TJC(Uart & _uart):uart(_uart){;}
    void init(){
        uart.set_splitter("");
    }


    void print(const String & str){
        uart.print(str);
        end();
    }


    class Ctrl{
    protected:
        TJC & instance;
        uint8_t ctrl_id;

        Ctrl(TJC & _instance, const uint8_t _ctrl_id):instance(_instance), ctrl_id(_ctrl_id){;}
    };

    class Waveform;

    class WaveWindow:public Ctrl{
    public:

        friend class Waveform;

        WaveWindow(TJC & _instance, const uint8_t _ctrl_id):Ctrl(_instance, _ctrl_id){;}

        void addChData(const uint8_t channel_id,const uint8_t data){
            auto str = String("add " + String(ctrl_id) + ',' + String(channel_id) + ',' + String(data));
            instance.print(str);
            // DEBUG_PRINTLN(str);
        }

        // Waveform getChannel(const uint8_t channel_id){return Waveform(*this, channel_id);}
    };

    class Waveform{
    protected:
        WaveWindow & parent;
        const uint8_t channel_id;
        const Range range;
    public:
        Waveform(WaveWindow & _parent, const uint8_t _channel_id, Range && _range):parent(_parent), channel_id(_channel_id), range(_range){;}

        void addData(const auto & data){
            real_t value = data;
            auto ratio = range.invlerp(value);
            parent.addChData(channel_id, uint8_t((255 * ratio)));
        }
    };

    class Label{
    protected:
        TJC & instance;
        String scene_name;
        String tag_name;
        String text;
    public:
        Label(TJC & _instance, const String & _scene_name, const String & _tag_name):instance(_instance), scene_name(_scene_name), tag_name(_tag_name){;}


        void setValue(const real_t value){
            text = String(float(value), 2);
            setText(text);
        }

        void setText(const String & _text){
            instance.print(String(scene_name + '.' + tag_name + ".txt=\"" + _text + '\"'));
        }
    };

};

}