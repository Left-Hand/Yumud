#include "src/testbench/tb.h"
#include "core/clock/clock.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/dma/dma.hpp"

#include "core/stream/ostream.hpp"

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

    channel.init({DmaChannel::Mode::synergy, DmaPriority::High});
    logger.println("DMA init done");


    channel.bind_half_cb([&](){
        logger.println("h", channel.pending());
    });

    channel.bind_done_cb([&](){
        logger.println("d", channel.pending());
    });



    logger.println("DMA it bind done");
    channel.enable_done_it();
    channel.enable_half_it();
    channel.enable_it({0,0});
    logger.println("DMA begin");
    channel.transfer_mem2mem<char>(dst, src, sizeof(src));
    while(channel.pending()){
        logger.println(channel.pending());
        clock::delay(200ms);
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