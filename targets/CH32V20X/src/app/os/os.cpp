#include "os.hpp"
#include "offset_table.hpp"

#define STORE_REG(lhs, rhs) asm volatile("lw " #lhs ", (" #rhs ")")
#define RESTORE_BY_OFFSET(lhs, offset) asm volatile("lw " #lhs ", " #offset "(sp)")

#define SP_ADDI(offset) asm volatile("addi sp, sp, " #offset)
#define CSRW(lhs, rhs) asm volatile("csrw " #lhs ", " #rhs)
#define MRET asm volatile("mret")

namespace RiscvCpu{

void restore_context(void){

    RESTORE_BY_OFFSET(t0, __reg_mepc_OFFSET);
    CSRW(mepc, t0);
    RESTORE_BY_OFFSET(t0, __reg_mstatus_OFFSET);
    CSRW(mstatus, t0);

    RESTORE_BY_OFFSET(x1, __reg_x1_OFFSET);
    RESTORE_BY_OFFSET(x3, __reg_x3_OFFSET);
    RESTORE_BY_OFFSET(x4, __reg_x4_OFFSET);
    RESTORE_BY_OFFSET(x5, __reg_x5_OFFSET);
    RESTORE_BY_OFFSET(x6, __reg_x6_OFFSET);
    RESTORE_BY_OFFSET(x7, __reg_x7_OFFSET);
    RESTORE_BY_OFFSET(x8, __reg_x8_OFFSET);
    RESTORE_BY_OFFSET(x9, __reg_x9_OFFSET);
    RESTORE_BY_OFFSET(x10, __reg_x10_OFFSET);
    RESTORE_BY_OFFSET(x11, __reg_x11_OFFSET);
    RESTORE_BY_OFFSET(x12, __reg_x12_OFFSET);
    RESTORE_BY_OFFSET(x13, __reg_x13_OFFSET);
    RESTORE_BY_OFFSET(x14, __reg_x14_OFFSET);
    RESTORE_BY_OFFSET(x15, __reg_x15_OFFSET);
    RESTORE_BY_OFFSET(x16, __reg_x16_OFFSET);
    RESTORE_BY_OFFSET(x17, __reg_x17_OFFSET);
    RESTORE_BY_OFFSET(x18, __reg_x18_OFFSET);
    RESTORE_BY_OFFSET(x19, __reg_x19_OFFSET);
    RESTORE_BY_OFFSET(x20, __reg_x20_OFFSET);
    RESTORE_BY_OFFSET(x21, __reg_x21_OFFSET);
    RESTORE_BY_OFFSET(x22, __reg_x22_OFFSET);
    RESTORE_BY_OFFSET(x23, __reg_x23_OFFSET);
    RESTORE_BY_OFFSET(x24, __reg_x24_OFFSET);
    RESTORE_BY_OFFSET(x25, __reg_x25_OFFSET);
    RESTORE_BY_OFFSET(x26, __reg_x26_OFFSET);
    RESTORE_BY_OFFSET(x27, __reg_x27_OFFSET);
    RESTORE_BY_OFFSET(x28, __reg_x28_OFFSET);
    RESTORE_BY_OFFSET(x29, __reg_x29_OFFSET);
    RESTORE_BY_OFFSET(x30, __reg_x30_OFFSET);
    RESTORE_BY_OFFSET(x31, __reg_x31_OFFSET);

    SP_ADDI(128);

#ifdef __riscv_f
    RESTORE_BY_OFFSET(f0, __reg_f0_OFFSET);
    RESTORE_BY_OFFSET(f1, __reg_f1_OFFSET);
    RESTORE_BY_OFFSET(f2, __reg_f2_OFFSET);
    RESTORE_BY_OFFSET(f3, __reg_f3_OFFSET);
    RESTORE_BY_OFFSET(f4, __reg_f4_OFFSET);
    RESTORE_BY_OFFSET(f5, __reg_f5_OFFSET);
    RESTORE_BY_OFFSET(f6, __reg_f6_OFFSET);
    RESTORE_BY_OFFSET(f7, __reg_f7_OFFSET);
    RESTORE_BY_OFFSET(f8, __reg_f8_OFFSET);
    RESTORE_BY_OFFSET(f9, __reg_f9_OFFSET);
    RESTORE_BY_OFFSET(f10, __reg_f10_OFFSET);
    RESTORE_BY_OFFSET(f11, __reg_f11_OFFSET);
    RESTORE_BY_OFFSET(f12, __reg_f12_OFFSET);
    RESTORE_BY_OFFSET(f13, __reg_f13_OFFSET);
    RESTORE_BY_OFFSET(f14, __reg_f14_OFFSET);
    RESTORE_BY_OFFSET(f15, __reg_f15_OFFSET);
    RESTORE_BY_OFFSET(f16, __reg_f16_OFFSET);
    RESTORE_BY_OFFSET(f17, __reg_f17_OFFSET);
    RESTORE_BY_OFFSET(f18, __reg_f18_OFFSET);
    RESTORE_BY_OFFSET(f19, __reg_f19_OFFSET);
    RESTORE_BY_OFFSET(f20, __reg_f20_OFFSET);
    RESTORE_BY_OFFSET(f21, __reg_f21_OFFSET);
    RESTORE_BY_OFFSET(f22, __reg_f22_OFFSET);
    RESTORE_BY_OFFSET(f23, __reg_f23_OFFSET);
    RESTORE_BY_OFFSET(f24, __reg_f24_OFFSET);
    RESTORE_BY_OFFSET(f25, __reg_f25_OFFSET);
    RESTORE_BY_OFFSET(f26, __reg_f26_OFFSET);
    RESTORE_BY_OFFSET(f27, __reg_f27_OFFSET);
    RESTORE_BY_OFFSET(f28, __reg_f28_OFFSET);
    RESTORE_BY_OFFSET(f29, __reg_f29_OFFSET);
    RESTORE_BY_OFFSET(f30, __reg_f30_OFFSET);
    RESTORE_BY_OFFSET(f31, __reg_f31_OFFSET);

    SP_ADDI(128);
#endif
}

void SW_Handler(void){
    #if ARCH_RISCV_FPU
	addi sp, sp, -128
	fsw  f0,  __reg_f0_OFFSET(sp)
	fsw  f1,  __reg_f1_OFFSET(sp)
	fsw  f2,  __reg_f2_OFFSET(sp)
	fsw  f3,  __reg_f3_OFFSET(sp)
	fsw  f4,  __reg_f4_OFFSET(sp)
	fsw  f5,  __reg_f5_OFFSET(sp)
	fsw  f6,  __reg_f6_OFFSET(sp)
	fsw  f7,  __reg_f7_OFFSET(sp)
	fsw  f8,  __reg_f8_OFFSET(sp)
	fsw  f9,  __reg_f9_OFFSET(sp)
	fsw  f10, __reg_f10_OFFSET(sp)
	fsw  f11, __reg_f11_OFFSET(sp)
	fsw  f12, __reg_f12_OFFSET(sp)
	fsw  f13, __reg_f13_OFFSET(sp)
	fsw  f14, __reg_f14_OFFSET(sp)
	fsw  f15, __reg_f15_OFFSET(sp)
	fsw  f16, __reg_f16_OFFSET(sp)
	fsw  f17, __reg_f17_OFFSET(sp)
	fsw  f18, __reg_f18_OFFSET(sp)
	fsw  f19, __reg_f19_OFFSET(sp)
	fsw  f20, __reg_f20_OFFSET(sp)
	fsw  f21, __reg_f21_OFFSET(sp)
	fsw  f22, __reg_f22_OFFSET(sp)
	fsw  f23, __reg_f23_OFFSET(sp)
	fsw  f24, __reg_f24_OFFSET(sp)
	fsw  f25, __reg_f25_OFFSET(sp)
	fsw  f26, __reg_f26_OFFSET(sp)
	fsw  f27, __reg_f27_OFFSET(sp)
	fsw  f28, __reg_f28_OFFSET(sp)
	fsw  f29, __reg_f29_OFFSET(sp)
	fsw  f30, __reg_f30_OFFSET(sp)
	fsw  f31, __reg_f31_OFFSET(sp)
#endif
// 	addi sp, sp, -128
// 	sw   t0, __reg_x5_OFFSET(sp)

// 	/* disable HPE */
// //    li   t0,    0x20
// //    csrs 0x804, t0

// 	csrr t0, mstatus
// 	sw   t0, __reg_mstatus_OFFSET(sp)
// 	csrr t0, mepc
// 	sw   t0, __reg_mepc_OFFSET(sp)

//     sw 	 x1, __reg_x1_OFFSET(sp)
//     sw   x3, __reg_x3_OFFSET(sp)
//     sw   x4, __reg_x4_OFFSET(sp)

//     sw   x6, __reg_x6_OFFSET(sp)
//     sw   x7, __reg_x7_OFFSET(sp)
//     sw   x8, __reg_x8_OFFSET(sp)
//     sw   x9, __reg_x9_OFFSET(sp)
//     sw   x10, __reg_x10_OFFSET(sp)
//     sw   x11, __reg_x11_OFFSET(sp)
//     sw   x12, __reg_x12_OFFSET(sp)
//     sw   x13, __reg_x13_OFFSET(sp)
//     sw   x14, __reg_x14_OFFSET(sp)
//     sw   x15, __reg_x15_OFFSET(sp)
//     sw   x16, __reg_x16_OFFSET(sp)
//     sw   x17, __reg_x17_OFFSET(sp)
//     sw   x18, __reg_x18_OFFSET(sp)
//     sw   x19, __reg_x19_OFFSET(sp)
//     sw   x20, __reg_x20_OFFSET(sp)
//     sw   x21, __reg_x21_OFFSET(sp)
//     sw   x22, __reg_x22_OFFSET(sp)
//     sw   x23, __reg_x23_OFFSET(sp)
//     sw   x24, __reg_x24_OFFSET(sp)
//     sw   x25, __reg_x25_OFFSET(sp)
//     sw   x26, __reg_x26_OFFSET(sp)
//     sw   x27, __reg_x27_OFFSET(sp)
//     sw   x28, __reg_x28_OFFSET(sp)
//     sw   x29, __reg_x29_OFFSET(sp)
//     sw   x30, __reg_x30_OFFSET(sp)
//     sw   x31, __reg_x31_OFFSET(sp)

// 	/* switch to irq stk */
//     mv t0, sp
//     lw sp, k_irq_stk_top     	/* cpu_init�м�ȥһ���ֿռ� */
//     sw t0, 0(sp)
//     /* clear software interrupt */
// 	call   sw_clearpend
// 	/* resume sp */
// 	lw sp, 0(sp)

//     la t0, k_curr_task         // t0 = &k_curr_task
//     la t1, k_next_task         // t1 = &k_next_task
//     // save sp to k_curr_task.sp
//     lw t2, (t0)
//     sw sp, (t2)

//     # switch task
//     # k_curr_task = k_next_task
//     lw t1, (t1)
//     sw t1, (t0)
//     # load new task sp
//     lw sp, (t1)

// 	/* new thread restore */
// 	lw t0, __reg_mstatus_OFFSET(sp)
// 	csrw mstatus, t0
// 	lw t0, __reg_mepc_OFFSET(sp)
// 	csrw mepc, t0

    RESTORE_BY_OFFSET(x1, __reg_x1_OFFSET);
    RESTORE_BY_OFFSET(x3, __reg_x3_OFFSET);
    RESTORE_BY_OFFSET(x4, __reg_x4_OFFSET);
    RESTORE_BY_OFFSET(x5, __reg_x5_OFFSET);
    RESTORE_BY_OFFSET(x6, __reg_x6_OFFSET);
    RESTORE_BY_OFFSET(x7, __reg_x7_OFFSET);
    RESTORE_BY_OFFSET(x8, __reg_x8_OFFSET);
    RESTORE_BY_OFFSET(x9, __reg_x9_OFFSET);
    RESTORE_BY_OFFSET(x10, __reg_x10_OFFSET);
    RESTORE_BY_OFFSET(x11, __reg_x11_OFFSET);
    RESTORE_BY_OFFSET(x12, __reg_x12_OFFSET);
    RESTORE_BY_OFFSET(x13, __reg_x13_OFFSET);
    RESTORE_BY_OFFSET(x14, __reg_x14_OFFSET);
    RESTORE_BY_OFFSET(x15, __reg_x15_OFFSET);
    RESTORE_BY_OFFSET(x16, __reg_x16_OFFSET);
    RESTORE_BY_OFFSET(x17, __reg_x17_OFFSET);
    RESTORE_BY_OFFSET(x18, __reg_x18_OFFSET);
    RESTORE_BY_OFFSET(x19, __reg_x19_OFFSET);
    RESTORE_BY_OFFSET(x20, __reg_x20_OFFSET);
    RESTORE_BY_OFFSET(x21, __reg_x21_OFFSET);
    RESTORE_BY_OFFSET(x22, __reg_x22_OFFSET);
    RESTORE_BY_OFFSET(x23, __reg_x23_OFFSET);
    RESTORE_BY_OFFSET(x24, __reg_x24_OFFSET);
    RESTORE_BY_OFFSET(x25, __reg_x25_OFFSET);
    RESTORE_BY_OFFSET(x26, __reg_x26_OFFSET);
    RESTORE_BY_OFFSET(x27, __reg_x27_OFFSET);
    RESTORE_BY_OFFSET(x28, __reg_x28_OFFSET);
    RESTORE_BY_OFFSET(x29, __reg_x29_OFFSET);
    RESTORE_BY_OFFSET(x30, __reg_x30_OFFSET);
    RESTORE_BY_OFFSET(x31, __reg_x31_OFFSET);

    SP_ADDI(128);

#ifdef __riscv_f
    RESTORE_BY_OFFSET(f0, __reg_f0_OFFSET);
    RESTORE_BY_OFFSET(f1, __reg_f1_OFFSET);
    RESTORE_BY_OFFSET(f2, __reg_f2_OFFSET);
    RESTORE_BY_OFFSET(f3, __reg_f3_OFFSET);
    RESTORE_BY_OFFSET(f4, __reg_f4_OFFSET);
    RESTORE_BY_OFFSET(f5, __reg_f5_OFFSET);
    RESTORE_BY_OFFSET(f6, __reg_f6_OFFSET);
    RESTORE_BY_OFFSET(f7, __reg_f7_OFFSET);
    RESTORE_BY_OFFSET(f8, __reg_f8_OFFSET);
    RESTORE_BY_OFFSET(f9, __reg_f9_OFFSET);
    RESTORE_BY_OFFSET(f10, __reg_f10_OFFSET);
    RESTORE_BY_OFFSET(f11, __reg_f11_OFFSET);
    RESTORE_BY_OFFSET(f12, __reg_f12_OFFSET);
    RESTORE_BY_OFFSET(f13, __reg_f13_OFFSET);
    RESTORE_BY_OFFSET(f14, __reg_f14_OFFSET);
    RESTORE_BY_OFFSET(f15, __reg_f15_OFFSET);
    RESTORE_BY_OFFSET(f16, __reg_f16_OFFSET);
    RESTORE_BY_OFFSET(f17, __reg_f17_OFFSET);
    RESTORE_BY_OFFSET(f18, __reg_f18_OFFSET);
    RESTORE_BY_OFFSET(f19, __reg_f19_OFFSET);
    RESTORE_BY_OFFSET(f20, __reg_f20_OFFSET);
    RESTORE_BY_OFFSET(f21, __reg_f21_OFFSET);
    RESTORE_BY_OFFSET(f22, __reg_f22_OFFSET);
    RESTORE_BY_OFFSET(f23, __reg_f23_OFFSET);
    RESTORE_BY_OFFSET(f24, __reg_f24_OFFSET);
    RESTORE_BY_OFFSET(f25, __reg_f25_OFFSET);
    RESTORE_BY_OFFSET(f26, __reg_f26_OFFSET);
    RESTORE_BY_OFFSET(f27, __reg_f27_OFFSET);
    RESTORE_BY_OFFSET(f28, __reg_f28_OFFSET);
    RESTORE_BY_OFFSET(f29, __reg_f29_OFFSET);
    RESTORE_BY_OFFSET(f30, __reg_f30_OFFSET);
    RESTORE_BY_OFFSET(f31, __reg_f31_OFFSET);

    SP_ADDI(128);
#endif
}

}