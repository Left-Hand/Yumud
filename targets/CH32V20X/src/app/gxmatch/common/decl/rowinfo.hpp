#pragma once

#include "core/platform.hpp"
#include "../enum.hpp"

namespace gxm{
class RowInfo{
protected:
    std::array<MaterialColor,3> colors_ = {
        MaterialColor::None,
        MaterialColor::None,
        MaterialColor::None
    };

    constexpr size_t check(const size_t idx) const {
        if(idx > 2) HALT;
        return idx;
    }
public:
    RowInfo(const RowInfo & other) = delete;
    RowInfo(RowInfo && other) = delete;

    RowInfo() = default;

    __fast_inline constexpr MaterialColor operator [](const TrayIndex idx) const {
        return colors_[check(size_t(idx))];
    }

    __fast_inline constexpr MaterialColor & operator [](const TrayIndex idx){
        return colors_[check(size_t(idx))];
    }

    __fast_inline constexpr bool confident() const {
        return std::find(colors_.begin(), colors_.end(), MaterialColor::None) == colors_.end();
    }
};

}