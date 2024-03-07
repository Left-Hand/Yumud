#include "timer1.hpp"

#ifdef HAVE_TIM1
Timer1 timer1;

__interrupt void TIM1_BRK_IRQHandler(void){
    if(TIM_GetITStatus(TIM1, TIM_IT_Break)){

        TIM_ClearITPendingBit(TIM1, TIM_IT_Break);
    }
}
__interrupt void TIM1_UP_IRQHandler(void){
    if(TIM_GetITStatus(TIM1, TIM_IT_Update)){

        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
}
__interrupt void TIM1_TRG_COM_IRQHandler(void){
    if(TIM_GetITStatus(TIM1, TIM_IT_Trigger)){

        TIM_ClearITPendingBit(TIM1, TIM_IT_Trigger);
    }else if(TIM_GetITStatus(TIM1, TIM_IT_COM)){

        TIM_ClearITPendingBit(TIM1, TIM_IT_COM);
    }
}

__interrupt void TIM1_CC_IRQHandler(void){
    if(TIM_GetITStatus(TIM1, TIM_IT_CC1)){

        TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
    }else if(TIM_GetITStatus(TIM1, TIM_IT_CC2)){

        TIM_ClearITPendingBit(TIM1, TIM_IT_CC2);
    }else if(TIM_GetITStatus(TIM1, TIM_IT_CC3)){

        TIM_ClearITPendingBit(TIM1, TIM_IT_CC3);
    }else if(TIM_GetITStatus(TIM1, TIM_IT_CC4)){

        TIM_ClearITPendingBit(TIM1, TIM_IT_CC4);
    }
}

#endif