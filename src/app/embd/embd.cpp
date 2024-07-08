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