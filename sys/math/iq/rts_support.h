#pragma once

#include <cstdint>

#ifndef __fast_inline
#define __fast_inline __inline
#endif

#ifndef scexpr
#define scexpr static constexpr
#endif


#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpy_clear_ctl0)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr void __mpy_clear_ctl0(void)
{
    /* Do nothing. */
    return;
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpy_set_frac)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr void __mpy_set_frac(void)
{
    /* Do nothing. */
    return;
}


////////////////////////////////////////////////////////////
//                                                        //
//                16-bit functions                        //
//                                                        //
////////////////////////////////////////////////////////////
#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpy_w)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int16_t __mpy_w(int16_t arg1, int16_t arg2)
{
    return (arg1 * arg2);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpy_uw)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr uint16_t __mpy_uw(uint16_t arg1, uint16_t arg2)
{
    return (arg1 * arg2);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyx_w)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int32_t __mpyx_w(int16_t arg1, int16_t arg2)
{
    return ((int32_t)arg1 * (int32_t)arg2);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyx_uw)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr uint32_t __mpyx_uw(uint16_t arg1, uint16_t arg2)
{
    return ((uint32_t)arg1 * (uint32_t)arg2);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyf_w)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int16_t __mpyf_w(int16_t arg1, int16_t arg2)
{
    return (int16_t)(((int32_t)arg1 * (int32_t)arg2) >> 15);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyf_w_reuse_arg1)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int16_t __mpyf_w_reuse_arg1(int16_t arg1, int16_t arg2)
{
    /* This is identical to __mpyf_w */
    return (int16_t)(((int32_t)arg1 * (int32_t)arg2) >> 15);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyf_uw)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr uint16_t __mpyf_uw(uint16_t arg1, uint16_t arg2)
{
    return (uint16_t)(((uint32_t)arg1 * (uint32_t)arg2) >> 15);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyf_uw_reuse_arg1)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr uint16_t __mpyf_uw_reuse_arg1(uint16_t arg1, uint16_t arg2)
{
    /* This is identical to __mpyf_uw */
    return (uint16_t)(((uint32_t)arg1 * (uint32_t)arg2) >> 15);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyfx_w)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int32_t __mpyfx_w(int16_t arg1, int16_t arg2)
{
    return (((int32_t)arg1 * (int32_t)arg2) << 1);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyfx_uw)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int32_t __mpyfx_uw(uint16_t arg1, uint16_t arg2)
{
    return (((uint32_t)arg1 * (uint32_t)arg2) << 1);
}


////////////////////////////////////////////////////////////
//                                                        //
//                 32-bit functions                       //
//                                                        //
////////////////////////////////////////////////////////////
#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpy_l)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int32_t __mpy_l(int32_t arg1, int32_t arg2)
{
    return (arg1 * arg2);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpy_ul)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr uint32_t __mpy_ul(uint32_t arg1, uint32_t arg2)
{
    return (arg1 * arg2);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyx)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int64_t __mpyx(int32_t arg1, int32_t arg2)
{
    return ((int64_t)arg1 * (int64_t)arg2);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyx_u)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr uint64_t __mpyx_u(uint32_t arg1, uint32_t arg2)
{
    return ((uint64_t)arg1 * (uint64_t)arg2);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyf_l)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int32_t __mpyf_l(int32_t arg1, int32_t arg2)
{
    return (int32_t)(((int64_t)arg1 * (int64_t)arg2) >> 31);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyf_l_reuse_arg1)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int32_t __mpyf_l_reuse_arg1(int32_t arg1, int32_t arg2)
{
    /* This is identical to __mpyf_l */
    return (int32_t)(((int64_t)arg1 * (int64_t)arg2) >> 31);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyf_ul)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr uint32_t __mpyf_ul(uint32_t arg1, uint32_t arg2)
{
    return (uint32_t)(((uint64_t)arg1 * (uint64_t)arg2) >> 31);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyf_ul_reuse_arg1)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int32_t __mpyf_ul_reuse_arg1(uint32_t arg1, uint32_t arg2)
{
    /* This is identical to __mpyf_ul */
    return (uint32_t)(((uint64_t)arg1 * (uint64_t)arg2) >> 31);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyfx)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr int64_t __mpyfx(int32_t arg1, int32_t arg2)
{
    return (((int64_t)arg1 * (int64_t)arg2) << 1);
}

#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__mpyfx_u)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif
__fast_inline constexpr uint64_t __mpyfx_u(uint32_t arg1, uint32_t arg2)
{
    return (((uint64_t)arg1 * (uint64_t)arg2) << 1);
}
