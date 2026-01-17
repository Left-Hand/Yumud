#include "fft32.hpp"

using namespace ymd;
using namespace ymd::dsp;

namespace {
    [[maybe_unused]] static void test_dft_bin_0(){ 
        constexpr auto real_in = [] ->std::array<math::fixed_t<16, int32_t>, 32>{
            std::array<math::fixed_t<16, int32_t>, 32> arr;
            arr.fill(1);
            return arr;
        }();
    
        static_assert(dft32_bin0<16>(std::span(real_in)) == 1);
    }

}
