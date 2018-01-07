; rt_memory.s: template for writing a memory model implementation
;
; Copyright 1999 ARM Limited. All rights reserved.
;
; RCS $Revision: 92593 $
; Checkin $Date: 2005-09-21 16:36:20 +0100 (Wed, 21 Sep 2005) $
; Revising $Author: statham $

; This module gives a framework for writing custom memory models
; in. It actually implements a very simple memory model, in which:
;
;  - the stack is 64Kb and grows down from 0x80000000
;  - the heap is 128Kb and grows up from 0x60000000
;  - a stack overflow is instantly fatal and does not even raise SIGSTAK
;  - no extra memory is available for the heap

        IMPORT  __rt_stackheap_storage

        AREA    |.text|, CODE, READONLY

; --------------------------------------------------

        ; Initialisation function. This routine isn't required to
        ; preserve any registers, although obviously it shouldn't
        ; lose LR because it won't know where to return to. On
        ; exit, SP and SL should describe a usable stack, and
        ; [a1,a2) should be the initial block to use for the heap.
        ;
        ; Returning a1==a2 is allowed.

        EXPORT  __rt_stackheap_init
__rt_stackheap_init

        ; Set up some example values.
        MOV     sp, #0x80000000
        SUB     sl, sp, #0x10000
        MOV     a1, #0x60000000
        ADD     a2, a1, #0x20000

        MOV     pc, lr

; --------------------------------------------------

        ; The stack checking function. This is called from the
        ; entry sequence of any routine that needs to check its
        ; stack usage. If stack checking is not enabled, this
        ; function is not needed at all.

        ; If the stack check is uneventful, this function will need
        ; to return. It should return with _all_ registers
        ; preserved as they were on entry, except that it is
        ; allowed to corrupt IP.

        ; On entry, SP contains the new requested stack pointer.
        ; Since this may be invalid, the initial check against the
        ; stack limit must be done without storing anything on the
        ; stack _or_ corrupting any register other than IP.

        IMPORT  __rt_raise
        IMPORT  __rt_exit
        IMPORT  _sys_exit

        EXPORT  __ARM_stackcheck
__ARM_stackcheck

        ; Check SP against the stack limit, using only IP.
        LDR     ip, =__stack_limit
        LDR     ip, [ip]
        CMP     ip, sp
        BXMI    lr                      ; or MOVMI pc,lr on non-Thumb CPUs

        ; SP is beyond the stack limit. Swap SP with IP so that SP
        ; is set _to_ the stack limit. Provided the memory model
        ; has ensured there is some spare space beyond the actual
        ; limit, this will allow the rest of this function to save
        ; registers on the stack.
        SUB     ip, sp, ip
        SUB     sp, sp, ip
        ADD     ip, sp, ip

        ; Drop sp by 192 bytes, since the calling code might have
        ; stored that much data beyond the stack limit.
        SUB     sp, sp, #192

        ; Now insert user stack-overflow handling code here. If the
        ; stack overflow is recoverable, this code should start by
        ; stacking some registers in the temporary stack area; it
        ; then needs to adjust __stack_limit and restore all
        ; registers and the input value of SP before returning.

        ; This stub implementation just exits immediately.
        MOV     a1, #100
        B       _sys_exit

; --------------------------------------------------

        ; The heap extend function. This is called using the
        ; ordinary ATPCS calling convention.
        ;
        ; On input: a1 is the minimum size of memory required
        ;              (guaranteed to be a multiple of 4)
        ;           a2 is a pointer to a word of memory, W, in which
        ;              to store the address of the block
        ;
        ; On exit:  a1 is size of returned block
        ;           W  contains base address of returned block
        ;      Or:  a1 is zero
        ;           W  has undefined contents

        EXPORT  __rt_heap_extend
__rt_heap_extend

        ; Refuse all requests for extra heap space.
        MOV     a1, #0

        LDMFD   sp!, {pc}

; --------------------------------------------------

        ; The longjmp cleanup function. This is called using the
        ; ordinary ATPCS calling convention.
        ;
        ; At minimum, this routine is required to set {SL,SP} to
        ; the values in {a1,a2}. It must set them _atomically_
        ; (i.e. in a single LDM instruction) so that an interrupt
        ; cannot occur between loading one and loading the other.
        ;
        ; This routine is not called unless software stack checking
        ; is enabled.

        EXPORT  __rt_stack_postlongjmp
__rt_stack_postlongjmp

        ; Perform only the minimum required processing.
        STMFD   sp!, {a1,a2}
        LDMIA   sp, {sl,sp}             ; notice absence of ! to avoid
                                        ;   undefined behaviour in LDM
        MOV     pc, lr

; --------------------------------------------------

        ; A place to store the stack limit. This will need to be
        ; accessed in a more complicated way if compiling ROPI or
        ; FPIC; alternatively, it can be omitted (along with
        ; __ARM_stackcheck) if not using stack checking.

        AREA    |.bss|, DATA, NOINIT
__stack_limit % 4

        END
