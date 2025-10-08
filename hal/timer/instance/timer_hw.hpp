#pragma once

#include "hal/timer/timer.hpp"

namespace ymd::hal{

#ifdef ENABLE_TIM1
extern AdvancedTimer timer1;
#endif

#ifdef ENABLE_TIM2
extern GenericTimer timer2;
#endif

#ifdef ENABLE_TIM3
extern GenericTimer timer3;
#endif

#ifdef ENABLE_TIM4
extern GenericTimer timer4;
#endif

#ifdef ENABLE_TIM5
extern GenericTimer timer5;
#endif

#ifdef ENABLE_TIM6
extern BasicTimer timer6;
#endif

#ifdef ENABLE_TIM7
extern BasicTimer timer7;
#endif

#ifdef ENABLE_TIM8
extern AdvancedTimer timer8;
#endif

#ifdef ENABLE_TIM9
extern AdvancedTimer timer9;
#endif

#ifdef ENABLE_TIM10
extern AdvancedTimer timer10;
#endif
}