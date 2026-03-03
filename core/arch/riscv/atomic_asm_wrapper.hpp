#pragma once

namespace ymd::core::sync{
// #define __riscv 1
// #define __riscv_atomic 1
// #define __riscv_cmodel_medlow 1
// #define __riscv_float_abi_soft 1
// #define __riscv_fdiv 1
// #define __riscv_flen 64
// #define __riscv_compressed 1
// #define __riscv_mul 1
// #define __riscv_muldiv 1
// #define __riscv_xlen 32
// #define __riscv_fsqrt 1
// #define __riscv_div 1

#ifdef __riscv 

#ifdef __riscv_atomic
/**
 * \brief  Atomic And with 32bit value
 * \details Atomically AND 32bit value with value in memory using amoand.d.
 * \param [in]    addr   Address pointer to data, address need to be 4byte aligned
 * \param [in]    value  value to be ANDed
 * \return  return memory value & and value
 */
static inline int32_t __AMOAND_W(volatile int32_t *addr, int32_t value) {
    int32_t result;

    __asm volatile ("amoand.w %0, %2, %1" : \
            "=r"(result), "+A"(*addr) : "r"(value) : "memory");
    return *addr;
}

/**
 * \brief  Atomic OR with 32bit value
 * \details Atomically OR 32bit value with value in memory using amoor.d.
 * \param [in]    addr   Address pointer to data, address need to be 4byte aligned
 * \param [in]    value  value to be ORed
 * \return  return memory value | and value
 */
static inline int32_t __AMOOR_W(volatile int32_t *addr, int32_t value) {
    int32_t result;

    __asm volatile ("amoor.w %0, %2, %1" : \
            "=r"(result), "+A"(*addr) : "r"(value) : "memory");
    return *addr;
}

/**
 * \brief  Atomic XOR with 32bit value
 * \details Atomically XOR 32bit value with value in memory using amoxor.d.
 * \param [in]    addr   Address pointer to data, address need to be 4byte aligned
 * \param [in]    value  value to be XORed
 * \return  return memory value ^ and value
 */
static inline uint32_t __AMOXOR_W(volatile int32_t *addr,uint32_t value) {
    uint32_t result;
    __asm volatile ("amoxor.w %0, %2, %1" : \
            "=r"(result), "+A"(*addr) : "r"(value) : "memory");
    return *addr;
}
/**
 * \brief Atomic Add with 32bit value
 * \details Atomically ADD 32bit value using amoadd.w
 * \param [in] addr   Address pointer (4-byte aligned)
 * \param [in] value  Value to add
 * \return New memory value after addition
 */
static inline int32_t __AMOADD_W(volatile int32_t *addr, int32_t value) {
    int32_t result;
    __asm volatile ("amoadd.w %0, %2, %1" : 
            "=r"(result), "+A"(*addr) : "r"(value) : "memory");
    return result;
}

/**
 * \brief Atomic Swap with 32bit value
 * \details Atomically swap memory value using amoswap.w
 * \param [in] addr   Address pointer (4-byte aligned)
 * \param [in] value  New value to set
 * \return Previous memory value
 */
static inline int32_t __AMOSWAP_W(volatile int32_t *addr, int32_t value) {
    int32_t result;
    __asm volatile ("amoswap.w %0, %2, %1" :
            "=r"(result), "+A"(*addr) : "r"(value) : "memory");
    return result;
}


/**
 * \brief Compare-and-Swap operation
 * \param [in] addr    Memory address (4-byte aligned)
 * \param [in] expected Expected value
 * \param [in] desired  New value
 * \return true if swap succeeded, false otherwise
 */
static inline bool __CAS_W(volatile int32_t *addr, int32_t expected, int32_t desired) {
    int32_t temp;
    __asm volatile (
        "1: lr.w %0, %1\n"
        "   bne %0, %2, 2f\n"
        "   sc.w %0, %3, %1\n"
        "   bnez %0, 1b\n"
        "2:"
        : "=&r"(temp), "+A"(*addr)
        : "r"(expected), "r"(desired)
        : "memory"
    );
    return temp == expected;
}

/**
 * \brief Atomic Maximum (signed)
 * \details Atomically store max of current value and input using amomax.w
 */
static inline int32_t __AMOMAX_W(volatile int32_t *addr, int32_t value) {
    int32_t result;
    __asm volatile ("amomax.w %0, %2, %1" :
            "=r"(result), "+A"(*addr) : "r"(value) : "memory");
    return result;
}

/**
 * \brief Atomic Minimum (unsigned)
 * \details Atomically store min of current value and input using amominu.w
 */
static inline uint32_t __AMOMINU_W(volatile uint32_t *addr, uint32_t value) {
    uint32_t result;
    __asm volatile ("amominu.w %0, %2, %1" :
            "=r"(result), "+A"(*addr) : "r"(value) : "memory");
    return result;
}
}

#endif

#endif