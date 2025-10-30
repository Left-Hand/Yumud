#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"

using namespace ymd;


#define REFLECT_FOREACH_1(f, _1) f(_1)
#define REFLECT_FOREACH_2(f, _1, _2) f(_1) f(_2)
#define REFLECT_FOREACH_3(f, _1, _2, _3) f(_1) f(_2) f(_3)
#define REFLECT_FOREACH_4(f, _1, _2, _3, _4) f(_1) f(_2) f(_3) f(_4)
#define REFLECT_FOREACH_5(f, _1, _2, _3, _4, _5) f(_1) f(_2) f(_3) f(_4) f(_5)


#define REFLECT_NARGS_IMPL(_1, _2, _3, _4, _5, N, ...) N

#define REFLECT_EXPAND_2(x) x
#define REFLECT_EXPAND(x) REFLECT_EXPAND_2(x)

#define REFLECT_CONCAT_2(a, b) a##b
#define REFLECT_CONCAT(a, b) REFLECT_CONCAT_2(a,b)

#define REFLECT_NARGS(...) REFLECT_NARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)
#define REFLECT_FOREACH(f, ...) \
REFLECT_EXPAND(REFLECT_CONCAT(REFLECT_FOREACH_, REFLECT_NARGS(__VA_ARGS__))(f, __VA_ARGS__))\


struct Info{
    int a;
    char b;
    iq16 c;
};

void reflect_main(){
    // REFLECT_NARGS(a,b,c)
    // REFLECT_CONCAT(REFLECT_FOREACH_, 3)
    // REFLECT_FOREACH(knfo, a, b, c);
}