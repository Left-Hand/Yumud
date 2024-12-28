#include <cstdint>

#ifdef STM32F30x

static inline void InitializeData() {

    uint32_t *pSrc;
    uint32_t *pDest;

    // Load initialized data from FLASH to RAM
    pSrc = &__etext;
    pDest = &__data_start__;

    while (pDest < &__data_end__)
    *pDest++ = *pSrc++;

    // Clear uninitialized data
    pDest = &__bss_start__;

    while (pDest < &__bss_end__)
    *pDest++ = 0;

    #ifdef __FPU_USED
        __asm(
            "  LDR.W R0, =0xE000ED88\n"
            "  LDR R1, [R0]\n"
            "  ORR R1, R1, #(0xF << 20)\n"
            "  STR R1, [R0]"
        );
    #endif
}

extern "C" void ResetHandler() {
    InitializeData();
    main();
}

#endif