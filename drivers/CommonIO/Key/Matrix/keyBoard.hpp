#pragma once

#include <array>
#include "../Key.hpp"

template<size_t W, size_t H>
class KeyMatrix{
public:


protected:
    std::array<std::array<Key, W>, H> keys;
};