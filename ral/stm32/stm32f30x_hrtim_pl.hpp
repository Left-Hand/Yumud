#pragma once

#include "stm32f30x_hrtim_def.hpp"

namespace hwspec::STM32F30x::PL{
using namespace STM32F30x::HRTIM_Regs;

void init_hrpwm(HRTIM_TypeDef * HRTIM1){
    //     RCC->CFGR3   |= RCC_CFGR3_HRTIM1SW_PLL;                                                 // Enable multiplier x2 for PLL frequency
    // RCC->APB2ENR |= RCC_APB2ENR_HRTIM1EN;                                                   // Enable clock for HRPWM

    // HRTIM1->sCommonRegs.DLLCR |= HRTIM_DLLCR_CAL | HRTIM_DLLCR_CALEN;                       // Start timer's calibration
    // while ((HRTIM1->sCommonRegs.ISR & HRTIM_ISR_DLLRDY) == RESET);                          // Waiting for the end fo calibration

    // HRTIM1->sTimerxRegs[0].PERxR = Hrpwm::periodHrpwm;                                      // Set period for timer
    // HRTIM1->sTimerxRegs[0].CMP1xR = 0;                                                      // Set starting duty
    // HRTIM1->sTimerxRegs[1].PERxR = Hrpwm::periodHrpwm;                                      
    // HRTIM1->sTimerxRegs[1].CMP1xR = 0;                                                      

    // HRTIM1->sTimerxRegs[0].OUTxR |= HRTIM_OUTR_DTEN;                                        // Enable dead-time
    // HRTIM1->sTimerxRegs[0].DTxR  |= (3 << HRTIM_DTR_DTPRSC_Pos);                            // Set Tdtg = (2^3) * 868 ps = 6.94 ns
    // HRTIM1->sTimerxRegs[0].DTxR  |= (5 << HRTIM_DTR_DTR_Pos) | (5 << HRTIM_DTR_DTF_Pos);    // Set dead-time rising and falling = 5 * Ttg = 34.7 ns
    // HRTIM1->sTimerxRegs[0].DTxR  |= HRTIM_DTR_DTFSLK | HRTIM_DTR_DTRSLK;                    // Lock value dead-time
    // HRTIM1->sTimerxRegs[1].OUTxR |= HRTIM_OUTR_DTEN;                                        
    // HRTIM1->sTimerxRegs[1].DTxR  |= (3 << HRTIM_DTR_DTPRSC_Pos);                            
    // HRTIM1->sTimerxRegs[1].DTxR  |= (5 << HRTIM_DTR_DTR_Pos) | (5 << HRTIM_DTR_DTF_Pos);    
    // HRTIM1->sTimerxRegs[1].DTxR  |= HRTIM_DTR_DTFSLK | HRTIM_DTR_DTRSLK;                    

    // HRTIM1->sTimerxRegs[0].SETx1R |= HRTIM_SET1R_PER;	                                    // Event forces the output to active state
    // HRTIM1->sTimerxRegs[0].RSTx1R |= HRTIM_RST1R_CMP1;                                      // Event forces the output to inactive state
    // HRTIM1->sTimerxRegs[1].SETx1R |= HRTIM_SET1R_PER;	                                    
    // HRTIM1->sTimerxRegs[1].RSTx1R |= HRTIM_RST1R_CMP1;                                      

    // HRTIM1->sTimerxRegs[0].TIMxCR |= HRTIM_TIMCR_CONT;                                      // Continuous mode
    // HRTIM1->sTimerxRegs[1].TIMxCR |= HRTIM_TIMCR_CONT;                                      

    // HRTIM1->sCommonRegs.OENR |= HRTIM_OENR_TA1OEN | HRTIM_OENR_TB1OEN;                      // Enable output PWM channel A and B

    // HRTIM1->sMasterRegs.MPER = Hrpwm::periodHrpwm;                                          // Period for master timer
    // HRTIM1->sMasterRegs.MCR |= HRTIM_MCR_MCEN | HRTIM_MCR_TACEN | HRTIM_MCR_TBCEN;          // Enable counter for Master and timer A and B

};
}
