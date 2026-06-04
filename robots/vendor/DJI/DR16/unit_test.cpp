#include "dr16.hpp"

using namespace ymd::robots::dji::dr16;

namespace{

[[maybe_unused]] static void test_code(){
    {
        constexpr auto ks = []{
            KeySpecifiers ret{0};
            ret.key_a().set(1);
            return ret;
        }();

        static_assert(ks.key_a().get() == 1);
        static_assert(ks.key_w().get() == 0);
    }
}


static_assert(__builtin_offsetof(FrameContext, mouse_x) == 6);
static_assert(__builtin_offsetof(FrameContext, mouse_y) == 8);
static_assert(__builtin_offsetof(FrameContext, mouse_z) == 10);
static_assert(__builtin_offsetof(FrameContext, mouse_left_pressed) == 12);
static_assert(__builtin_offsetof(FrameContext, mouse_right_pressed) == 13);
static_assert(__builtin_offsetof(FrameContext, key_specifiers) == 14);
static_assert(__builtin_offsetof(FrameContext, __resv__) == 16);

static_assert(sizeof(FrameContext) == 18);

}