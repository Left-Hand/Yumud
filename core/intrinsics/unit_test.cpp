#include "ctzclz.hpp"

using namespace ymd::intrinsics;
namespace {
static_assert(ctz32(0) == 32, "ctz(0) should be 32");
static_assert(ctz32(1) == 0, "ctz(1) should be 0");
static_assert(ctz32(2) == 1, "ctz(2) should be 1");
static_assert(ctz32(3) == 0, "ctz(3) should be 0");
static_assert(ctz32(4) == 2, "ctz(4) should be 2");
static_assert(ctz32(5) == 0, "ctz(5) should be 0");
static_assert(ctz32(6) == 1, "ctz(6) should be 1");
static_assert(ctz32(7) == 0, "ctz(7) should be 0");
static_assert(ctz32(8) == 3, "ctz(8) should be 3");
static_assert(ctz32(0x80000000) == 31, "ctz(0x80000000) should be 31");
static_assert(ctz32(0xFFFFFFFF) == 0, "ctz(0xFFFFFFFF) should be 0");
static_assert(ctz32(0x00010000) == 16, "ctz(0x00010000) should be 16");
static_assert(ctz32(0x00008000) == 15, "ctz(0x00008000) should be 15");
static_assert(ctz32(0x00000010) == 4, "ctz(0x00000010) should be 4");
static_assert(ctz32(0x00000020) == 5, "ctz(0x00000020) should be 5");
static_assert(ctz32(0x00000400) == 10, "ctz(0x00000400) should be 10");
static_assert(ctz32(0x00000800) == 11, "ctz(0x00000800) should be 11");
static_assert(ctz32(0x00001000) == 12, "ctz(0x00001000) should be 12");
static_assert(ctz32(0x00002000) == 13, "ctz(0x00002000) should be 13");
static_assert(ctz32(0x00004000) == 14, "ctz(0x00004000) should be 14");
static_assert(ctz32(0x00008000) == 15, "ctz(0x00008000) should be 15");
static_assert(ctz32(0x00010000) == 16, "ctz(0x00010000) should be 16");
static_assert(ctz32(0x00020000) == 17, "ctz(0x00020000) should be 17");
static_assert(ctz32(0x00040000) == 18, "ctz(0x00040000) should be 18");
static_assert(ctz32(0x00080000) == 19, "ctz(0x00080000) should be 19");
static_assert(ctz32(0x00100000) == 20, "ctz(0x00100000) should be 20");
static_assert(ctz32(0x00200000) == 21, "ctz(0x00200000) should be 21");
static_assert(ctz32(0x00400000) == 22, "ctz(0x00400000) should be 22");
static_assert(ctz32(0x00800000) == 23, "ctz(0x00800000) should be 23");
static_assert(ctz32(0x01000000) == 24, "ctz(0x01000000) should be 24");
static_assert(ctz32(0x02000000) == 25, "ctz(0x02000000) should be 25");
static_assert(ctz32(0x04000000) == 26, "ctz(0x04000000) should be 26");
static_assert(ctz32(0x08000000) == 27, "ctz(0x08000000) should be 27");
static_assert(ctz32(0x10000000) == 28, "ctz(0x10000000) should be 28");
static_assert(ctz32(0x20000000) == 29, "ctz(0x20000000) should be 29");
static_assert(ctz32(0x40000000) == 30, "ctz(0x40000000) should be 30");
static_assert(ctz32(0x80000000) == 31, "ctz(0x80000000) should be 31");

// 测试
static_assert(clz32(0) == 32, "clz(0) should be 32");
static_assert(clz32(1) == 31, "clz(1) should be 31");
static_assert(clz32(2) == 30, "clz(2) should be 30");
static_assert(clz32(3) == 30, "clz(3) should be 30");
static_assert(clz32(4) == 29, "clz(4) should be 29");
static_assert(clz32(0x80000000) == 0, "clz(0x80000000) should be 0");
static_assert(clz32(0x40000000) == 1, "clz(0x40000000) should be 1");
static_assert(clz32(0x00008000) == 16, "clz(0x00008000) should be 16");
static_assert(clz32(0x00010000) == 15, "clz(0x00010000) should be 15");
static_assert(clz32(0xFFFFFFFF) == 0, "clz(0xFFFFFFFF) should be 0");
}
