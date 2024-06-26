#ifndef __OPA_HPP__

#define __OPA_HPP__

#include "sys/platform.h"

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port.hpp"

class Opa{
protected:
    uint8_t opa_index;
    uint8_t psel;
    uint8_t nsel;
    uint8_t osel;

    Gpio & getPosPin(){
        switch(opa_index){
            #ifdef HAVE_OPA1
            case 1:
                switch(psel){
                    case 0:
                        return OPA1_P0_Gpio;
                    case 1:
                        return OPA1_P1_Gpio;
                }
            #endif
            #ifdef HAVE_OPA2
            case 2:
                switch(psel){
                    case 0:
                        return OPA2_P0_Gpio;
                    case 1:
                        return OPA2_P1_Gpio;
                }
            #endif
            default:
                return GpioNull;
        }
    }

    Gpio & getNegPin(){
        switch(opa_index){
            #ifdef HAVE_OPA1
            case 1:
                switch(psel){
                    case 0:
                        return OPA1_N0_Gpio;
                    case 1:
                        return OPA1_N1_Gpio;
                }
            #endif
            #ifdef HAVE_OPA2
            case 2:
                switch(psel){
                    case 0:
                        return OPA2_N0_Gpio;
                    case 1:
                        return OPA2_N1_Gpio;
                }
            #endif
            default:
                return GpioNull;
        }
    }


    Gpio & getOutPin(){
        switch(opa_index){
            #ifdef HAVE_OPA1
            case 1:
                switch(psel){
                    case 0:
                        return OPA1_O0_Gpio;
                    case 1:
                        return OPA1_O1_Gpio;
                }
            #endif
            #ifdef HAVE_OPA2
            case 2:
                switch(psel){
                    case 0:
                        return OPA2_O0_Gpio;
                    case 1:
                        return OPA2_O1_Gpio;
                }
            #endif
            default:
                return GpioNull;
        }
    }

public:
    Opa(const uint8_t & _opa_index):opa_index( _opa_index ){;}

    void init( const uint8_t & sel){
        getNegPin().inana();
        getPosPin().inana();

        OPA_InitTypeDef OPA_InitStructure;
        OPA_InitStructure.OPA_NUM = CLAMP((OPA_Num_TypeDef)(OPA1 + (opa_index - 1)), OPA1, OPA4);
        OPA_InitStructure.PSEL = (OPA_PSEL_TypeDef)sel;
        OPA_InitStructure.NSEL = (OPA_NSEL_TypeDef)sel;
        OPA_InitStructure.Mode = (OPA_Mode_TypeDef)sel;
        OPA_Init(&OPA_InitStructure);

        OPA_Cmd(OPA_InitStructure.OPA_NUM, ENABLE);
    }
};

extern Opa opa1;
extern Opa opa2;

#endif