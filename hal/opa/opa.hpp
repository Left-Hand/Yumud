#pragma once

#include "sys/core/platform.h"

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port.hpp"


namespace yumud{
class Opa{
protected:
    uint8_t opa_index;
    uint8_t psel;
    uint8_t nsel;
    uint8_t osel;

    Gpio & getPosPin(){
        switch(opa_index){
            #ifdef ENABLE_OPA1
            case 1:
                switch(psel){
                    case 0:
                        return OPA1_P0_GPIO;
                    case 1:
                        return OPA1_P1_GPIO;
                }
            #endif
            #ifdef ENABLE_OPA2
            case 2:
                switch(psel){
                    case 0:
                        return OPA2_P0_GPIO;
                    case 1:
                        return OPA2_P1_GPIO;
                }
            #endif
            default:
                return GpioNull;
        }
    }

    Gpio & getNegPin(){
        switch(opa_index){
            #ifdef ENABLE_OPA1
            case 1:
                switch(psel){
                    case 0:
                        return OPA1_N0_GPIO;
                    case 1:
                        return OPA1_N1_GPIO;
                }
            #endif
            #ifdef ENABLE_OPA2
            case 2:
                switch(psel){
                    case 0:
                        return OPA2_N0_GPIO;
                    case 1:
                        return OPA2_N1_GPIO;
                }
            #endif
            default:
                return GpioNull;
        }
    }


    Gpio & getOutPin(){
        switch(opa_index){
            #ifdef ENABLE_OPA1
            case 1:
                switch(psel){
                    case 0:
                        return OPA1_O0_GPIO;
                    case 1:
                        return OPA1_O1_GPIO;
                }
            #endif
            #ifdef ENABLE_OPA2
            case 2:
                switch(psel){
                    case 0:
                        return OPA2_O0_GPIO;
                    case 1:
                        return OPA2_O1_GPIO;
                }
            #endif
            default:
                return GpioNull;
        }
    }

public:
    Opa(const uint8_t _opa_index):opa_index( _opa_index ){;}

    void init( const uint8_t sel){
        getNegPin().inana();
        getPosPin().inana();

        OPA_InitTypeDef OPA_InitStructure;
        OPA_InitStructure.OPA_NUM = (OPA_Num_TypeDef)CLAMP((int)(OPA1 + (opa_index - 1)), (int)OPA1, (int)OPA4);
        OPA_InitStructure.PSEL = (OPA_PSEL_TypeDef)sel;
        OPA_InitStructure.NSEL = (OPA_NSEL_TypeDef)sel;
        OPA_InitStructure.Mode = (OPA_Mode_TypeDef)sel;
        OPA_Init(&OPA_InitStructure);

        OPA_Cmd(OPA_InitStructure.OPA_NUM, ENABLE);
    }
};

#ifdef ENABLE_OPA1
static inline Opa opa1;
#endif

#ifdef ENABLE_OPA2
static inline Opa opa2;
#endif

}