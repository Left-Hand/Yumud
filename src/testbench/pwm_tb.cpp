#include "tb.h"

void pwm_tb(OutputStream & logger){
    while(true){
        logger.println(t);
        delayMicroseconds(100);
    }
}