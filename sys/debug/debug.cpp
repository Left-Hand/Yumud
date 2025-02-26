#include "debug.hpp"

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


extern "C"{


void _init(void) {}
void _fini(void) {}
    
void _kill(int pid, int sig) {
    // 空实现
}
pid_t _getpid(){
  return 0;
}

int raise(int sig) {
    // 不执行任何操作
    return 0; // 返回0表示成功
}

void abort(void){
  while(1);
}

void *__dso_handle = nullptr;
void __cxa_atexit(void (*func)(void), void *objptr, void *dso_handle) {}
// void * __cxa_atexit =0;

// FILE *__sf_fake_stderr(void) {
//     return NULL;
// }
// FILE *__sf_fake_stdout(void) {
//     return NULL;
// }
// FILE *__sf_fake_stdin(void) {
//     return NULL;
// }

}