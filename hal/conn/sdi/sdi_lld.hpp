#pragma once

#include <cstddef>
#include <cstdint>
#include <span>


namespace ymd::lld{
void sdi_blocking_write_byte(const uint8_t byte);

void sdi_blocking_write_bytes(std::span<const uint8_t> bytes);

[[nodiscard]] size_t sdi_free_capacity();

void sdi_init();

}



