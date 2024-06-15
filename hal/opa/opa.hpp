#ifndef __OPA_HPP__

#define __OPA_HPP__

#include "sys/platform.h"
#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port.hpp"

class Opa{
protected:
    uint8_t opa_num;
    uint8_t psel;
    uint8_t nsel;
    uint8_t osel;

    Gpio & getPosPin(){
        switch(opa_num){
            #ifdef HAVE_OPA1
            case 1:
                switch(psel){
                    case 0:
                        return portB[(Pin)OPA1_P0_Pin];
                    case 1:
                        return portB[(Pin)OPA1_P1_Pin];
                    default:
                        break;
                }
                break;
            #endif
            #ifdef HAVE_OPA2
            case 2:
                switch(psel){
                    case 0:
                        return portB[(Pin)OPA2_P0_Pin];
                    case 1:
                        return portA[(Pin)OPA2_P1_Pin];
                    default:
                        break;
                }
                break;
            #endif
            default:
                break;
        }
        // return portA[Pin::None];
        return GpioNull;
    }

    Gpio & getNegPin(){
        switch(opa_num){
            #ifdef HAVE_OPA1
            case 1:
                switch(psel){
                    case 0:
                        return portB[(Pin)OPA1_N0_Pin];
                    case 1:
                        return portA[(Pin)OPA1_N1_Pin];
                    default:
                        break;
                }
                break;
            #endif

            #ifdef HAVE_OPA2
            case 2:
                switch(psel){
                    case 0:
                        return portB[(Pin)OPA2_N0_Pin];
                    case 1:
                        return portA[(Pin)OPA2_N1_Pin];
                    default:
                        break;
                }
                break;
            #endif
            default:
                break;
        }
        // return portA[Pin::None];
        return GpioNull;
    }

public:
    Opa(const uint8_t & _opa_num):opa_num( _opa_num ){;}

    void init( const uint8_t & sel){
        getNegPin().InAnalog();
        getPosPin().InAnalog();
        OPA_InitTypeDef OPA_InitStructure;
        OPA_InitStructure.OPA_NUM = CLAMP((OPA_Num_TypeDef)(OPA1 + (opa_num - 1)), OPA1, OPA4);
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