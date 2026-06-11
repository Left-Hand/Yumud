#pragma once

#include <cstdint>


// https://www.cnblogs.com/foxclever/p/6533907.html

namespace ymd::drivers{

struct CalibrateCoeffs{
    uint16_t senst1;        //C1压力灵敏度
    uint16_t offt1;         //C2压力补偿值
    uint16_t tcs;           //C3压力灵敏度温度系数
    uint16_t tco;           //C4压力补偿温度系数
    uint16_t tref;          //C5参考温度
    uint16_t tempsens;      //C6温度传感器温度系数

    constexpr void get(
        const uint16_t digital_pressure_value,
        const uint16_t digital_temperature_value
    ){
        int32_t dT=digital_temperature_value-tref*256;
        int32_t temp=(int32_t)(2000+dT*tempsens/(1 << 23));
        int64_t off=(int64_t)(offt1*(1 << 17)+(tco*dT)/(1 << 6));
        int64_t sens=(int64_t)(senst1*(1 << 16)+(tcs*dT)/(1 << 7));
        [[maybe_unused]] int32_t pres=(int32_t)((digital_pressure_value*sens/(1 << 21)-off)/(1 << 15));

        
        /*对温度和压力进行二阶修正*/
        int64_t ti=0;
        int64_t offi=0;
        int64_t sensi=0;
        int64_t off2=0;
        int64_t sens2=0; 

        if(temp<2000){
            ti=(int64_t)((int64_t(11)*dT*dT) >> 35);
            offi=(int64_t)(31*(temp-2000)*(temp-2000)/(1 << 3));
            sensi=(int64_t)(63*(temp-2000)*(temp-2000)/(1 << 5));

            off2=off-offi;
            sens2=sens-sensi;

            temp=temp-(int32_t)ti;
            pres=(int32_t)((digital_pressure_value*sens2/(1 << 21)-off2)/(1 << 15));
        }

        

        // if((-4000<=temp)&&(temp<=8500)){
        //     *pTemp=(float)temp/100.0;
        // }

        // if((1000<=pres)&&(pres<=120000)){
        //     *pPres=(float)pres/100.0;
        // }
    }

};

}