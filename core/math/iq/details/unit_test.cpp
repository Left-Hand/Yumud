#include "_IQNconv.hpp"

using namespace ymd;
using namespace ymd::iqmath::details;


// 添加更多测试
static_assert(_IQFtoN<16>(0.0f) == 0);
static_assert(_IQFtoN<16>(-0.0f) == 0);
static_assert(_IQFtoN<16>(1.0f) == 65536);  // Q16: 1.0 = 65536
static_assert(_IQFtoN<16>(-1.0f) == -65536);
static_assert(_IQFtoN<16>(0.5f) == 32768);
static_assert(_IQFtoN<16>(-0.5f) == -32768);
static_assert(_IQFtoN<16>(0.25f) == 16384);
static_assert(_IQFtoN<16>(4.25f) == 16384 * 17);
static_assert(_IQFtoN<16>(-4.25f) == -16384 * 17);
static_assert(_IQFtoN<16>(1.5f) == 98304);  // 1.5 * 65536 = 98304

// 测试非常小的数
static_assert(_IQFtoN<16>(0.0001f) == 6);  // 近似值
static_assert(_IQFtoN<16>(-0.0001f) == -6);

// 测试Q不同值的情况
static_assert(_IQFtoN<8>(1.0f) == 256);    // Q8: 1.0 = 256
static_assert(_IQFtoN<8>(0.5f) == 128);
static_assert(_IQFtoN<24>(1.0f) == 16777216);  // Q24: 1.0 = 16777216

static_assert(_IQFtoN<16>(-32768.0f / 65536.0f) == -32768);
// static_assert(std::bit_cast<uint32_t>(_IQFtoN<16>(-4.3542107E22F)) == 0xE51386C2);
// static_assert(std::bit_cast<uint32_t>(_IQFtoN<16>(-4.3542107E22F)) == 0xE51386C2);