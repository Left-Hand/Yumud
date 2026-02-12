#include "xtoa.hpp"
#include "core/utils/Result.hpp"

using namespace ymd;
using namespace ymd::str;


namespace {
// 测试用例
static_assert(_u32_num_digits_r10(0) == 1, "0 should return 1");
static_assert(_u32_num_digits_r10(1) == 1, "1 should return 1");
static_assert(_u32_num_digits_r10(9) == 1, "9 should return 1");
static_assert(_u32_num_digits_r10(10) == 2, "10 should return 2");
static_assert(_u32_num_digits_r10(99) == 2, "99 should return 2");
static_assert(_u32_num_digits_r10(100) == 3, "100 should return 3");
static_assert(_u32_num_digits_r10(100000) == 6, "100 should return 3");

// 关键测试：0x80000000
static_assert(_u32_num_digits_r10(0x80000000) == 10, "0x80000000 should return 10");

// 更大值的测试
static_assert(_u32_num_digits_r10(0xFFFFFFFF) == 10, "0xFFFFFFFF should return 10");
static_assert(_u32_num_digits_r10(0x3B9ACA00) == 10, "0x3B9ACA00 (1e9) should return 10");
static_assert(_u32_num_digits_r10(0x3B9ACA01) == 10, "0x3B9ACA01 should return 10");

#if 0
static constexpr std::tuple<uint32_t, uint32_t> depart_hilo_18(const uint32_t hi, const uint32_t lo) {
    // 计算 val / 10^9 和 val % 10^9
    // val = hi * 2^32 + lo
    // 2^32 = 4294967296 = 4 * 10^9 + 294967296
    
    // 先处理高32位部分
    uint32_t hi_div_1e9 = hi / 1000000000;      // 高位贡献几个完整的10^9
    uint32_t hi_rem_1e9 = hi % 1000000000;      // 高位剩余部分
    
    // hi_rem_1e9 * 294967296 + lo 可能超过32位，用64位中间结果
    uint64_t carry = (uint64_t)hi_rem_1e9 * 294967296 + lo;
    
    // 合并贡献：
    // 1. hi_div_1e9 * 4 来自 hi_div_1e9 * (4 * 10^9) / 10^9
    // 2. carry / 10^9 来自剩余部分的贡献
    uint32_t quotient = hi_div_1e9 * 4 + (uint32_t)(carry / 1000000000);
    uint32_t remainder = (uint32_t)(carry % 1000000000);
    
    return {quotient, remainder};
}

// 基础测试
static_assert(depart_hilo_18(0, 0) == std::make_tuple(0u, 0u), "0 should return (0,0)");
static_assert(depart_hilo_18(0, 1) == std::make_tuple(0u, 1u), "1 should return (0,1)");
static_assert(depart_hilo_18(0, 9) == std::make_tuple(0u, 9u), "9 should return (0,9)");
static_assert(depart_hilo_18(0, 10) == std::make_tuple(0u, 10u), "10 should return (0,10)");
static_assert(depart_hilo_18(0, 999999999) == std::make_tuple(0u, 999999999u), "999999999 should return (0,999999999)");
static_assert(depart_hilo_18(0, 1000000000) == std::make_tuple(1u, 0u), "1e9 should return (1,0)");
static_assert(depart_hilo_18(0, 1000000001) == std::make_tuple(1u, 1u), "1e9+1 should return (1,1)");
static_assert(depart_hilo_18(0, 1999999999) == std::make_tuple(1u, 999999999u), "1999999999 should return (1,999999999)");
static_assert(depart_hilo_18(0, 2000000000) == std::make_tuple(2u, 0u), "2e9 should return (2,0)");
static_assert(depart_hilo_18(0, UINT32_MAX) == std::make_tuple(4u, 294967295u), "0xFFFFFFFF should return (4,294967295)");

// 测试高32位非零的情况
static_assert(depart_hilo_18(1, 0) == std::make_tuple(4u, 294967296u), "0x100000000 should return (4,294967296)");
static_assert(depart_hilo_18(1, 1) == std::make_tuple(4u, 294967297u), "0x100000001 should return (4,294967297)");
static_assert(depart_hilo_18(1, 4294967295) == std::make_tuple(5u, 294967295u), "0x1FFFFFFFF should return (5,294967295)");

// 测试边界值：2^32-1 和 2^32
static_assert(depart_hilo_18(0, 0xFFFFFFFF) == std::make_tuple(4u, 294967295u), "0xFFFFFFFF -> (4,294967295)");
static_assert(depart_hilo_18(1, 0) == std::make_tuple(4u, 294967296u), "0x100000000 -> (4,294967296)");

// 测试接近10^9整数倍的值
static_assert(depart_hilo_18(0, 4000000000) == std::make_tuple(4u, 0u), "4e9 should return (4,0)");
static_assert(depart_hilo_18(0, 4000000001) == std::make_tuple(4u, 1u), "4e9+1 should return (4,1)");
static_assert(depart_hilo_18(4, 294967296) == std::make_tuple(18u, 294967296u), "0x4FFFFFFFF? need verify");

// 测试最大值范围
static_assert(depart_hilo_18(0xFFFFFFFF, 0xFFFFFFFF) == 
              std::make_tuple(18446744073u, 554309260u), "0xFFFFFFFFFFFFFFFF should return known values");
#endif


static constexpr auto _pow10 = [](size_t n) -> uint64_t {
    size_t sum = 1;
    for(size_t i = 0; i < n; i++){
        sum *= 10;
    }
    return sum;
};

[[maybe_unused]] void test_num_digits_r10(){
    constexpr auto u32_test_n = [&](size_t n) -> Result<void, void> {
        if(not (_u32_num_digits_r10(_pow10(n-1)) == n)) return Err();
        if(not (_u32_num_digits_r10(_pow10(n) - 1) == n)) return Err();
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
    static_assert(u32_res.is_ok(), "_u32_num_digits_r10 failed");
}

static_assert(_div_3(0) == 0);
static_assert(_div_3(3) == 1);
static_assert(_div_3(34) == 11);
static_assert(_div_3(33) == 11);


}