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
DMA_IT_TEMPLATE(1,1);
DMA_IT_TEMPLATE(1,2);
DMA_IT_TEMPLATE(1,3);
DMA_IT_TEMPLATE(1,4);
DMA_IT_TEMPLATE(1,5);
DMA_IT_TEMPLATE(1,6);
DMA_IT_TEMPLATE(1,7);
#endif

#ifdef HAVE_DMA2
DMA_IT_TEMPLATE(2,1);
DMA_IT_TEMPLATE(2,2);
DMA_IT_TEMPLATE(2,3);
DMA_IT_TEMPLATE(2,4);
DMA_IT_TEMPLATE(2,5);
DMA_IT_TEMPLATE(2,6);
DMA_IT_TEMPLATE(2,7);
DMA_IT_TEMPLATE(2,8);
DMA_IT_TEMPLATE(2,9);
DMA_IT_TEMPLATE(2,10);
DMA_IT_TEMPLATE(2,11);
#endif

DMA_BIND_CB_TEMPLATE(Done, DONE)
DMA_BIND_CB_TEMPLATE(Half, HALF)

#undef DMA_DONE_CB
#undef DMA_IT_TEMPLATE
#undef DMA_XY_BIND_DONE_TEMPLATE
#undef DMA_XY_BIND_HALF_TEMPLATE
#undef DMA1_BIND_CB_TEMPLATE
#undef DMA2_BIND_CB_TEMPLATE
#undef DMA_BIND_CB_TEMPLATE

#ifdef HAVE_DMA1
DmaChannel dma1Ch1{DMA1_Channel1};
DmaChannel dma1Ch2{DMA1_Channel2};
DmaChannel dma1Ch3{DMA1_Channel3};
DmaChannel dma1Ch4{DMA1_Channel4};
DmaChannel dma1Ch5{DMA1_Channel5};
DmaChannel dma1Ch6{DMA1_Channel6};
DmaChannel dma1Ch7{DMA1_Channel7};
#endif

#ifdef HAVE_DMA2
DmaChannel dma2Ch1{DMA2_Channel1};
DmaChannel dma2Ch2{DMA2_Channel2};
DmaChannel dma2Ch3{DMA2_Channel3};
DmaChannel dma2Ch4{DMA2_Channel4};
DmaChannel dma2Ch5{DMA2_Channel5};
DmaChannel dma2Ch6{DMA2_Channel6};
DmaChannel dma2Ch7{DMA2_Channel7};
DmaChannel dma2Ch8{DMA2_Channel8};
DmaChannel dma2Ch9{DMA2_Channel9};
DmaChannel dma2Ch10{DMA2_Channel10};
DmaChannel dma2Ch11{DMA2_Channel11};
#endif