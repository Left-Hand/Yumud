#include "os.hpp"
#include "offset_table.hpp"

#define STORE_REG(lhs, rhs) asm volatile("lw " #lhs ", (" #rhs ")")

// secondary liternal string
// 将宏所代表的数值进行字面量字符串化 而不是宏传入的参数本身的字符串
//https://www.cnblogs.com/LiuYanYGZ/p/10659457.html
#define __STR(R) #R
#define STR(R) __STR(R)

// 通过SP指针的偏移量载入到寄存器中
#define LOAD_BY_OFFSET(lhs, offset) asm volatile("lw " STR(lhs) ", " STR(offset) "(sp)")
#define STORE_BY_OFFSET(lhs, offset) asm volatile("sw " STR(lhs) ", " STR(offset) "(sp)")

// 通过SP指针和宏定义的寄存器对应的偏移量载入到寄存器中
#define LOAD(lhs) asm volatile("lw " STR(lhs) ", " STR(__reg_ ## lhs ## _OFFSET) "(sp)")
#define STORE(lhs) asm volatile("sw " STR(lhs) ", " STR(__reg_ ## lhs ## _OFFSET) "(sp)")

#define SP_ADDI(offset) asm volatile("addi sp, sp, " #offset)
#define CSRW(lhs, rhs) asm volatile("csrw " #lhs ", " #rhs)
#define MRET asm volatile("mret")

namespace RiscvCpu{


void restore_context(void){

    LOAD_BY_OFFSET(t0, __reg_mepc_OFFSET);
    CSRW(mepc, t0);
    LOAD_BY_OFFSET(t0, __reg_mstatus_OFFSET);
    CSRW(mstatus, t0);

    LOAD(x1);
    LOAD(x3);
    LOAD(x4);
    LOAD(x5);
    LOAD(x6);
    LOAD(x7);
    LOAD(x8);
    LOAD(x9);
    LOAD(x10);
    LOAD(x11);
    LOAD(x12);
    LOAD(x13);
    LOAD(x14);
    LOAD(x15);
    LOAD(x16);
    LOAD(x17);
    LOAD(x18);
    LOAD(x19);
    LOAD(x20);
    LOAD(x21);
    LOAD(x22);
    LOAD(x23);
    LOAD(x24);
    LOAD(x25);
    LOAD(x26);
    LOAD(x27);
    LOAD(x28);
    LOAD(x29);
    LOAD(x30);
    LOAD(x31);

    SP_ADDI(128);

#ifdef __riscv_f
    LOAD(f1);
    LOAD(f2);
    LOAD(f3);
    LOAD(f4);
    LOAD(f5);
    LOAD(f6);
    LOAD(f7);
    LOAD(f8);
    LOAD(f9);
    LOAD(f10);
    LOAD(f11);
    LOAD(f12);
    LOAD(f13);
    LOAD(f14);
    LOAD(f15);
    LOAD(f16);
    LOAD(f17);
    LOAD(f18);
    LOAD(f19);
    LOAD(f20);
    LOAD(f21);
    LOAD(f22);
    LOAD(f23);
    LOAD(f24);
    LOAD(f25);
    LOAD(f26);
    LOAD(f27);
    LOAD(f28);
    LOAD(f29);
    LOAD(f30);
    LOAD(f31);

    SP_ADDI(128);
#endif
    MRET;
    __builtin_unreachable();
}

}

using namespace RiscvCpu;

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

    LOAD(x1);
    LOAD(x3);
    LOAD(x4);
    LOAD(x5);
    LOAD(x6);
    LOAD(x7);
    LOAD(x8);
    LOAD(x9);
    LOAD(x10);
    LOAD(x11);
    LOAD(x12);
    LOAD(x13);
    LOAD(x14);
    LOAD(x15);
    LOAD(x16);
    LOAD(x17);
    LOAD(x18);
    LOAD(x19);
    LOAD(x20);
    LOAD(x21);
    LOAD(x22);
    LOAD(x23);
    LOAD(x24);
    LOAD(x25);
    LOAD(x26);
    LOAD(x27);
    LOAD(x28);
    LOAD(x29);
    LOAD(x30);
    LOAD(x31);

    SP_ADDI(128);

#ifdef __riscv_f
    LOAD(f1);
    LOAD(f2);
    LOAD(f3);
    LOAD(f4);
    LOAD(f5);
    LOAD(f6);
    LOAD(f7);
    LOAD(f8);
    LOAD(f9);
    LOAD(f10);
    LOAD(f11);
    LOAD(f12);
    LOAD(f13);
    LOAD(f14);
    LOAD(f15);
    LOAD(f16);
    LOAD(f17);
    LOAD(f18);
    LOAD(f19);
    LOAD(f20);
    LOAD(f21);
    LOAD(f22);
    LOAD(f23);
    LOAD(f24);
    LOAD(f25);
    LOAD(f26);
    LOAD(f27);
    LOAD(f28);
    LOAD(f29);
    LOAD(f30);
    LOAD(f31);

    SP_ADDI(128);
#endif
    MRET;
    __builtin_unreachable();
}
