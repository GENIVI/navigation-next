/* rt_fp.h: prototypes for the fplib basic-arithmetic functions
 *
 * Copyright 1999 ARM Limited. All rights reserved.
 *
 * RCS $Revision: 91721 $
 * Checkin $Date: 2005-08-08 19:30:07 +0100 (Mon, 08 Aug 2005) $
 * Revising $Author: drodgman $
 */

#ifndef __RT_FP_H
#define __RT_FP_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * For the benefit of the VFP support library, we allow a module
 * including this file to define a name-mapping macro `_PREFIX()',
 * which transforms a name such as `_fadd' into something like
 * `vfp_fadd'.
 */
#ifndef _PREFIX
#define _PREFIX(x) x
#endif

/*
 * Trivial routines.
 */
extern __softfp float _PREFIX(_fneg)(float);
extern __softfp float _PREFIX(__aeabi_fneg)(float);
extern __softfp float _PREFIX(_fabs)(float);
extern __softfp double _PREFIX(_dneg)(double);
extern __softfp double _PREFIX(__aeabi_dneg)(double);
extern __softfp double _PREFIX(_dabs)(double);

/*
 * Single-precision arithmetic routines.
 */
extern __softfp float _PREFIX(_fadd)(float, float);
extern __softfp float _PREFIX(__aeabi_fadd)(float, float);
extern __softfp float _PREFIX(_fsub)(float, float);
extern __softfp float _PREFIX(__aeabi_fsub)(float, float);
extern __softfp float _PREFIX(_frsb)(float, float);
extern __softfp float _PREFIX(__aeabi_frsub)(float, float);
extern __softfp float _PREFIX(_fmul)(float, float);
extern __softfp float _PREFIX(__aeabi_fmul)(float, float);
extern __softfp float _PREFIX(_fdiv)(float, float);
extern __softfp float _PREFIX(__aeabi_fdiv)(float, float);
extern __softfp float _PREFIX(_frdiv)(float, float);
extern __softfp float _PREFIX(_frem)(float, float);
extern __softfp float _PREFIX(_frnd)(float);
extern __softfp float _PREFIX(_fsqrt)(float);

/*
 * Double-precision arithmetic routines.
 */
extern __softfp double _PREFIX(_dadd)(double, double);
extern __softfp double _PREFIX(__aeabi_dadd)(double, double);
extern __softfp double _PREFIX(_dsub)(double, double);
extern __softfp double _PREFIX(__aeabi_dsub)(double, double);
extern __softfp double _PREFIX(_drsb)(double, double);
extern __softfp double _PREFIX(__aeabi_drsub)(double, double);
extern __softfp double _PREFIX(_dmul)(double, double);
extern __softfp double _PREFIX(__aeabi_dmul)(double, double);
extern __softfp double _PREFIX(_ddiv)(double, double);
extern __softfp double _PREFIX(__aeabi_ddiv)(double, double);
extern __softfp double _PREFIX(_drdiv)(double, double);
extern __softfp double _PREFIX(_drem)(double, double);
extern __softfp double _PREFIX(_drnd)(double);
extern __softfp double _PREFIX(_dsqrt)(double);

/*
 * Boolean-return compares ({f,d}cmp{eq,ge,le} are not listed
 * because they return results in the flags and so cannot be
 * described by C prototypes).
 */
extern __softfp int _PREFIX(_feq)(float, float);
extern __softfp int _PREFIX(__aeabi_fcmpeq)(float, float);
extern __softfp int _PREFIX(_fneq)(float, float);
extern __softfp int _PREFIX(_fgeq)(float, float);
extern __softfp int _PREFIX(__aeabi_fcmpge)(float, float);
extern __softfp int _PREFIX(_fgr)(float, float);
extern __softfp int _PREFIX(__aeabi_fcmpgt)(float, float);
extern __softfp int _PREFIX(_fleq)(float, float);
extern __softfp int _PREFIX(__aeabi_fcmple)(float, float);
extern __softfp int _PREFIX(_fls)(float, float);
extern __softfp int _PREFIX(__aeabi_fcmplt)(float, float);
extern __softfp int _PREFIX(_fun)(float, float);
extern __softfp int _PREFIX(__aeabi_fcmpun)(float, float);
extern __softfp int _PREFIX(_deq)(double, double);
extern __softfp int _PREFIX(__aeabi_dcmpeq)(double, double);
extern __softfp int _PREFIX(_dneq)(double, double);
extern __softfp int _PREFIX(_dgeq)(double, double);
extern __softfp int _PREFIX(__aeabi_dcmpge)(double, double);
extern __softfp int _PREFIX(_dgr)(double, double);
extern __softfp int _PREFIX(__aeabi_dcmpgt)(double, double);
extern __softfp int _PREFIX(_dleq)(double, double);
extern __softfp int _PREFIX(__aeabi_dcmple)(double, double);
extern __softfp int _PREFIX(_dls)(double, double);
extern __softfp int _PREFIX(__aeabi_dcmplt)(double, double);
extern __softfp int _PREFIX(_dun)(double, double);
extern __softfp int _PREFIX(__aeabi_dcmpun)(double, double);

/*
 * Four-way compares. These return VFP-type status flags _in_ the
 * CPSR, but also return in r0 so they can be called sensibly from
 * C.
 */
extern __softfp unsigned _PREFIX(_fcmp4)(float, float);
extern __softfp unsigned _PREFIX(_fcmp4e)(float, float);
extern __softfp unsigned _PREFIX(_dcmp4)(double, double);
extern __softfp unsigned _PREFIX(_dcmp4e)(double, double);
extern __softfp unsigned _PREFIX(_fdcmp4)(float, double);
extern __softfp unsigned _PREFIX(_fdcmp4e)(float, double);
extern __softfp unsigned _PREFIX(_dfcmp4)(double, float);
extern __softfp unsigned _PREFIX(_dfcmp4e)(double, float);

/*
 * Floating-to-floating format conversions.
 */
extern __softfp double _PREFIX(_f2d)(float);
extern __softfp double _PREFIX(__aeabi_f2d)(float);
extern __softfp float _PREFIX(_d2f)(double);
extern __softfp float _PREFIX(__aeabi_d2f)(double);

/*
 * Integer-to-floating format conversions.
 */
extern __softfp float _PREFIX(_fflt)(int);
extern __softfp float _PREFIX(__aeabi_i2f)(int);
extern __softfp float _PREFIX(_ffltu)(unsigned int);
extern __softfp float _PREFIX(__aeabi_ui2f)(unsigned int);
extern __softfp double _PREFIX(_dflt)(int);
extern __softfp double _PREFIX(__aeabi_i2d)(int);
extern __softfp double _PREFIX(_dfltu)(unsigned int);
extern __softfp double _PREFIX(__aeabi_ui2d)(unsigned int);
#if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)
extern __softfp float _PREFIX(_ll_sto_f)(long long);
extern __softfp float _PREFIX(__aeabi_l2f)(long long);
extern __softfp float _PREFIX(_ll_uto_f)(unsigned long long);
extern __softfp float _PREFIX(__aeabi_ul2f)(unsigned long long);
extern __softfp double _PREFIX(_ll_sto_d)(long long);
extern __softfp double _PREFIX(__aeabi_l2d)(long long);
extern __softfp double _PREFIX(_ll_uto_d)(unsigned long long);
extern __softfp double _PREFIX(__aeabi_ul2d)(unsigned long long);
#endif /* __STRICT_ANSI__ */

/*
 * Floating-to-integer format conversions, rounding toward zero
 * always.
 */
extern __softfp int _PREFIX(_ffix)(float);
extern __softfp int _PREFIX(__aeabi_f2iz)(float);
extern __softfp unsigned int _PREFIX(_ffixu)(float);
extern __softfp unsigned int _PREFIX(__aeabi_f2uiz)(float);
extern __softfp int _PREFIX(_dfix)(double);
extern __softfp int _PREFIX(__aeabi_d2iz)(double);
extern __softfp unsigned int _PREFIX(_dfixu)(double);
extern __softfp unsigned int _PREFIX(__aeabi_d2uiz)(double);
#if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)
extern __softfp long long _PREFIX(_ll_sfrom_f)(float);
extern __softfp long long _PREFIX(__aeabi_f2lz)(float);
extern __softfp unsigned long long _PREFIX(_ll_ufrom_f)(float);
extern __softfp unsigned long long _PREFIX(__aeabi_f2ulz)(float);
extern __softfp long long _PREFIX(_ll_sfrom_d)(double);
extern __softfp long long _PREFIX(__aeabi_d2lz)(double);
extern __softfp unsigned long long _PREFIX(_ll_ufrom_d)(double);
extern __softfp unsigned long long _PREFIX(__aeabi_d2ulz)(double);
#endif /* __STRICT_ANSI__ */

/*
 * Floating-to-integer format conversions, rounding to nearest or
 * configurably.
 */
extern __softfp int _PREFIX(_ffix_r)(float);
extern __softfp unsigned int _PREFIX(_ffixu_r)(float);
extern __softfp int _PREFIX(_dfix_r)(double);
extern __softfp unsigned int _PREFIX(_dfixu_r)(double);
#if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)
extern __softfp long long _PREFIX(_ll_sfrom_f_r)(float);
extern __softfp unsigned long long _PREFIX(_ll_ufrom_f_r)(float);
extern __softfp long long _PREFIX(_ll_sfrom_d_r)(double);
extern __softfp unsigned long long _PREFIX(_ll_ufrom_d_r)(double);
#endif /* __STRICT_ANSI__ */

/*
 * Call this before using any fplib routines, if you're trying to
 * use fplib on the bare metal.
 */
extern void _PREFIX(_fp_init)(void);

/*
 * This returns a pointer to the FP status word, when it's stored
 * in memory.
 */
extern unsigned * _PREFIX(__rt_fp_status_addr)(void);

#ifdef __cplusplus
}
#endif

#endif
