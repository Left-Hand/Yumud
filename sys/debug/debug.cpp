#include "debug_inc.h"
#include "hal/bus/uart/uarthw.hpp"

using namespace ymd;

int _write(int file, char *buf, int len){
    DEBUGGER.write(buf, len);
    return len;
}


void *_sbrk(ptrdiff_t incr){
    extern char _end[];
    extern char _heap_end[];
    static char *curbrk = _end;

    if ((curbrk + incr < _end) || (curbrk + incr > _heap_end))
    return (void *)UINT32_MAX;

    curbrk += incr;
    return curbrk - incr;
}

void _exit(int status){
    while(true);
}

ssize_t _read(int fd, void *buf, size_t count){
    return 0;
}
