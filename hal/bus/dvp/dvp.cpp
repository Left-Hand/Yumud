#include "dvp.hpp"

#ifdef HAVE_DVP

void Dvp::install(){
    portA[9].InPullUP();
    portA[10].InPullUP();
    portC[8].InPullUP();
    portC[9].InPullUP();
    portC[11].InPullUP();
    portB[6].InPullUP();
    portB[8].InPullUP();
    portB[9].InPullUP();

    portA[6].InPullUP();
    portA[4].InPullUP();
    portA[5].InPullUP();
}

void Dvp::init(uint32_t *image0_addr, uint32_t *image1_addr, uint16_t col_len, uint16_t row_len){
    install();
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DVP, ENABLE);

    // 使用8位采集模式，PCLK上升沿采样数据，HSYNC高电平有效，VSYNC高电平数据有效，使能DVP
    DVP->CR0 = RB_DVP_D8_MOD | RB_DVP_V_POLAR | RB_DVP_ENABLE;
    // DVP->CR0 = RB_DVP_D8_MOD | RB_DVP_ENABLE;
    // 捕获所有帧，捕获完整图像，连续模式，使能DMA
    DVP->CR1 = DVP_RATE_100P | RB_DVP_DMA_EN;

    DVP->ROW_NUM = row_len;                     // rows行数
    DVP->COL_NUM = col_len;                     // cols列数

    DVP->DMA_BUF0 = (uint32_t)image0_addr;        // DMA addr0
    DVP->DMA_BUF1 = (uint32_t)image1_addr;        // DMA addr1

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