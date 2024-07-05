#include "tb.h"

#define DMA_TB_STRING

void dma_tb(OutputStream & logger, DmaChannel & channel){

    char src[] = "HelloWorld";
    char dst[sizeof(src)];

    logger.println("DMA Test");
    logger.println("======");
    logger.println("before");
    logger.println("src:", src);
    logger.println("dst:", dst);
    logger.println("======");

    channel.init(DmaChannel::Mode::synergy);
    logger.println("DMA init done");
    channel.begin(dst, src, sizeof(src));
    logger.println("DMA begin");
    while(not channel.done());

    logger.println("======");
    logger.println("after");
    logger.println("src:", src);
    logger.println("dst:", dst);
    logger.println("======");

    if(memcmp(dst, src, sizeof(src)) == 0){
        logger.println("DMA Test: Passed");
    }else{
        logger.println("DMA Test: Failed");
    }
}