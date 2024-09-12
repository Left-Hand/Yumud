#ifndef __EXTRA_CONVS_HPP__

#define __EXTRA_CONVS_HPP__

#include "../sys/stream/stream.hpp"
#include "../sys/string/string.hpp"
#include "../types/iq/iqt.hpp"

struct iq_t;

OutputStream & operator<<(OutputStream & os, const iq_t value);

#endif