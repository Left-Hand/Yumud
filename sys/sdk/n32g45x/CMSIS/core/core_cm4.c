#include "core_cm4.h"


void _init(void) {}
void _fini(void) {}

__attribute__((weak)) void *__dso_handle;

/*@} end of CMSIS_core_DebugFunctions */

// _close_r
int _close_r(struct _reent *r, int fd) {
    // Do nothing
    return -1;
}

// _fstat_r
int _fstat_r(struct _reent *r, int fd, struct stat *st) {
    // Do nothing
    return -1;
}

// _getpid_r
pid_t _getpid_r(struct _reent *r) {
    // Do nothing
    return -1;
}

// _gettimeofday_r
int _gettimeofday_r(struct _reent *r, struct timeval *tv, void *tz) {
    // Do nothing
    return -1;
}

// _isatty_r
int _isatty_r(struct _reent *r, int fd) {
    // Do nothing
    return 0;
}

// _kill_r
int _kill_r(struct _reent *r, pid_t pid, int sig) {
    // Do nothing
    return -1;
}

// _link_r
int _link_r(struct _reent *r, const char *oldpath, const char *newpath) {
    // Do nothing
    return -1;
}

// _lseek_r
off_t _lseek_r(struct _reent *r, int fd, off_t offset, int whence) {
    // Do nothing
    return -1;
}

// _open_r
int _open_r(struct _reent *r, const char *path, int flags, ...) {
    // Do nothing
    return -1;
}

// _read_r
ssize_t _read_r(struct _reent *r, int fd, void *ptr, size_t n) {
    // Do nothing
    return -1;
}

// _times_r
clock_t _times_r(struct _reent *r, struct tms *buf) {
    // Do nothing
    return -1;
}

// _unlink_r
int _unlink_r(struct _reent *r, const char *path) {
    // Do nothing
    return -1;
}

// _write_r
ssize_t _write_r(struct _reent *r, int fd, const void *ptr, size_t n) {
    // Do nothing
    return -1;
}