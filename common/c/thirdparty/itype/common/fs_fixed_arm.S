#ifdef __arm__

@ Copyright (C) 2008-2009 Monotype Imaging Inc. All rights reserved.
@ Confidential information of Monotype Imaging Inc.
@ fs_fixed_arm.S


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ GNU AS assembly file
@ fixed point arithmetic for Arm 
@ adheres to the Arm Proceedure Calling Standard
@ swp 8/16/2008
@ revised, swp 3/4/2010
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    .file   "fs_fixed_arm.S"
    .text
    .align  2
    
@
@ unsigned 64/32 divide
@ uint ux_div(uint hi, uint lo, uint divisor);
@
@ on entry:
@ r0 = high 32 bits of dividend
@ r1 = low 32 bits of dividend
@ r2 = divisor
@ msb of (r0) and (r2) must be clear
@ r2 must be non-zero
@
@ on return:
@ r0 = quotient, 0x7FFFFFFF on overflow
@ r2 = divisor
@ r3 is preserved
@
    .global ux_div
    .type   ux_div, %function
ux_div:
    @ set r12 to 12*{0,4,8,12,16,20,24 or 28} 
    @ and shift (r0:r1) << r12
    @ takes 16 cycles + branch later
    mov     r12,#0
    @ can we skip 16 iterations ?
    cmp     r0,r2,lsr #16
    movlo   r0,r0,lsl #16
    orrlo   r0,r0,r1,lsr #16
    movlo   r1,r1,lsl #16
    addlo   r12,r12,#16*12
    @ can we skip 8 (more) iterations ?
    cmp     r0,r2,lsr #8
    movlo   r0,r0,lsl #8
    orrlo   r0,r0,r1,lsr #24
    movlo   r1,r1,lsl #8
    addlo   r12,r12,#8*12
    @ can we skip 4 (more) iterations ?
    cmp     r0,r2,lsr #4
    movlo   r0,r0,lsl #4
    orrlo   r0,r0,r1,lsr #28
    movlo   r1,r1,lsl #4
    addlo   r12,r12,#4*12
    
    @ negate divisor
    rsb     r2,r2,#0

    @ initialize quotient
    adds    r1,r1,r1

    @ skip (r12) bytes
    add     pc,pc,r12
    nop     @ pc is offset by 4
    
    @ 32 divide iterations
    .rept 32
    adcs    r0,r2,r0,lsl #1
    rsbcc   r0,r2,r0
    adcs    r1,r1,r1
    .endr
    
    @ check for overflow(s)
    rsb     r2,r2,#0
    cmp     r0,r2
    mvnhs   r1,#-2147483648             @ if remainder>divisor, overflow
    mvn     r2,#-2147483648
    cmp     r1,r2
    mvnhs   r1,#-2147483648             @ if quotient>max, overflow

    @ move quotient to r0, and return
    mov     r0,r1
    mov     pc,lr
    .size   ux_div, .-ux_div
    
@
@ int varmul_asm(int a, int b, int n)
@
    .global varmul_asm
    .type   varmul_asm, %function
varmul_asm:
    @ push tmps
    stmfd   sp!, {r4, r5}

    @ sign of result
    eor     r3,r0,r1
    mov     r4,#0

    @ make args positive
    cmp     r0,#0
    beq     0f                          @ 0*any == 0
    rsblt   r0,r0,#0
    cmp     r1,#0
    beq     0f                          @ any*0 == 0
    rsblt   r1,r1,#0

    @ (r5,r4) := r0*r1
    umull   r4,r5,r0,r1

    @ (r5,r4) += (1 << (n-1))
    sub     r0,r2,#1
    mov     r1,#1
    adds    r4,r4,r1,lsl r0
    adc     r5,r5,#0

    @ (r5,r4) >>= n
    rsb     r0,r2,#32
    mov     r4,r4,lsr r2
    orr     r4,r4,r5,lsl r0
    movs    r5,r5,lsr r2                @ set flags for r5

    @ test for overflow
    mvn     r0,#-2147483648
    movne   r4,r0                       @ if r5>0, overflow
    cmp     r4,r0
    movcs   r4,r0                       @ if r4>r0, overflow

0:  @ correct sign of result
    mov     r0,r4
    cmp     r3,#0
    rsbmi   r0,r0,#0

    @ pop regs and return               
    ldmfd   sp!, {r4, r5}
    mov     pc,lr
    .size   varmul_asm, .-varmul_asm

@
@ int vardiv_asm(register int a, register int b, register int n)
@
    .global vardiv_asm
    .type   vardiv_asm, %function
vardiv_asm:
    @ push tmps and return address
    stmfd   sp!, {r4,r5,r6,lr}

    @ sign of result
    eor     r3,r0,r1
    
    @ check args
    cmp     r0,#0
    beq     1f                          @ if a==0, result=0
    rsbmi   r0,r0,#0                    @ if a<0, a= -a;
    cmp     r1,#0
    mvneq   r0,#-2147483648             @ if b==0, result = OVERFLOW
    beq     1f
    rsbmi   r1,r1,#0

    @ (hi:lo) = r0 << r2
    rsb     r4,r2,#32
    mov     r5,r0,lsr r4                @ hi
    mov     r6,r0,lsl r2                @ lo

    @ (hi:lo) += r1/2
    adds    r6,r6,r1,lsr #1
    adc     r5,r5,#0

    @ do the divide
    mov     r2,r1                       @ divisor
    mov     r1,r6                       @ lo
    mov     r0,r5                       @ hi
    bl      ux_div

1:  @ correct the sign of the result and return
    cmp     r3,#0
    rsbmi   r0,r0,#0
    ldmfd   sp!, {r4, r5, r6, pc}
    .size   vardiv_asm, .-vardiv_asm

@
@ int muldiv_asm(register int a, register int b, register int c)
@
    .align  2
    .global muldiv_asm
    .type   muldiv_asm, %function
muldiv_asm:
    @ push tmps and return address
    stmfd   sp!, {r4, r5, lr}

    @ sign of result
    eor     r3,r0,r1
    eor     r3,r3,r2

    @ check args
    cmp     r2,#0
    rsbmi   r2,r2,#0
    mvneq   r0,#-2147483648             @ divide by 0 ?
    beq     2f

    cmp     r1,#0
    moveq   r0,#0                       @ if r1==0, r0=0
    beq     2f                          @ if r1==0, return
    rsbmi   r1,r1,#0

    cmp     r0,#0
    beq     2f                          @ if r0==0, return
    rsbmi   r0,r0,#0

    @ (r4,r5) = r0*r1
    umull   r5,r4,r0,r1

    @ (r4:r5) += r2/2
    adds    r5,r5,r2,lsr #1
    adc     r4,r4,#0

    @ do the divide
    mov     r0,r4
    mov     r1,r5 
    bl      ux_div

2:  @ correct sign of result and return
    cmp     r3,#0
    rsbmi   r0,r0,#0
    ldmfd   sp!, {r4, r5, pc}
    .size   muldiv_asm, .-muldiv_asm

    .ident  "GCC: (GNU) 3.3.5 (Debian 1:3.3.5-13)"

#endif /* __arm__ */
