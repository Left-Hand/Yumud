#include "../decimal.hpp"
#include "../fixedpoint.hpp"
#include "../nondecimal.hpp"
#include "../scientific_fixedpoint.hpp"
#include "../fmtnum.hpp"
#include "core/utils/Result.hpp"

using namespace ymd;
using namespace ymd::str;


namespace {

struct Diag{
    using Self = Diag;

    std::array<char, 32> buffer;
    size_t length;

    static constexpr Self from_zero(){
        Self self;
        std::fill_n(self.buffer.data(), self.buffer.size(), 0);
        self.length = 0;
        return self;
    }

    constexpr MutStrSpan crop() {
        return {buffer.data(), buffer.data() + length};
    }
};

#if 0

__attribute__((optimize("Ofast")))
[[nodiscard]] static constexpr uint64_t approx_div1e9(uint64_t x) {
    struct [[nodiscard]] Dump final{
        uint64_t bits;

        [[nodiscard]] constexpr uint32_t low15() const {
            static constexpr uint32_t MASK = (1u << 15) - 1;
            return uint32_t(bits) & MASK;
        }

        [[nodiscard]] constexpr uint32_t middle15() const {
            static constexpr uint32_t MASK = (1u << 15) - 1;
            return uint32_t(bits >> 15) & MASK;
        }

        [[nodiscard]] constexpr uint32_t high31() const {
            static constexpr uint32_t MASK = (1u << 31) - 1;
            return uint32_t(bits >> 30) & MASK;
        }
    };

    static_assert(sizeof(Dump) == sizeof(uint64_t));
    const Dump dump = std::bit_cast<Dump>(x);
    uint64_t result_bits = 0;

    // 第一部分：低15位
    {
        constexpr uint64_t FACTOR = (UINT64_MAX) / 500000000ULL;
        uint32_t value = dump.low15();
        uint64_t scaled_value = (value * FACTOR) >> 32;
        result_bits += scaled_value;
    }

    // 第二部分：中15位
    {
        constexpr uint64_t FACTOR = uint64_t((1ull << 48) / 1000000000ULL) + 1;
        uint32_t value = dump.middle15();
        uint64_t scaled_value = value * FACTOR;
        result_bits += scaled_value;
    }

    // 第三部分：高31位
    {
        constexpr uint64_t FACTOR = static_cast<uint64_t>((1ull << 52) / 1000000000ULL) + 1;
        uint32_t value = dump.high31();
        uint64_t scaled_value = (value * FACTOR) << 11;
        result_bits += scaled_value;
    }

    // result_bits 是 N * 2^32 / 1e6 的定点表示，右移32位得到整数部分
    return (uint32_t(result_bits >> 32) + 1) >> 1;
}

__attribute__((optimize("Ofast")))
static constexpr std::tuple<uint64_t, uint32_t> divrem1e9(const uint64_t x){
    static constexpr uint32_t _1E9 = 1000ull * 1000ull * 1000ull;
    static constexpr size_t MAX_TRIM_TIMES = 3;


    static_assert(_1E9 < INT32_MAX);
    uint64_t q = approx_div1e9(x);
    uint64_t re_x = q * _1E9;
    int32_t rem = int32_t(x - re_x);


    for(size_t i = 0; i < MAX_TRIM_TIMES; i++){
        if(rem > int32_t(_1E9)){
            q += 1;
            re_x += _1E9;
            rem -= _1E9;
            continue;
        }

        if(rem < 0){
            q += -1;
            re_x += -_1E9;
            rem += _1E9;
            continue;
        }
        break;
    }
    return {q, uint32_t(rem)};
}


static_assert(approx_div1e9(1000000) == 0);
static_assert(approx_div1e9(2000000) == 0);
static_assert(approx_div1e9(0) == 0);
static_assert(approx_div1e9(uint64_t(1E9)) == 1);
static_assert(approx_div1e9(uint64_t(4E9)) == 4);
static_assert(approx_div1e9(uint64_t(7E9)) == 7);
static_assert(approx_div1e9(uint64_t(1E12)) == 1000);
static_assert(approx_div1e9(uint64_t(1000'000'000'000ull)) == 1000);
static_assert(approx_div1e9(uint64_t(999'000'000'000ull)) == 999);
static_assert(approx_div1e9(uint64_t(999'000'000'001ull)) == 999);
static_assert(std::get<0>(divrem1e9(uint64_t(998'999'999'999ull))) == 998);
static_assert(std::get<1>(divrem1e9(uint64_t(998'999'999'999ull))) == 999'999'999);

#endif


static_assert(std::get<0>(bigint_divrem1e9(uint64_t(998'999'999'999ull))) == 998);
static_assert(std::get<1>(bigint_divrem1e9(uint64_t(998'999'999'999ull))) == 999'999'999);
// 边界测试：小于 1e9
static_assert(std::get<0>(bigint_divrem1e9(0ull)) == 0);
static_assert(std::get<1>(bigint_divrem1e9(0ull)) == 0);

static_assert(std::get<0>(bigint_divrem1e9(1ull)) == 0);
static_assert(std::get<1>(bigint_divrem1e9(1ull)) == 1);

static_assert(std::get<0>(bigint_divrem1e9(999'999'999ull)) == 0);
static_assert(std::get<1>(bigint_divrem1e9(999'999'999ull)) == 999'999'999);

// 等于 1e9
static_assert(std::get<0>(bigint_divrem1e9(1'000'000'000ull)) == 1);
static_assert(std::get<1>(bigint_divrem1e9(1'000'000'000ull)) == 0);

// 1e9 + 1
static_assert(std::get<0>(bigint_divrem1e9(1'000'000'001ull)) == 1);
static_assert(std::get<1>(bigint_divrem1e9(1'000'000'001ull)) == 1);

// 2^32 - 1 (4,294,967,295)
static_assert(std::get<0>(bigint_divrem1e9(4'294'967'295ull)) == 4);
static_assert(std::get<1>(bigint_divrem1e9(4'294'967'295ull)) == 294'967'295);

// 2^32 (4,294,967,296)
static_assert(std::get<0>(bigint_divrem1e9(4'294'967'296ull)) == 4);
static_assert(std::get<1>(bigint_divrem1e9(4'294'967'296ull)) == 294'967'296);

// 用户提供的测试：998,999,999,999
static_assert(std::get<0>(bigint_divrem1e9(998'999'999'999ull)) == 998);
static_assert(std::get<1>(bigint_divrem1e9(998'999'999'999ull)) == 999'999'999);

// 恰好 1e9 的倍数：1,000,000,000,000 (1e12)
static_assert(std::get<0>(bigint_divrem1e9(1'000'000'000'000ull)) == 1000);
static_assert(std::get<1>(bigint_divrem1e9(1'000'000'000'000ull)) == 0);

// 2^63 - 1 (9,223,372,036,854,775,807)
static_assert(std::get<0>(bigint_divrem1e9(9'223'372'036'854'775'807ull)) == 9'223'372'036ull);
static_assert(std::get<1>(bigint_divrem1e9(9'223'372'036'854'775'807ull)) == 854'775'807);

// 2^63 (9,223,372,036,854,775,808)
static_assert(std::get<0>(bigint_divrem1e9(9'223'372'036'854'775'808ull)) == 9'223'372'036ull);
static_assert(std::get<1>(bigint_divrem1e9(9'223'372'036'854'775'808ull)) == 854'775'808);

// 最大值 2^64 - 1 (18,446,744,073,709,551,615)
static_assert(std::get<0>(bigint_divrem1e9(18'446'744'073'709'551'615ull)) == 18'446'744'073ull);
static_assert(std::get<1>(bigint_divrem1e9(18'446'744'073'709'551'615ull)) == 709'551'615);

// 另一个大数：1899'999'999'999'999'999 (1,899,999,999,999,999,999)
static_assert(std::get<0>(bigint_divrem1e9(1'899'999'999'999'999'999ull)) == 1'899'999'999ull);
static_assert(std::get<1>(bigint_divrem1e9(1'899'999'999'999'999'999ull)) == 999'999'999);

// 余数接近 1e9-1 的大数：10,000,000,000,999,999,999
static_assert(std::get<0>(bigint_divrem1e9(10'000'000'000'999'999'999ull)) == 10'000'000'000ull);
static_assert(std::get<1>(bigint_divrem1e9(10'000'000'000'999'999'999ull)) == 999'999'999);

// 余数为 0 的大数：10,000,000,000,000,000,000 (1e19)
static_assert(std::get<0>(bigint_divrem1e9(10'000'000'000'000'000'000ull)) == 10'000'000'000ull);
static_assert(std::get<1>(bigint_divrem1e9(10'000'000'000'000'000'000ull)) == 0);

// 随机跨块的大数：123,456,789,012,345,678
static_assert(std::get<0>(bigint_divrem1e9(123'456'789'012'345'678ull)) == 123'456'789ull);
static_assert(std::get<1>(bigint_divrem1e9(123'456'789'012'345'678ull)) == 12'345'678);






[[maybe_unused]] static void test_all(){


    {
        constexpr auto diag = []{
            auto ret = Diag::from_zero();
            ret.length = _fmtnum_u64_dec_fittest(ret.buffer.data(), 1000'245'000) - ret.buffer.data();
            return ret;
        }();

        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '0');
        static_assert(diag.buffer[2] == '0');
        static_assert(diag.buffer[3] == '0');
        static_assert(diag.buffer[4] == '2');
        static_assert(diag.buffer[5] == '4');
        static_assert(diag.buffer[6] == '5');
        static_assert(diag.buffer[7] == '0');
        static_assert(diag.buffer[8] == '0');
        static_assert(diag.buffer[9] == '0');
        static_assert(diag.length == 10);
    }

    {
        constexpr auto diag = []{
            auto ret = Diag::from_zero();
            ret.length = _fmtnum_u64_dec_fittest(ret.buffer.data(), 1999'999'999) - ret.buffer.data();
            return ret;
        }();

        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '9');
        static_assert(diag.buffer[2] == '9');
        static_assert(diag.buffer[3] == '9');
        static_assert(diag.buffer[4] == '9');
        static_assert(diag.buffer[5] == '9');
        static_assert(diag.buffer[6] == '9');
        static_assert(diag.buffer[7] == '9');
        static_assert(diag.buffer[8] == '9');
        static_assert(diag.buffer[9] == '9');
        static_assert(diag.length == 10);
    }

    {
        constexpr auto diag = []{
            auto ret = Diag::from_zero();
            ret.length = _fmtnum_u64_dec_fittest(ret.buffer.data(), 123'999'999'999) - ret.buffer.data();
            return ret;
        }();

        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '2');
        static_assert(diag.buffer[2] == '3');
        static_assert(diag.buffer[3] == '9');
        static_assert(diag.buffer[4] == '9');
        static_assert(diag.buffer[5] == '9');
        static_assert(diag.buffer[6] == '9');
        static_assert(diag.buffer[7] == '9');
        static_assert(diag.buffer[8] == '9');
        static_assert(diag.buffer[9] == '9');
        static_assert(diag.buffer[10] == '9');
        static_assert(diag.buffer[11] == '9');
        static_assert(diag.length == 12);
    }


    {
        constexpr auto diag = []{
            auto ret = Diag::from_zero();
            ret.length = _fmtnum_u64_dec_fittest(ret.buffer.data(), 1899'999'999'999'999) - ret.buffer.data();
            return ret;
        }();

        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '8');
        static_assert(diag.buffer[2] == '9');
        static_assert(diag.buffer[3] == '9');
        static_assert(diag.buffer[4] == '9');
        static_assert(diag.buffer[5] == '9');
        static_assert(diag.buffer[6] == '9');
        static_assert(diag.buffer[7] == '9');
        static_assert(diag.buffer[8] == '9');
        static_assert(diag.buffer[9] == '9');
        static_assert(diag.buffer[10] == '9');
        static_assert(diag.buffer[11] == '9');
        static_assert(diag.buffer[12] == '9');
        static_assert(diag.buffer[13] == '9');
        static_assert(diag.buffer[14] == '9');
        static_assert(diag.buffer[15] == '9');
        static_assert(diag.length == 16);
    }

    {
        constexpr auto diag = []{
            auto ret = Diag::from_zero();
            ret.length = _fmtnum_u64_dec_fittest(ret.buffer.data(), 1899'999'999'999'999'999) - ret.buffer.data();
            return ret;
        }();

        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '8');
        static_assert(diag.buffer[2] == '9');
        static_assert(diag.buffer[3] == '9');
        static_assert(diag.buffer[4] == '9');
        static_assert(diag.buffer[5] == '9');
        static_assert(diag.buffer[6] == '9');
        static_assert(diag.buffer[7] == '9');
        static_assert(diag.buffer[8] == '9');
        static_assert(diag.buffer[9] == '9');
        static_assert(diag.buffer[10] == '9');
        static_assert(diag.buffer[11] == '9');
        static_assert(diag.buffer[12] == '9');
        static_assert(diag.buffer[13] == '9');
        static_assert(diag.buffer[14] == '9');
        static_assert(diag.buffer[15] == '9');
        static_assert(diag.buffer[16] == '9');
        static_assert(diag.buffer[17] == '9');
        static_assert(diag.buffer[18] == '9');
        static_assert(diag.length == 19);
    }

}

}
