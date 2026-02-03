#include "syscalls.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;




extern "C"{

int _write(int file, char * buf_begin, int len){
    DEBUGGER.write_bytes(std::span(
        reinterpret_cast<const uint8_t *>(buf_begin),
        len
    ));
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
    __builtin_trap();
}

ssize_t _read(int fd, void *buf, size_t count){
    return 0;
}

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

}

// 避免链接器错误，提供空实现
void __cxa_pure_virtual() {
    // 纯虚函数调用时的处理
    while(1);  // 或你的错误处理
}

extern "C" {


// ------------------------------------------------------------
// 文件系统桩函数实现
// 在无文件系统环境下，所有操作均成功但不执行实际操作
// ------------------------------------------------------------

// 打开文件 - 桩实现
int _open_r(struct _reent* reent_ptr, const char* filename, int open_flags) {
    // 消除未使用参数警告，使用不同变量名和顺序
    (void)reent_ptr;
    (void)filename;
    (void)open_flags;

    // 桩实现总是返回成功
    // 注意：实际无文件系统环境中，可以返回0表示成功
    return 0;
}

// 关闭文件 - 桩实现
int _close_r(struct _reent* reent_ptr, int file_descriptor) {
    (void)reent_ptr;
    (void)file_descriptor;

    // 关闭操作总是成功
    return 0;
}

// 文件定位 - 桩实现
int _lseek_r(struct _reent* reent_ptr, int fd, int offset, int whence) {
    // 参数处理使用不同命名
    struct _reent* unused_reent = reent_ptr;
    int unused_fd = fd;
    int unused_offset = offset;
    int unused_whence = whence;

    (void)unused_reent;
    (void)unused_fd;
    (void)unused_offset;
    (void)unused_whence;

    // 定位操作总是成功，返回当前位置0
    return 0;
}

// 读取文件 - 桩实现
int _read_r(struct _reent* reent_ptr, int fd, char* buffer, size_t count) {
    // 不同参数处理方式：赋值给临时变量
    struct _reent* r = reent_ptr;
    int file = fd;
    char* buf = buffer;
    size_t len = count;

    (void)r;
    (void)file;
    (void)buf;
    (void)len;

    // 读取0字节表示文件结束
    return 0;
}


int _putc_r(struct _reent*, int, FILE*) __attribute__((weak));


// ------------------------------------------------------------
// 程序终止处理函数
// ------------------------------------------------------------

// 程序退出处理注册 - 桩实现
// 在资源受限环境中，退出处理通常不可用
int atexit(void (*exit_handler)(void)) {
    // 消除未使用参数警告
    void (*handler)(void) = exit_handler;
    (void)handler;

    // 桩实现不支持退出处理，返回失败
    // 注意：这里选择返回-1表示不支持，与原始可能不同
    return -1;
}


};
