#ifndef __TM8211_HPP__

#define __TM8211_HPP__

#include "../bus/bus_inc.h"
#include "../types/real.hpp"

class TM8211{
    public:
        int16_t ldata;
    int16_t rdata;
private:
    I2sDrv busdrv;


    real_t voltH;
    real_t voltL;
    real_t voltComm;
    real_t voltDiff_2;

    __fast_inline int16_t VoltageToData(real_t volt){
        volt = CLAMP(volt, voltL, voltH);
        real_t k = ((volt - voltComm) / voltDiff_2);
        return (int16_t)(k * 0x0fff);
    }
public:
    TM8211(I2sDrv & _busdrv):busdrv(_busdrv){
        setRail(real_t(3.3 * 0.25f), real_t(3.3 * 0.75f)); 
    }

    void setRail(const real_t & _voltL, const real_t & _voltH){
        voltL = _voltL;
        voltH = _voltH;
        voltComm = (voltL + voltH) / 2;
        voltDiff_2 = (voltH - voltL) / 2;
    }

    void write(const uint32_t & data){
        busdrv.write(data);
    }

    void setChData(const uint8_t & index,const uint16_t & data){
        if(index) rdata = data;
        else ldata = data;

        write((ldata << 16) | rdata);
    }

    void setChVoltage(const uint8_t & index, const real_t & volt){
        setChData(index, VoltageToData(volt));
    }

    void setVoltage(const real_t & lvolt, const real_t & rvolt){
        ldata = VoltageToData(lvolt);
        rdata = VoltageToData(rvolt);

        write((ldata << 16) | rdata);
    }
};
#endif