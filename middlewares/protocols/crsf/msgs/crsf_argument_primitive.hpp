#pragma once

#include "crsf_primitive.hpp"

namespace ymd::crsf{

struct [[nodiscard]] FloatContext final{
    math::fp32          value;
    math::fp32          minimal;
    math::fp32          maximum;
    math::fp32          default_value;
    uint8_t             decimal_point;
    int32_t             step_size;
    ustr<MAX_STR_LENGTH>     unit;                       // 空终止测量单位字符串

};

struct [[nodiscard]] TextSelectionContext final{
    ustr<MAX_STR_LENGTH> options;
    math::fp32          value;
    math::fp32          minimal;
    math::fp32          maximum;
    math::fp32          default_value;
    ustr<MAX_STR_LENGTH>     unit;                       // 空终止字符串
};

struct [[nodiscard]] StringContext final{
    ustr<MAX_STR_LENGTH>     value;                      // 空终止字符串

    [[nodiscard]] constexpr size_t bytes_length() const{
        return value.length() + 1;
    }

    template<typename Receiver>
    [[nodiscard]] constexpr void sink_to(Receiver & receiver) const{
        auto & self = *this;
        receiver.push_push_zero_terminated_uchars(self.value);
    }
};

struct [[nodiscard]] FolderContext final{
    std::span<const uint8_t>      list_of_children;

    [[nodiscard]] constexpr size_t bytes_length() const{
        // 用0xFF条目标记列表末尾
        return list_of_children.size() + 1;
    }
};

struct [[nodiscard]] MessageContext final{
    ustr<MAX_STR_LENGTH>      info;

    [[nodiscard]] constexpr size_t bytes_length() const{
        return info.length() + 1;
    }
};

struct [[nodiscard]] CommandContext final{
    CommandStatus       status;
    uint8_t             timeout; //ms * 100
    ustr<MAX_STR_LENGTH>      info;    // 用0xFF条目标记列表末尾

    [[nodiscard]] constexpr size_t bytes_length() const{
        return 1 + 1 + info.length() + 1;
    }
};


}