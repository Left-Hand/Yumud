#include <cstring>

#include "src/testbench/tb.h"
#include "core/clock/clock.hpp"

#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/dma/dma.hpp"

#include "core/stream/ostream.hpp"

#define DMA_TB_STRING

using namespace ymd;



void dma_tb(OutputStream & logger, hal::DmaChannel & dma_channel){

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

    dma_channel.init({hal::DmaMode::Synergy, hal::DmaPriority::High});
    logger.println("DMA init done");


    dma_channel.set_event_callback([&](const hal::DmaEvent ev){
        switch(ev){
            case hal::DmaEvent::TransferComplete:
                logger.println("d", dma_channel.pending_count());
                break;
            case hal::DmaEvent::TransferOnhalf:
                logger.println("h", dma_channel.pending_count());
                break;
            default:
                break;
        }
    });


    logger.println("DMA it bind done");
    dma_channel.enable_interrupt<hal::DmaIT::Done>(EN);
    dma_channel.enable_interrupt<hal::DmaIT::Half>(EN);
    dma_channel.register_nvic(hal::NvicPriorityCode::highest(),  EN);
    logger.println("DMA begin");
    dma_channel.start_transfer_mem2mem<hal::DmaWordSize::OneByte, hal::DmaWordSize::OneByte>(dst, src, sizeof(src));
    while(dma_channel.pending_count()){
        logger.println(dma_channel.pending_count());
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