#pragma once

#include <type_traits>

struct Endian {
private:
    bool value;

public:
    constexpr explicit Endian(bool _value) : value(_value) {}

    constexpr operator bool() const { return value; }
};

static constexpr Endian MSB{true};
static constexpr Endian LSB{false};