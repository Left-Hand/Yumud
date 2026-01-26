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
    CharsNullTerminated     unit;                       // 空终止测量单位字符串
};

struct [[nodiscard]] TextSelectionContext final{
    CharsNullTerminated options;
    math::fp32          value;
    math::fp32          minimal;
    math::fp32          maximum;
    math::fp32          default_value;
    CharsNullTerminated     unit;                       // 空终止字符串

};

struct [[nodiscard]] StringContext final{
    CharsNullTerminated     value;                      // 空终止字符串

    [[nodiscard]] constexpr size_t bytes_length() const{
        return value.size() + 1;
    }
    [[nodiscard]] constexpr OptionalUCharPtr fill_bytes(std::span<uint8_t> bytes){
        return GenericSerializer::ser_zero_terminated_uchars(bytes, value);
    }
};

struct [[nodiscard]] FolderContext final{
    std::span<const uint8_t>      list_of_children;

    [[nodiscard]] constexpr size_t bytes_length() const{
        // 用0xFF条目标记列表末尾
        return list_of_children.size() + 1;
    }

    __attribute__((always_inline))
    [[nodiscard]] constexpr OptionalUCharPtr fill_bytes(std::span<uint8_t> bytes){
        return GenericSerializer::ser_0xff_terminated_uchars(bytes, list_of_children);
    }
};

struct [[nodiscard]] MessageContext final{
    CharsNullTerminated      info;

    [[nodiscard]] constexpr size_t bytes_length() const{
        return info.size() + 1;
    }
    [[nodiscard]] constexpr OptionalUCharPtr fill_bytes(std::span<uint8_t> bytes){
        return GenericSerializer::ser_zero_terminated_uchars(bytes, info);
    }
};

struct [[nodiscard]] CommandContext final{
    CommandStatus       status;
    uint8_t             timeout; //ms * 100
    CharsNullTerminated      info;

    [[nodiscard]] constexpr size_t bytes_length() const{
        return 1 + 1 + info.size() + 1;
    }

    [[nodiscard]] constexpr OptionalUCharPtr fill_bytes(std::span<uint8_t> bytes){
        auto [ptr, rest] = split_bytes(bytes, 1 + 1);
        ptr[0] = static_cast<uint8_t>(status);
        ptr[1] = timeout;
        return GenericSerializer::ser_zero_terminated_uchars(rest, info);
    }
    // 用0xFF条目标记列表末尾
};


}