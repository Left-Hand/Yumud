#include "dma.hpp"



#define DMA_DONE_CB(x,y) dma##x##_ch##y##_done_cb
#define DMA_HALF_CB(x,y) dma##x##_ch##y##_half_cb


#define DMA_IT_TEMPLATE(x,y)\
static DmaChannel::Callback DMA_DONE_CB(x,y);\
static DmaChannel::Callback DMA_HALF_CB(x,y);\
__interrupt void DMA##x##_Channel##y##_IRQHandler(void){\
    if(DMA_GetFlagStatus(DMA##x##_IT_TC##y)){\
        EXECUTE(DMA_DONE_CB(x,y));\
        DMA_ClearFlag(DMA##x##_IT_TC##y);\
    }else if(DMA_GetFlagStatus(DMA##x##_IT_HT##y)){\
        EXECUTE(DMA_HALF_CB(x,y));\
        DMA_ClearFlag(DMA##x##_IT_HT##y);\
    }\
}\



#define DMA_CH_TEMPLATE(x,y)\
DmaChannel dma##x##Ch##y {DMA##x##_Channel##y };\
DMA_IT_TEMPLATE(x,y);


#define DMA_XY_BIND_DONE_TEMPLATE(x,y)\
case y:\
DMA_DONE_CB(x,y) = cb;\
break;\

#define DMA_XY_BIND_HALF_TEMPLATE(x,y)\
case y:\
DMA_HALF_CB(x,y) = cb;\
break;\


#define DMA1_BIND_CB_TEMPLATE(uname)\
case 1:\
    switch(channel_index){\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 1)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 2)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 3)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 4)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 5)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 6)\
                DMA_XY_BIND_##uname##_TEMPLATE(1, 7)\
    }\
    break;\

#define DMA2_BIND_CB_TEMPLATE(uname)\
case 2:\
    switch(channel_index){\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 1)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 2)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 3)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 4)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 5)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 6)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 7)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 8)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 9)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 10)\
                DMA_XY_BIND_##uname##_TEMPLATE(2, 11)\
    }\
    break;\


#ifdef HAVE_DMA2

#define DMA_BIND_CB_TEMPLATE(name,uname)\
void DmaChannel::bind##name##Cb(Callback && cb){\
    switch(dma_index){\
        DMA1_BIND_CB_TEMPLATE(uname)
        DMA2_BIND_CB_TEMPLATE(uname)
    }\
}\

#else

#define DMA_BIND_CB_TEMPLATE(name,uname)\
void DmaChannel::bind##name##Cb(Callback && cb){\
    switch(dma_index){\
        DMA1_BIND_CB_TEMPLATE(uname)\
    }\
}\

#endif

#ifdef HAVE_DMA1
DMA_CH_TEMPLATE(1,1);
DMA_CH_TEMPLATE(1,2);
DMA_CH_TEMPLATE(1,3);
DMA_CH_TEMPLATE(1,4);
DMA_CH_TEMPLATE(1,5);
DMA_CH_TEMPLATE(1,6);
DMA_CH_TEMPLATE(1,7);
#endif

#ifdef HAVE_DMA2
DMA_CH_TEMPLATE(2,1);
DMA_CH_TEMPLATE(2,2);
DMA_CH_TEMPLATE(2,3);
DMA_CH_TEMPLATE(2,4);
DMA_CH_TEMPLATE(2,5);
DMA_CH_TEMPLATE(2,6);
DMA_CH_TEMPLATE(2,7);
DMA_CH_TEMPLATE(2,8);
DMA_CH_TEMPLATE(2,9);
DMA_CH_TEMPLATE(2,10);
DMA_CH_TEMPLATE(2,11);
#endif

DMA_BIND_CB_TEMPLATE(Done, DONE)
DMA_BIND_CB_TEMPLATE(Half, HALF)