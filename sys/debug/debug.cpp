#include "debug_inc.h"

extern "C"{
__attribute__((used)) int _write(int fd, char *buf, int size)
{
    DEBUGGER.write(buf, size);
    return size;
}
__attribute__((used)) void *_sbrk(ptrdiff_t incr)
{
    extern char _end[];
    extern char _heap_end[];
    static char *curbrk = _end;

    if ((curbrk + incr < _end) || (curbrk + incr > _heap_end))
    return (void *)UINT32_MAX;

    curbrk += incr;
    return curbrk - incr;
}

__attribute__((used)) void _exit(int status){
    while(true);
}
}