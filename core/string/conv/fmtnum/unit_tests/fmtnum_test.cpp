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
    std::array<char, 32> buffer;
    size_t length;

    constexpr MutStrSpan crop() {
        return {buffer.data(), buffer.data() + length};
    }
};


[[maybe_unused]] void test_fmt_u8(){


    {
        constexpr auto diag = []{
            Diag ret;
            ret.length = _fmtnum_u32_dec_fittest(ret.buffer.data(), 11) - ret.buffer.data();
            return ret;
        }();

        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '1');
        static_assert(diag.length == 2);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            ret.length = 4;
            _fmtnum_u32_dec_padded(ret.crop(), 11);
            return ret;
        }();

        static_assert(diag.buffer[0] == '0');
        static_assert(diag.buffer[1] == '0');
        static_assert(diag.buffer[2] == '1');
        static_assert(diag.buffer[3] == '1');
        static_assert(diag.length == 4);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            ret.length = 4;
            _fmtnum_u32_dec_padded(ret.crop(), 114514);
            return ret;
        }();

        static_assert(diag.buffer[0] == '4');
        static_assert(diag.buffer[1] == '5');
        static_assert(diag.buffer[2] == '1');
        static_assert(diag.buffer[3] == '4');
        static_assert(diag.length == 4);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            ret.length = _fmtnum_u32_dec_fittest(ret.buffer.data(), 112178021) - ret.buffer.data();
            return ret;
        }();

        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '1');
        static_assert(diag.buffer[2] == '2');
        static_assert(diag.buffer[3] == '1');
        static_assert(diag.buffer[4] == '7');
        static_assert(diag.buffer[5] == '8');
        static_assert(diag.buffer[6] == '0');
        static_assert(diag.buffer[7] == '2');
        static_assert(diag.buffer[8] == '1');
        static_assert(diag.length == 9);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            ret.length = 2;
            _fmtnum_u32_hex(ret.crop(), 0x5a);
            return ret;
        }();

        static_assert(diag.buffer[0] == '5');
        static_assert(diag.buffer[1] == 'A');
        static_assert(diag.length == 2);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            ret.length = 8;
            _fmtnum_u32_bin(ret.crop(), 0x5a);
            return ret;
        }();


        static_assert(diag.buffer[0] == '0');
        static_assert(diag.buffer[1] == '1');
        static_assert(diag.buffer[2] == '0');
        static_assert(diag.buffer[3] == '1');
        static_assert(diag.buffer[4] == '1');
        static_assert(diag.buffer[5] == '0');
        static_assert(diag.buffer[6] == '1');
        static_assert(diag.buffer[7] == '0');
        static_assert(diag.length == 8);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            const auto end = fmtnum_integral32(ret.buffer.data(), 0x5a, 8, IntTypeErased::from<int32_t>());
            ret.length = end - ret.buffer.data();
            return ret;
        }();


        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '3');
        static_assert(diag.buffer[2] == '2');
        static_assert(diag.length == 3);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            const auto end = fmtnum_integral32(ret.buffer.data(), 2134, 10, IntTypeErased::from<int32_t>());
            ret.length = end - ret.buffer.data();
            return ret;
        }();


        static_assert(diag.buffer[0] == '2');
        static_assert(diag.buffer[1] == '1');
        static_assert(diag.buffer[2] == '3');
        static_assert(diag.buffer[3] == '4');
        static_assert(diag.length == 4);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            const auto end = fmtnum_integral32(ret.buffer.data(), 254, 2, IntTypeErased::from<int8_t>());
            ret.length = end - ret.buffer.data();
            return ret;
        }();


        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '1');
        static_assert(diag.buffer[2] == '1');
        static_assert(diag.buffer[3] == '1');

        static_assert(diag.buffer[4 + 0] == '1');
        static_assert(diag.buffer[4 + 1] == '1');
        static_assert(diag.buffer[4 + 2] == '1');
        static_assert(diag.buffer[4 + 3] == '0');
        static_assert(diag.length == 8);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            const auto end = fmtnum_integral32(ret.buffer.data(), -123, 10, IntTypeErased::from<int8_t>());
            ret.length = end - ret.buffer.data();
            return ret;
        }();


        static_assert(diag.buffer[0] == '-');
        static_assert(diag.buffer[1] == '1');
        static_assert(diag.buffer[2] == '2');
        static_assert(diag.buffer[3] == '3');

        static_assert(diag.length == 4);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            const auto end = fmtnum_integral32(ret.buffer.data(), 0x12345678, 16, IntTypeErased::from<uint32_t>());
            ret.length = end - ret.buffer.data();
            return ret;
        }();

        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '2');
        static_assert(diag.buffer[2] == '3');
        static_assert(diag.buffer[3] == '4');

        static_assert(diag.buffer[4 + 0] == '5');
        static_assert(diag.buffer[4 + 1] == '6');
        static_assert(diag.buffer[4 + 2] == '7');
        static_assert(diag.buffer[4 + 3] == '8');
        static_assert(diag.length == 8);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            const auto end = fmtnum_integral32(ret.buffer.data(), 0x12345678, 16, IntTypeErased::from<uint16_t>());
            ret.length = end - ret.buffer.data();
            return ret;
        }();

        static_assert(diag.buffer[0] == '5');
        static_assert(diag.buffer[1] == '6');
        static_assert(diag.buffer[2] == '7');
        static_assert(diag.buffer[3] == '8');
        static_assert(diag.length == 4);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            const auto end = fmtnum_integral32(ret.buffer.data(), UINT32_MAX, 10, IntTypeErased::from<uint32_t>());
            ret.length = end - ret.buffer.data();
            return ret;
        }();


        static_assert(diag.buffer[0] == '4');
        static_assert(diag.buffer[1] == '2');
        static_assert(diag.buffer[2] == '9');
        static_assert(diag.buffer[3] == '4');

        static_assert(diag.buffer[9] == '5');
        static_assert(diag.length == 10);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            const auto end = fmtnum_integral32(ret.buffer.data(), UINT32_MAX, 10, IntTypeErased::from<int32_t>());
            ret.length = end - ret.buffer.data();
            return ret;
        }();


        static_assert(diag.buffer[0] == '-');
        static_assert(diag.buffer[1] == '1');

        static_assert(diag.length == 2);
    }

    {
        constexpr auto diag = []{
            Diag ret;
            const auto type = FixedTypeErased{.is_signed = false, .q_num = 16};
            const auto end = fmtnum_fixedpoint(ret.buffer.data(), uint32_t(1.145127 * (1ull << 16)), 6, type);
            ret.length = end - ret.buffer.data();
            return ret;
        }();


        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '.');
        static_assert(diag.buffer[2] == '1');
        static_assert(diag.buffer[3] == '4');
        static_assert(diag.buffer[4] == '5');
        static_assert(diag.buffer[5] == '1');
        static_assert(diag.buffer[6] == '2');
        static_assert(diag.buffer[7] == '6');

        static_assert(
            std::abs((long double)(uint32_t(1.145127 * (1ull << 16))) / (1ull << 16) - 1.14514) <=
            std::abs((long double)(uint32_t(1.145126 * (1ull << 16))) / (1ull << 16) - 1.14514)
        );

        static_assert(diag.length == 8);
    }


    {
        constexpr auto diag = []{
            Diag ret;
            const auto type = FixedTypeErased{.is_signed = false, .q_num = 28};
            const auto end = fmtnum_fixedpoint(ret.buffer.data(), uint32_t(1.145127 * (1ull << 28)), 7, type);
            ret.length = end - ret.buffer.data();
            return ret;
        }();


        static_assert(diag.buffer[0] == '1');
        static_assert(diag.buffer[1] == '.');
        static_assert(diag.buffer[2] == '1');
        static_assert(diag.buffer[3] == '4');
        static_assert(diag.buffer[4] == '5');
        static_assert(diag.buffer[5] == '1');
        static_assert(diag.buffer[6] == '2');
        static_assert(diag.buffer[7] == '7');
        static_assert(diag.buffer[8] == '1');

        static_assert(diag.length == 9);
    }
}

}