#include "../decimal.hpp"
#include "../fixedpoint.hpp"
#include "../nondecimal.hpp"
#include "../scientific_fixedpoint.hpp"
#include "../fmtnum.hpp"
#include "core/utils/Result.hpp"

using namespace ymd;
using namespace ymd::str;


namespace {
// 测试用例
static_assert(_least_u32_num_digits_dec(0) == 1, "0 should return 1");
static_assert(_least_u32_num_digits_dec(1) == 1, "1 should return 1");
static_assert(_least_u32_num_digits_dec(9) == 1, "9 should return 1");
static_assert(_least_u32_num_digits_dec(10) == 2, "10 should return 2");
static_assert(_least_u32_num_digits_dec(99) == 2, "99 should return 2");
static_assert(_least_u32_num_digits_dec(100) == 3, "100 should return 3");
static_assert(_least_u32_num_digits_dec(100000) == 6, "100 should return 3");

static_assert(div_10_maylossy(0) == 0);
static_assert(div_10_maylossy(9) == 0);
static_assert(div_10_maylossy(99) == 9);
static_assert(div_10_maylossy(100) == 10);
static_assert(div_10_maylossy(100000) == 10000);
static_assert(div_10_maylossy(999999) == 99999);
static_assert(div_10_maylossy(1000000) == 100000);
static_assert(div_10_maylossy(9999999) == 999999);
static_assert(div_10_maylossy(10000000) == 1000000);
static_assert(div_10_maylossy(99999999) == 9999999);
static_assert(div_10_maylossy(100000000) == 10000000);
static_assert(div_10_maylossy(999999999) == 99999999);


static_assert(frac_abs_f32_nonfpu(114.514f, 10000) == 5140);
static_assert(frac_abs_f32_nonfpu(0.1919f, 10000) == 1919);
static_assert(frac_abs_f32_nonfpu(0.19185f, 10000) == 1919);
static_assert(frac_abs_f32_nonfpu(0.191849f, 10000) == 1918);
static_assert(frac_abs_f32_nonfpu(0.114514f, 1000000) == 114514);
static_assert(frac_abs_f32_nonfpu(0.001f, 1000) == 1);
static_assert(frac_abs_f32_nonfpu(1e-8f, 1000) == 0);


static_assert(frac_abs_f32_nonfpu(1099.9996, 1e3) == 1000);

static_assert(floor_abs_f32_nonfpu(1000) == 1000);
static_assert(floor_abs_f32_nonfpu(1e6) == 1e6);
static_assert(floor_abs_f32_nonfpu(1e7) == 1e7);
// static_assert(floor_abs_f32_nonfpu(1e8) == 1e8);
// static_assert(floor_abs_f32(1e8) == 1e8);


// 关键测试：0x80000000
static_assert(_least_u32_num_digits_dec(0x80000000) == 10, "0x80000000 should return 10");

// 更大值的测试
static_assert(_least_u32_num_digits_dec(0xFFFFFFFF) == 10, "0xFFFFFFFF should return 10");
static_assert(_least_u32_num_digits_dec(0x3B9ACA00) == 10, "0x3B9ACA00 (1e9) should return 10");
static_assert(_least_u32_num_digits_dec(0x3B9ACA01) == 10, "0x3B9ACA01 should return 10");

static constexpr auto _pow10 = [](size_t n) -> uint64_t {
    size_t sum = 1;
    for(size_t i = 0; i < n; i++){
        sum *= 10;
    }
    return sum;
};

[[maybe_unused]] static void test_num_digits_dec(){
    constexpr auto u32_test_n = [&](size_t n) -> Result<void, void> {
        if(not (_least_u32_num_digits_dec(_pow10(n-1)) == n)) return Err();
        if(not (_least_u32_num_digits_dec(_pow10(n) - 1) == n)) return Err();
        return Ok();
    };

    constexpr auto test_all = [&]<typename Fn>(Fn && fn, size_t n) -> Result<void, int>{
        for(int i = 1; i <= int(n); i++){
            if(const auto res = (fn)(i); res.is_err()){
                return Err(i);
            }
        }
        return Ok();
    };

    static constexpr auto u32_res = test_all(u32_test_n, 9);
    static_assert(u32_res.is_ok(), "_least_u32_num_digits_dec failed");
}

static_assert(_div_3(0) == 0);
static_assert(_div_3(3) == 1);
static_assert(_div_3(34) == 11);
static_assert(_div_3(33) == 11);



static_assert(_least_u32_num_digits_hex(0xFFFFFFFF) == 8);
static_assert(_least_u32_num_digits_hex(0xFFFFFFF) == 7);
static_assert(_least_u32_num_digits_hex(0xFFFFFF) == 6);
static_assert(_least_u32_num_digits_hex(0xFFFFF) == 5);
static_assert(_least_u32_num_digits_hex(0xFFFF) == 4);


// 测试用例
static_assert(_least_u32_num_digits_oct(0xFFFFFFFF) == 11);  // 37777777777 (32位全1，11位八进制)
static_assert(_least_u32_num_digits_oct(077777777) == 8);    // 8位八进制
static_assert(_least_u32_num_digits_oct(0777777) == 6);      // 6位八进制
static_assert(_least_u32_num_digits_oct(07777) == 4);        // 4位八进制
static_assert(_least_u32_num_digits_oct(077) == 2);          // 2位八进制
static_assert(_least_u32_num_digits_oct(07) == 1);           // 1位八进制
static_assert(_least_u32_num_digits_oct(0) == 1);            // 0特殊处理



static_assert(_least_u32_num_digits_hex(0xFFFFFFFF) == 8);
static_assert(_least_u32_num_digits_hex(0xFFFFFFF) == 7);
static_assert(_least_u32_num_digits_hex(0xFFFFFF) == 6);
static_assert(_least_u32_num_digits_hex(0xFFFFF) == 5);
static_assert(_least_u32_num_digits_hex(0xFFFF) == 4);


static_assert(_div_100000(uint32_t(0xFFFFFFFF)) == 0xFFFFFFFF / 100000);
static_assert(_div_100000(uint32_t(100000)) == 1);

static_assert(sizeof(uint32_t) == 4);

}
