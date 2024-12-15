#pragma once



#ifdef HDW_SXX32

    #ifdef __cplusplus
    extern "C"{
    #endif


    __interrupt void NMI_Handler(void);
    __interrupt void HardFault_Handler(void);



    #ifdef __cplusplus
    }
    #endif

#endif
