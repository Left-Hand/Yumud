#include "src/testbench/tb.h"
#include "core/clock/clock.h"

#define DMA_TB_STRING

using namespace ymd;
using namespace ymd::hal;


void dma_tb(OutputStream & logger, DmaChannel & channel){

    char src[] = "The quick brown fox jumps over the lazy dog, A favorite copy set by writing teachers for their pupils is the following, because it contains every letter of the alphabet:'A quick brown fox jumps over the lazy dog.";
    char dst[sizeof(src)];

    logger.println("DMA Test");
    logger.println("======");
    logger.println("before");
    logger.println("src:", src);
    logger.println("dst:", dst);
    logger.println("======");

    channel.init(DmaChannel::Mode::synergy);
    logger.println("DMA init done");


    channel.bindHalfCb([&](){
        logger.println("h", channel.pending());
    });

    channel.bindDoneCb([&](){
        logger.println("d", channel.pending());
    });



    logger.println("DMA it bind done");
    channel.enableDoneIt();
    channel.enableHalfIt();
    channel.enableIt({0,0});
    logger.println("DMA begin");
    channel.start(dst, src, sizeof(src));
    while(channel.pending()){
        logger.println(channel.pending());
        delay(200);
    }


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