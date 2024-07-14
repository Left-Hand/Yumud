#include "embd.h"

#include "host.hpp"
#include "node.hpp"

void embd_main(){
    #ifdef CH32V20X
    node_main();
    #else
    host_main();
    #endif
    while(true);
}

#ifdef CH32V30X
void host_main(){
    using TimerUtils::IT;

    auto & logger = uart2;
    logger.init(921600, CommMethod::Blocking);

    can1.init(Can::BaudRate::Mbps1);
    usbfs.init();

    uart7.init(115200);
    while(true){
        uart7.println("csjck");
    }

    EmbdHost host{uart7, can1};

    timer3.init(800);
    timer3.bindCb(IT::Update, [&](){host.run();});
    timer3.enableIt(IT::Update, NvicPriority(0, 0));

    host.main();
}
#endif