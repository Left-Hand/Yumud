#include "dvp.hpp"
#include "core/debug/debug.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/gpio_port.hpp"

using namespace ymd;
using namespace ymd::hal;
#ifdef ENABLE_DVP

void Dvp::plant(){
    hal::PA<9>().inpu();
    hal::PA<10>().inpu();
    hal::PC<8>().inpu();
    hal::PC<9>().inpu();
    hal::PC<11>().inpu();
    hal::PB<6>().inpu();
    hal::PB<8>().inpu();
    hal::PB<9>().inpu();

    hal::PA<6>().inpu();
    hal::PA<4>().inpu();
    hal::PA<5>().inpu();
}


void Dvp::enable_rcc(const Enable en){
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DVP, en == EN);
}    

void Dvp::init(const Config & cfg){
    plant();
    enable_rcc(EN);

    // 使用8位采集模式，PCLK上升沿采样数据，HSYNC高电平有效，VSYNC高电平数据有效，使能DVP
    DVP->CR0 = RB_DVP_D8_MOD | RB_DVP_V_POLAR | RB_DVP_ENABLE;
    // DVP->CR0 = RB_DVP_D8_MOD | RB_DVP_ENABLE;
    // 捕获所有帧，捕获完整图像，连续模式，使能DMA
    DVP->CR1 = DVP_RATE_100P | RB_DVP_DMA_EN;

    DVP->ROW_NUM = cfg.num_row;                     // rows行数
    DVP->COL_NUM = cfg.num_col;                     // cols列数

    DVP->DMA_BUF0 = reinterpret_cast<size_t>(cfg.image0_addr);        // DMA addr0
    DVP->DMA_BUF1 = reinterpret_cast<size_t>(cfg.image1_addr);        // DMA addr1

    // 使能中断
    DVP->IER = RB_DVP_IE_FRM_DONE;              // 开启帧接收完成中断

    NVIC_SetPriority(DVP_IRQn, 0);
    NVIC_EnableIRQ(DVP_IRQn);
}

extern "C"{
__interrupt void DVP_IRQHandler(void){
    if (DVP->IFR & RB_DVP_IF_ROW_DONE){
        /* Write 0 clear 0 */
        DVP->IFR &= ~RB_DVP_IF_ROW_DONE;  //clear Interrupt

    }

    else if (DVP->IFR & RB_DVP_IF_FRM_DONE){
        DVP->IFR &= ~RB_DVP_IF_FRM_DONE;  //clear Interrupt

    }

    else if (DVP->IFR & RB_DVP_IF_STR_FRM){
        DVP->IFR &= ~RB_DVP_IF_STR_FRM;  //clear Interrupt
    }

    else if (DVP->IFR & RB_DVP_IF_STP_FRM){
        DVP->IFR &= ~RB_DVP_IF_STP_FRM;  //clear Interrupt
    }

    else if (DVP->IFR & RB_DVP_IF_FIFO_OV){
        DVP->IFR &= ~RB_DVP_IF_FIFO_OV;   //clear Interrupt
    }
}
}

Dvp dvp;
#endif