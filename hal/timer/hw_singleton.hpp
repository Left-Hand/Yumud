
#pragma once

#include "hal/timer/timer.hpp"

namespace ymd::hal{

#ifdef TIM1_PRESENT
extern AdvancedTimer timer1;
#endif

#ifdef TIM2_PRESENT
extern GeneralTimer timer2;
#endif

#ifdef TIM3_PRESENT
extern GeneralTimer timer3;
#endif

#ifdef TIM4_PRESENT
extern GeneralTimer timer4;
#endif

#ifdef TIM5_PRESENT
extern GeneralTimer timer5;
#endif

#ifdef TIM6_PRESENT
extern BasicTimer timer6;
#endif

#ifdef TIM7_PRESENT
extern BasicTimer timer7;
#endif

#ifdef TIM8_PRESENT
extern AdvancedTimer timer8;
#endif

#ifdef TIM9_PRESENT
extern AdvancedTimer timer9;
#endif

#ifdef TIM10_PRESENT
extern AdvancedTimer timer10;
#endif
}
