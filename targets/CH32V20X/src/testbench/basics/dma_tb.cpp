#include "src/testbench/tb.h"
#include "core/clock/clock.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/dma/dma.hpp"

#include "core/stream/ostream.hpp"

#define DMA_TB_STRING

using namespace ymd;



void dma_tb(OutputStream & logger, hal::DmaChannel & channel){

    char src[] = 
    R"(The quick brown fox jumps over the lazy dog, 
    A favorite copy set by writing teachers for their pupils is the following, 
    because it contains every letter of the alphabet:
    'A quick brown fox jumps over the lazy dog.)";
    char dst[sizeof(src)];

    logger.println("DMA Test");
    logger.println("======");
    logger.println("before");
    logger.println("src:", src);
    logger.println("dst:", dst);
    logger.println("======");

    channel.init({hal::DmaMode::Synergy, hal::DmaPriority::High});
    logger.println("DMA init done");


    channel.set_event_handler([&](const hal::DmaEvent ev){
        switch(ev){
            case hal::DmaEvent::TransferComplete:
                logger.println("d", channel.remaining());
                break;
            case hal::DmaEvent::HalfTransfer:
                logger.println("h", channel.remaining());
                break;
            default:
                break;
        }
    });


    logger.println("DMA it bind done");
    channel.enable_interrupt<hal::DmaIT::Done>(EN);
    channel.enable_interrupt<hal::DmaIT::Half>(EN);
    channel.register_nvic({0,0}, EN);
    logger.println("DMA begin");
    channel.start_transfer_mem2mem<char>(dst, src, sizeof(src));
    while(channel.remaining()){
        logger.println(channel.remaining());
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