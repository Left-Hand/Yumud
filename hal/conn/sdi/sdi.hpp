#pragma once

#include "sdi_lld.hpp"

namespace ymd::hal{
class Sdi{
public:

    static void blocking_write_byte(const uint8_t byte);

    static void blocking_write_bytes(std::span<const uint8_t> bytes);

    [[nodiscard]] static size_t free_capacity();

    static void init();
};

#ifdef ENABLE_SDI
extern Sdi sdi;
#endif
}