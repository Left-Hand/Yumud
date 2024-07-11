#pragma once

#include "../../sys/core/platform.h"
#include <vector>

std::vector<uint8_t> quicklz_compress(const std::vector<uint8_t> & src);
std::vector<uint8_t> quicklz_decompress(const std::vector<uint8_t>& qlzdata);