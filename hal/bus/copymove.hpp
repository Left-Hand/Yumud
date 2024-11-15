#pragma once

namespace ymd{

class NonCopyTrait{
protected:
    NonCopyTrait() = default;
    ~NonCopyTrait() = default;

private:
    NonCopyTrait(const NonCopyTrait&) = delete;
    NonCopyTrait& operator=(const NonCopyTrait&) = delete;
};


class NonMoveTrait {
protected:
    NonMoveTrait() = default;
    ~NonMoveTrait() = default;

private:
    NonMoveTrait(NonMoveTrait&&) = delete;
    NonMoveTrait& operator=(NonMoveTrait&&) = delete;
};


class NonCopyMoveTrait : public NonCopyTrait, public NonMoveTrait {
};


}