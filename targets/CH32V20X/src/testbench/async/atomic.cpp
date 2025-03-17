
struct atomic
{
    volatile int counter;
};
typedef struct atomic atomic_t;void atomic_inc(atomic_t *atomic)
{
    __asm__ __volatile__("amoadd.w.aqrl  zero, %1, %0\n" : "+A"(atomic->counter) : "r"(1u) : "memory");

    return;
}