#pragma once

#include "Apriltag.hpp"
#include "Apriltag_utils.hpp"

#include "core/utils/Option.hpp"

namespace ymd::nvcv2::apriltag{
class Apriltag16H5Decoder:public ApriltagDecoder{
public:
    static constexpr std::array<uint16_t, 30> PATTERN_CODE_TABLE = {
        0x231b,
        0x2ea5,
        0x346a,
        0x45b9,
        0x79a6,
        0x7f6b,
        0xb358,
        0xe745,
        0xfe59,
        0x156d,
        0x380b,
        0xf0ab,
        0x0d84,
        0x4736,
        0x8c72,
        0xaf10,
        0x093c,
        0x93b4,
        0xa503,
        0x468f,
        0xe137,
        0x5795,
        0xdf42,
        0x1c1d,
        0xe9dc,
        0x73ad,
        0xad5f,
        0xd530,
        0x07ca,
        0xaf2e
    };

    constexpr Apriltag16H5Decoder(){;}
    Option<ApriltagResult> update(const ApriltagPattern pattern){
        return utils::find_code(
            std::span(PATTERN_CODE_TABLE), utils::get_rcr_code_16h5, pattern);
    }
private:
    Option<ApriltagResult> result_ = None;
};

};