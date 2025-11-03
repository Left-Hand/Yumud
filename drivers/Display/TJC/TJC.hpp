#pragma once

#include "cstdint.h"
#include <array>

namespace ymd::drivers{


class TJC_Phy{
public:
    TJC_Phy(Uart & _uart):uart_(_uart){;}


    // template<typename >
    void print(const StringView str){
        uart_.print(str);
        print_end();
    }


private:

    Uart & uart_;

    void print_end(){
        constexpr std::array tail = {char(0xff), char(0xff), char(0xff)};
        uart_.write(tail, tail.size());
    }
};


struct TJC_Scene{
public:
    struct Config{
        StringView name;
    };
private:

    StringView name_;
}

struct TJC_Controls{
    class Ctrl{
    protected:
        TJC & inst_;
        uint8_t ctrl_id;

        Ctrl(TJC & inst, const uint8_t _ctrl_id):inst_(inst), ctrl_id(_ctrl_id){;}
    };

    class Waveform;

    class WaveWindow:public Ctrl{
    public:

        friend class Waveform;

        WaveWindow(TJC & inst, const uint8_t _ctrl_id):Ctrl(inst, _ctrl_id){;}

        void add_ch_data(const uint8_t channel_id,const uint8_t data){
            auto str = StringView("add " + StringView(ctrl_id) + ',' + StringView(channel_id) + ',' + StringView(data));
            inst_.print(str);
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

        void add_data(const auto & data){
            real_t value = data;
            auto ratio = range.invlerp(value);
            parent.add_ch_data(channel_id, uint8_t((255 * ratio)));
        }
    };

    class Label{
        StringView tag_name;
    };

    template<typename ... Args>
    static void serial_print(Args && ...args){

    }

    static void set_label_text(const Label & label, const StringView text){
        serial_print(StringView(scene_name + '.' + tag_name + ".txt=\"" + text + '\"'));
    }
}

}