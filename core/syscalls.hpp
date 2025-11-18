#pragma once

#include <cstdint>
#include <cstddef>
#include <sys/types.h>

extern "C"{
__attribute__((used))
int _write(int file, char *buf, int len);

__attribute__((used))
void *_sbrk(ptrdiff_t incr);

__attribute__((used))
__attribute__((noreturn))
void _exit(int status);

__attribute__((used))
ssize_t _read(int fd, void *buf, size_t count);
}