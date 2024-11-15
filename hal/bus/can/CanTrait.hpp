#pragma once

namespace ymd{

class CanTrait{
public:

public:
    CanTrait() = default;
    CanTrait(const CanTrait & other) = delete;
    CanTrait(CanTrait && other) = delete;
};

}