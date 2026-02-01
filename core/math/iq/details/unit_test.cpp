#include "_IQNconv.hpp"

using namespace ymd;
using namespace ymd::iqmath::details;


namespace {

// 添加更多测试
static_assert(_IQFtoN<16>(0.0f) == 0);
static_assert(_IQFtoN<16>(-0.0f) == 0);
static_assert(_IQFtoN<16>(1.0f) == 65536);  // Q16: 1.0 = 65536
static_assert(_IQFtoN<16>(-1.0f) == -65536);

// 测试非常小的数
static_assert(_IQFtoN<16>(0.0001f) == 6);  // 近似值
static_assert(_IQFtoN<16>(-0.0001f) == -6);

// 测试Q不同值的情况
static_assert(_IQFtoN<8>(1.0f) == 256);    // Q8: 1.0 = 256
static_assert(_IQFtoN<8>(0.5f) == 128);
static_assert(_IQFtoN<24>(1.0f) == 16777216);  // Q24: 1.0 = 16777216

static_assert(_IQFtoN<16>(-32768.0f / 65536.0f) == -32768);

static_assert(_IQFtoN<16>(6556.0f) == 6556 << 16);
static_assert(_IQFtoN<16>(.04f) == int32_t(0.04 * 65536));
static_assert(_IQFtoN<16>(.001f) == int32_t(0.001 * 65536));

static_assert(_IQFtoN<16>(-6556.0f) == -6556 * 65536);
static_assert(_IQFtoN<16>(-0.04f) == int32_t(-0.04 * 65536));
static_assert(_IQFtoN<16>(-0.001f) == int32_t(-0.001 * 65536));

static_assert(_IQFtoN<24>(66.0f) == 66 << 24);
static_assert(_IQFtoN<24>(.04f) == int32_t(0.04 * (1 << 24)));
static_assert(_IQFtoN<24>(.0001f) == int32_t(0.0001 * (1 << 24)));

static_assert(_IQFtoN<24>(-66.0f) == -66 * (1 << 24));
static_assert(_IQFtoN<24>(-0.04f) == int32_t(-0.04 * (1 << 24)));
static_assert(_IQFtoN<24>(-0.0001f) == int32_t(-0.0001 * (1 << 24)));



}