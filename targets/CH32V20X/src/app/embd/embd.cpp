#include "embd.h"

#ifdef CH32V20X
#include "node.hpp"
#else
#include "host.hpp"
#endif

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

    auto & logger = DEBUGGER;
    logger.init(576000, CommMethod::Blocking);

    can1.init(Can::BaudRate::_1M);
    can1.enableHwReTransmit();
    can1.cancelAllTransmit();

    usbfs.init();

    uart7.init(115200);
    EmbdHost host{logger, can1};

    host.main();
}
#endif