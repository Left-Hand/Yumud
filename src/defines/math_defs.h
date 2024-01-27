#ifndef __MATH_DEFS_H__

#define __MATH_DEFS_H__

#define CMP_EPSILON 0.00001
#define CMP_EPSILON2 (CMP_EPSILON * CMP_EPSILON)

#define CMP_NORMALIZE_TOLERANCE 0.000001
#define CMP_POINT_IN_PLANE_EPSILON 0.00001

#ifndef LN2
#define LN2 0.6931471805599453094172321215
#endif

#ifndef TAU
#define TAU 6.2831853071795864769252867666
#endif

#ifndef PI
#define PI 3.1415926535897932384626433833
#endif

#ifndef E
#define E 2.7182818284590452353602874714
#endif

#ifndef INF
#define INF (__builtin_inff())
#endif

#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif

#ifndef SQRT3
#define SQRT3 1.73205080757f
#endif

#ifndef SQRT2
#define SQRT2 1.41421356237f
#endif

#ifndef LOGE
#define LOGE 0.434294481903
#endif

#ifndef LOG2
#define LOG2 0.301029995664
#endif


#ifndef MAX
#define MAX(x,y) (x > y ? x : y)
#endif

#ifndef MIN
#define MIN(x,y) (x < y ? x : y)
#endif

#ifndef ABS
#define ABS(x) (x < 0? -(x) : x)
#endif

#ifndef SIGN
#define SIGN(x) (x < 0 ? -1 : 1)
#endif

#ifndef IS_ALIGNED
#define IS_ALIGNED(x, a) (((x) & ((typeof(x))(a) - 1)) == 0)
#endif

#ifndef TYPE_CHECK
#define TYPE_CHECK(a,b) (typeof(a) == typeof(b))
#endif

#ifndef SWAP
#define SWAP(a, b, type) { \
    type temp; \
    temp = (a); \
    (a) = (b); \
    (b) = temp; \
}
#endif

#ifndef CLAMP
#define CLAMP(x, mi, ma) MIN(MAX(x, mi), ma)
#endif

#endif