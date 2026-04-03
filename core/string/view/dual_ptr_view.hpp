#pragma once

namespace ymd{

template<typename T>
struct [[nodiscard]] DualPtrView final{
    T * begin;
    T * end;
};


}