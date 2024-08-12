#ifndef __EXTRA_CONVS_HPP__

#define __EXTRA_CONVS_HPP__

#include "../sys/kernel/stream.hpp"
#include "../sys/string/string.hpp"
#include "../types/iq/iqt.hpp"

struct iq_t;

String toString(const iq_t iq, unsigned char eps = 3);
OutputStream & operator<<(OutputStream & os, const iq_t value);

#endif