#ifndef __TM8211_HPP__

#define __TM8211_HPP__

#include "../bus/i2s/i2sSw.hpp"

class TM8211{
private:
    BusDrv busdrv;
    uint16_t ldata;
    uint16_t rdata;

    float Hvolt = 3.3 * 0.75;
    float Lvolt = 3.3 * 0.25;
    uint16_t VoltageToData(const float & volt){
        float Mvolt = (Hvolt + Lvolt)/2;
        uint16_t ret;
        if(volt > Mvolt){
            float deltaVolt = MIN(volt - Mvolt, Hvolt - Mvolt);
            float Scale = deltaVolt / (Hvolt - Mvolt);
            ret = Scale * 0x7FFF;
        }else{
            float deltaVolt = MAX(volt - Mvolt, Lvolt - Mvolt);
            float Scale = deltaVolt / (Lvolt - Mvolt);
            ret = 0xFFFF - Scale * 0x7FFF;
        }
        return ret;
    }
public:
    TM8211(BusDrv & _busdrv):busdrv(_busdrv){;}
    void write(const uint32_t & data){
        busdrv.write(data);
    }

    void setChData(const uint8_t & index,const uint16_t & data){
        if(index) rdata = data;
        else ldata = data;

        write((ldata << 16) | rdata);
    }

    void setChVoltage(const uint8_t & index, const float & volt){
        setChData(index, VoltageToData(volt));
    }

    void setVoltage(const float & lvolt, const float & rvolt){
        ldata = VoltageToData(lvolt);
        rdata = VoltageToData(rvolt);

        write((ldata << 16) | rdata);
    }
};
#endif