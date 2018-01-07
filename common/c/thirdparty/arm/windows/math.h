/*
 * math.h: ANSI 'C' (X3J11 Oct 88) library header, section 4.5
 * Copyright (C) Codemist Ltd., 1988
 * Copyright 1991-1998,2004 ARM Limited. All rights reserved
 */

/*
 * RCS $Revision: 98606 $ Codemist 0.03
 * Checkin $Date: 2006-03-02 13:51:13 +0000 (Thu, 02 Mar 2006) $
 * Revising $Author: jdou $
 */

/*
 * Parts of this file are based upon fdlibm:
 *
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#ifndef __math_h
#define __math_h

#define _ARMABI __declspec(__nothrow)
#define _ARMABI_SOFTFP __declspec(__nothrow) __softfp
#define _ARMABI_PURE __declspec(__nothrow) __pure

#ifdef __cplusplus
#define _ARMABI_INLINE inline
#elif defined(__GNUC__)
#define _ARMABI_INLINE static __inline
#else
#define _ARMABI_INLINE __inline
#endif

/*
 * Macros for our inline functions down below.
 * unsigned& __FLT(float x) - returns the bit pattern of x
 * unsigned& __HI(double x) - returns the bit pattern of the high part of x
 *                            (high part has exponent & sign bit in it)
 * unsigned& __LO(double x) - returns the bit pattern of the low part of x
 *
 * We can assign to __FLT, __HI, and __LO and the appropriate bits get set in
 * the floating point variable used.
 *
 * __HI & __LO are affected by the endianness and the target FPU.
 */
#define __FLT(x) (*(unsigned *)&(x))
#ifdef __BIG_ENDIAN
#  define __LO(x) (*(1 + (unsigned *)&(x)))
#  define __HI(x) (*(unsigned *)&(x))
#else /* ndef __BIG_ENDIAN */
#  define __HI(x) (*(1 + (unsigned *)&(x)))
#  define __LO(x) (*(unsigned *)&(x))
#endif /* ndef __BIG_ENDIAN */

#   ifndef __MATH_DECLS
#   define __MATH_DECLS


/*
 * A set of functions that we don't actually want to put in the standard
 * namespace ever.  These are all called by the C99 macros.  As they're
 * not specified by any standard they can't belong in ::std::.  The
 * macro #defines are below amongst the standard function declarations.
 * We only include these if we actually need them later on
 */
#if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)
#   ifdef __cplusplus
      extern "C" {
#   endif /* __cplusplus */

extern __softfp unsigned __ARM_dcmp4(double /*x*/, double /*y*/);
extern __softfp unsigned __ARM_fcmp4(float /*x*/, float /*y*/);
    /*
     * Compare x and y and return the CPSR in r0.  These means we can test for
     * result types with bit pattern matching.
     *
     * These are a copy of the declarations in rt_fp.h keep in sync.
     */

extern _ARMABI_SOFTFP int __ARM_fpclassifyf(float /*x*/);
extern _ARMABI_SOFTFP int __ARM_fpclassify(double /*x*/);
    /* Classify x into NaN, infinite, normal, subnormal, zero */
    /* Used by fpclassify macro */

_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_isfinitef(float __x)
{
    return ((__FLT(__x) >> 23) & 0xff) != 0xff;
}
_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_isfinite(double __x)
{
    return ((__HI(__x) >> 20) & 0x7ff) != 0x7ff;
}
    /* Return 1 if __x is finite, 0 otherwise */
    /* Used by isfinite macro */

_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_isinff(float __x)
{
    return (__FLT(__x) << 1) == 0xff000000;
}
_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_isinf(double __x)
{
    return ((__HI(__x) << 1) == 0xffe00000) && (__LO(__x) == 0);
}
    /* Return 1 if __x is infinite, 0 otherwise */
    /* Used by isinf macro */

_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_islessgreaterf(float __x, float __y)
{
    unsigned __f = __ARM_fcmp4(__x, __y) >> 28;
    return (__f == 8) || (__f == 2); /* Just N set or Just Z set */
}
_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_islessgreater(double __x, double __y)
{
    unsigned __f = __ARM_dcmp4(__x, __y) >> 28;
    return (__f == 8) || (__f == 2); /* Just N set or Just Z set */
}
    /*
     * Compare __x and __y and return 1 if __x < __y or __x > __y, 0 otherwise
     * Used by islessgreater macro
     */

_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_isnanf(float __x)
{
    return (0x7f800000 - (__FLT(__x) & 0x7fffffff)) >> 31;
}
_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_isnan(double __x)
{
    unsigned __xf = __HI(__x) | ((__LO(__x) == 0) ? 0 : 1);
    return (0x7ff00000 - (__xf & 0x7fffffff)) >> 31;
}
    /* Return 1 if __x is a NaN, 0 otherwise */
    /* Used by isnan macro */

_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_isnormalf(float __x)
{
    unsigned __xe = (__FLT(__x) >> 23) & 0xff;
    return (__xe != 0xff) && (__xe != 0);
}
_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_isnormal(double __x)
{
    unsigned __xe = (__HI(__x) >> 20) & 0x7ff;
    return (__xe != 0x7ff) && (__xe != 0);
}
    /* Return 1 if __x is a normalised number, 0 otherwise */
    /* used by isnormal macro */

_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_signbitf(float __x)
{
    return __FLT(__x) >> 31;
}
_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_signbit(double __x)
{
    return __HI(__x) >> 31;
}
    /* Return signbit of __x */
    /* Used by signbit macro */

#   ifdef __cplusplus
      } /* extern "C" */
#   endif /* __cplusplus */
#endif /* Strict ANSI */

#   undef __CLIBNS

#   ifdef __cplusplus
      namespace std {
#       define __CLIBNS ::std::
        extern "C" {
#   else
#       define __CLIBNS
#   endif  /* __cplusplus */


#if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)
  /* C99 additions */
  typedef float float_t;
  typedef double double_t;
#   ifdef __FP_FAST
#       define FLT_EVAL_METHOD (-1)
#   else
#       define FLT_EVAL_METHOD 0
#   endif
#   define HUGE_VALF ((float)__INFINITY__)
#   define HUGE_VALL ((long double)__INFINITY__)
#   define INFINITY ((float)__INFINITY__)
#   define NAN (__ESCAPE__(0f_7FC00000))
#endif
#define HUGE_VAL ((double)__INFINITY__)

extern _ARMABI double acos(double /*x*/);
   /* computes the principal value of the arc cosine of x */
   /* a domain error occurs for arguments not in the range -1 to 1 */
   /* Returns: the arc cosine in the range 0 to Pi. */
extern _ARMABI double asin(double /*x*/);
   /* computes the principal value of the arc sine of x */
   /* a domain error occurs for arguments not in the range -1 to 1 */
   /* and -HUGE_VAL is returned. */
   /* Returns: the arc sine in the range -Pi/2 to Pi/2. */

extern _ARMABI_PURE double atan(double /*x*/);
   /* computes the principal value of the arc tangent of x */
   /* Returns: the arc tangent in the range -Pi/2 to Pi/2. */

extern _ARMABI double atan2(double /*y*/, double /*x*/);
   /* computes the principal value of the arc tangent of y/x, using the */
   /* signs of both arguments to determine the quadrant of the return value */
   /* a domain error occurs if both args are zero, and -HUGE_VAL returned. */
   /* Returns: the arc tangent of y/x, in the range -Pi to Pi. */

extern _ARMABI_PURE double cos(double /*x*/);
   /* computes the cosine of x (measured in radians). A large magnitude */
   /* argument may yield a result with little or no significance */
   /* Returns: the cosine value. */
extern _ARMABI_PURE double sin(double /*x*/);
   /* computes the sine of x (measured in radians). A large magnitude */
   /* argument may yield a result with little or no significance */
   /* Returns: the sine value. */

extern void __use_accurate_range_reduction(void);
   /* reference this to select the larger, slower, but more accurate */
   /* range reduction in sin, cos and tan */

extern _ARMABI double tan(double /*x*/);
   /* computes the tangent of x (measured in radians). A large magnitude */
   /* argument may yield a result with little or no significance */
   /* Returns: the tangent value. */
   /*          if range error; returns HUGE_VAL. */

extern _ARMABI double cosh(double /*x*/);
   /* computes the hyperbolic cosine of x. A range error occurs if the */
   /* magnitude of x is too large. */
   /* Returns: the hyperbolic cosine value. */
   /*          if range error; returns HUGE_VAL. */
extern _ARMABI double sinh(double /*x*/);
   /* computes the hyperbolic sine of x. A range error occurs if the */
   /* magnitude of x is too large. */
   /* Returns: the hyperbolic sine value. */
   /*          if range error; returns -HUGE_VAL or HUGE_VAL depending */
   /*          on the sign of the argument */

extern _ARMABI_PURE double tanh(double /*x*/);
   /* computes the hyperbolic tangent of x. */
   /* Returns: the hyperbolic tangent value. */

extern _ARMABI double exp(double /*x*/);
   /* computes the exponential function of x. A range error occurs if the */
   /* magnitude of x is too large. */
   /* Returns: the exponential value. */
   /*          if underflow range error; 0 is returned. */
   /*          if overflow range error; HUGE_VAL is returned. */

extern _ARMABI double frexp(double /*value*/, int * /*exp*/);
   /* breaks a floating-point number into a normalised fraction and an */
   /* integral power of 2. It stores the integer in the int object pointed */
   /* to by exp. */
   /* Returns: the value x, such that x is a double with magnitude in the */
   /* interval 0.5 to 1.0 or zero, and value equals x times 2 raised to the */
   /* power *exp. If value is zero, both parts of the result are zero. */

extern _ARMABI double ldexp(double /*x*/, int /*exp*/);
   /* multiplies a floating-point number by an integral power of 2. */
   /* A range error may occur. */
   /* Returns: the value of x times 2 raised to the power of exp. */
   /*          if range error; HUGE_VAL is returned. */
extern _ARMABI double log(double /*x*/);
   /* computes the natural logarithm of x. A domain error occurs if the */
   /* argument is negative, and -HUGE_VAL is returned. A range error occurs */
   /* if the argument is zero. */
   /* Returns: the natural logarithm. */
   /*          if range error; -HUGE_VAL is returned. */
extern _ARMABI double log10(double /*x*/);
   /* computes the base-ten logarithm of x. A domain error occurs if the */
   /* argument is negative. A range error occurs if the argument is zero. */
   /* Returns: the base-ten logarithm. */
extern _ARMABI double modf(double /*value*/, double * /*iptr*/);
   /* breaks the argument value into integral and fraction parts, each of */
   /* which has the same sign as the argument. It stores the integral part */
   /* as a double in the object pointed to by iptr. */
   /* Returns: the signed fractional part of value. */

extern _ARMABI double pow(double /*x*/, double /*y*/);
   /* computes x raised to the power of y. A domain error occurs if x is */
   /* zero and y is less than or equal to zero, or if x is negative and y */
   /* is not an integer, and -HUGE_VAL returned. A range error may occur. */
   /* Returns: the value of x raised to the power of y. */
   /*          if underflow range error; 0 is returned. */
   /*          if overflow range error; HUGE_VAL is returned. */
extern _ARMABI double sqrt(double /*x*/);
   /* computes the non-negative square root of x. A domain error occurs */
   /* if the argument is negative, and -HUGE_VAL returned. */
   /* Returns: the value of the square root. */
#ifdef __TARGET_FPU_VFP
    /* these functions should expand inline as the native VFP square root
     * instructions. They will not behave like the C sqrt() function, because
     * they will report unusual values as IEEE exceptions (in fpmodes which
     * support IEEE exceptions) rather than in errno. These function names
     * are not specified in any standard. */
    extern _ARMABI_PURE double _sqrt(double);
    extern _ARMABI_PURE float _sqrtf(float);
#else
    _ARMABI_INLINE double _sqrt(double __x) { return sqrt(__x); }
    _ARMABI_INLINE float _sqrtf(float __x) { return (float)sqrt(__x); }
#endif

extern _ARMABI_PURE double ceil(double /*x*/);
   /* computes the smallest integer not less than x. */
   /* Returns: the smallest integer not less than x, expressed as a double. */
extern _ARMABI_PURE double fabs(double /*x*/);
   /* computes the absolute value of the floating-point number x. */
   /* Returns: the absolute value of x. */

extern _ARMABI_PURE double floor(double /*d*/);
   /* computes the largest integer not greater than x. */
   /* Returns: the largest integer not greater than x, expressed as a double */

extern _ARMABI double fmod(double /*x*/, double /*y*/);
   /* computes the floating-point remainder of x/y. */
   /* Returns: the value x - i * y, for some integer i such that, if y is */
   /*          nonzero, the result has the same sign as x and magnitude */
   /*          less than the magnitude of y. If y is zero, a domain error */
   /*          occurs and -HUGE_VAL is returned. */

    /* Additional Mathlib functions not defined by the ANSI standard.
     * Not guaranteed, and not necessarily very well tested.
     * C99 requires the user to include <math.h> to use these functions
     * declaring them "by hand" is not sufficient
     *
     * The above statement is not completely true now.  Some of the above
     * C99 functionality has been added as per the Standard, and (where
     * necessary) old Mathlib functionality withdrawn/changed.  Before
     * including this header #define __ENABLE_MATHLIB_LEGACY if you want to
     * re-enable the legacy functionality.
     */

#if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)

extern _ARMABI double acosh(double /*x*/);
    /*
     * Inverse cosh. EDOM if argument < 1.0
     */
extern _ARMABI double asinh(double /*x*/);
    /*
     * Inverse sinh.
     */
extern _ARMABI double atanh(double /*x*/);
    /*
     * Inverse tanh. EDOM if |argument| > 1.0
     */
extern _ARMABI double cbrt(double /*x*/);
    /*
     * Cube root.
     */
_ARMABI_INLINE _ARMABI_SOFTFP double copysign(double __x, double __y)
    /*
     * Returns x with sign bit replaced by sign of y.
     */
{
    __HI(__x) = (__HI(__x) & 0x7fffffff) | (__HI(__y) & 0x80000000);
    return __x;
}
_ARMABI_INLINE _ARMABI_SOFTFP float copysignf(float __x, float __y)
    /*
     * Returns x with sign bit replaced by sign of y.
     */
{
    __FLT(__x) = (__FLT(__x) & 0x7fffffff) | (__FLT(__y) & 0x80000000);
    return __x;
}
extern _ARMABI double erf(double /*x*/);
    /*
     * Error function. (2/sqrt(pi)) * integral from 0 to x of exp(-t*t) dt.
     */
extern _ARMABI double erfc(double /*x*/);
    /*
     * 1-erf(x). (More accurate than just coding 1-erf(x), for large x.)
     */
extern _ARMABI double expm1(double /*x*/);
    /*
     * exp(x)-1. (More accurate than just coding exp(x)-1, for small x.)
     */
#define fpclassify(x) \
    ((sizeof(x) == sizeof(float)) ? \
        __ARM_fpclassifyf(x) : __ARM_fpclassify(x))
    /*
     * Classify a floating point number into one of the following values:
     */
#define FP_ZERO         (0)
#define FP_SUBNORMAL    (4)
#define FP_NORMAL       (5)
#define FP_INFINITE     (3)
#define FP_NAN          (7)

extern _ARMABI double gamma(double /*x*/);
    /*
     * The log of the absolute value of the gamma function of x. The sign
     * of the gamma function of x is returned in the global `signgam'.
     */
extern _ARMABI double gamma_r(double /*x*/, int * /*signgam*/);
    /*
     * The log of the absolute value of the gamma function of x. The sign
     * of the gamma function of x is returned in the second argument.
     */
extern _ARMABI double hypot(double /*x*/, double /*y*/);
    /*
     * sqrt(x*x+y*y), ie the length of the vector (x,y) or the
     * hypotenuse of a right triangle whose other two sides are x
     * and y. Won't overflow unless the _answer_ is too big, even
     * if the intermediate x*x+y*y is too big.
     */
extern _ARMABI_SOFTFP int ilogb(double /*x*/);
    /*
     * Exponent of x (returns 0 for 1.0, 1 for 2.0, -1 for 0.5, etc.)
     */
extern _ARMABI_SOFTFP int ilogbf(float /*x*/);
    /*
     * Like ilogb but takes a float
     */
extern _ARMABI_SOFTFP int ilogbl(long double /*x*/);
    /*
     * Exponent of x (returns 0 for 1.0, 1 for 2.0, -1 for 0.5, etc.)
     */
#define FP_ILOGB0   (-0x7fffffff) /* ilogb(0) == -INT_MAX */
#define FP_ILOGBNAN ( 0x80000000) /* ilogb(NAN) == INT_MIN */

#define isfinite(x) \
    ((sizeof(x) == sizeof(float)) \
        ? __ARM_isfinitef(x) \
        : __ARM_isfinite(x))
    /*
     * Returns true if x is a finite number, size independent.
     */

#define isgreater(x, y) \
    (((sizeof(x) == sizeof(float)) && (sizeof(y) == sizeof(float))) \
        ? ((__ARM_fcmp4((x), (y)) & 0xf0000000) == 0x20000000) \
        : ((__ARM_dcmp4((x), (y)) & 0xf0000000) == 0x20000000))
    /*
     * Returns true if x > y, throws no exceptions except on Signaling NaNs
     *
     * We want the C not set but the Z bit clear, V must be clear
     */

#define isgreaterequal(x, y) \
    (((sizeof(x) == sizeof(float)) && (sizeof(y) == sizeof(float))) \
        ? ((__ARM_fcmp4((x), (y)) & 0x30000000) == 0x20000000) \
        : ((__ARM_dcmp4((x), (y)) & 0x30000000) == 0x20000000))
    /*
     * Returns true if x >= y, throws no exceptions except on Signaling NaNs
     *
     * We just need to see if the C bit is set or not and ensure V clear
     */

#define isinf(x) \
    ((sizeof(x) == sizeof(float)) \
        ? __ARM_isinff(x) \
        : __ARM_isinf(x))
    /*
     * Returns true if x is an infinity, size independent.
     */

#define isless(x, y)  \
    (((sizeof(x) == sizeof(float)) && (sizeof(y) == sizeof(float))) \
        ? ((__ARM_fcmp4((x), (y)) & 0xf0000000) == 0x80000000) \
        : ((__ARM_dcmp4((x), (y)) & 0xf0000000) == 0x80000000))
    /*
     * Returns true if x < y, throws no exceptions except on Signaling NaNs
     *
     * We're less than if N is set, V clear
     */

#define islessequal(x, y) \
    (((sizeof(x) == sizeof(float)) && (sizeof(y) == sizeof(float))) \
        ? ((__ARM_fcmp4((x), (y)) & 0xc0000000) != 0) \
        : ((__ARM_dcmp4((x), (y)) & 0xc0000000) != 0))
    /*
     * Returns true if x <= y, throws no exceptions except on Signaling NaNs
     *
     * We're less than or equal if one of N or Z is set, V clear
     */

#define islessgreater(x, y) \
    (((sizeof(x) == sizeof(float)) && (sizeof(y) == sizeof(float))) \
        ? __ARM_islessgreaterf((x), (y)) \
        : __ARM_islessgreater((x), (y)))
    /*
     * Returns true if x <> y, throws no exceptions except on Signaling NaNs
     * Unfortunately this test is too complicated to do in a macro without
     * evaluating x & y twice.  Shame really...
     */

#define isnan(x) \
    ((sizeof(x) == sizeof(float)) \
        ? __ARM_isnanf(x) \
        : __ARM_isnan(x))
    /*
     * Returns TRUE if x is a NaN.
     */

#define isnormal(x) \
    ((sizeof(x) == sizeof(float)) \
        ? __ARM_isnormalf(x) \
        : __ARM_isnormal(x))
    /*
     * Returns TRUE if x is a NaN.
     */

#define isunordered(x, y) \
    (((sizeof(x) == sizeof(float)) && (sizeof(y) == sizeof(float))) \
        ? ((__ARM_fcmp4((x), (y)) & 0x10000000) == 0x10000000) \
        : ((__ARM_dcmp4((x), (y)) & 0x10000000) == 0x10000000))
    /*
     * Returns true if x ? y, throws no exceptions except on Signaling NaNs
     * Unordered occurs if and only if the V bit is set
     */

extern _ARMABI double j0(double /*x*/);
    /*
     * Bessel function of the first kind, order 0. Returns ERANGE
     * if |x| > 2^52 (total loss of significance).
     */
extern _ARMABI double j1(double /*x*/);
    /*
     * Bessel function of the first kind, order 1. Returns ERANGE
     * if |x| > 2^52 (total loss of significance).
     */
extern _ARMABI double jn(int /*n*/, double /*x*/);
    /*
     * Bessel function of the first kind, order n. Returns ERANGE
     * if |x| > 2^52 (total loss of significance).
     */
extern _ARMABI double lgamma (double /*x*/);
    /*
     * The log of the absolute value of the gamma function of x. The sign
     * of the gamma function of x is returned in the global `signgam'.
     */
extern _ARMABI double lgamma_r(double /*x*/, int * /*signgam*/);
    /*
     * The log of the absolute value of the gamma function of x. The sign
     * of the gamma function of x is returned in the second argument.
     */
extern _ARMABI double log1p(double /*x*/);
    /*
     * log(1+x). (More accurate than just coding log(1+x), for small x.)
     */
extern _ARMABI_SOFTFP double logb(double /*x*/);
    /*
     * Like ilogb but returns a double.
     */
extern _ARMABI_SOFTFP float logbf(float /*x*/);
    /*
     * Like logb but takes and returns float
     */
extern _ARMABI_SOFTFP long double logbl(long double /*x*/);
    /*
     * Like logb but takes and returns long double
     */
extern _ARMABI_SOFTFP double nextafter(double /*x*/, double /*y*/);
    /*
     * Returns the next representable number after x, in the
     * direction toward y.
     */
extern _ARMABI_SOFTFP float nextafterf(float /*x*/, float /*y*/);
    /*
     * Returns the next representable number after x, in the
     * direction toward y.
     */
extern _ARMABI_SOFTFP long double nextafterl(long double /*x*/, long double /*y*/);
    /*
     * Returns the next representable number after x, in the
     * direction toward y.
     */
extern _ARMABI_SOFTFP double nexttoward(double /*x*/, long double /*y*/);
    /*
     * Returns the next representable number after x, in the
     * direction toward y.
     */
extern _ARMABI_SOFTFP float nexttowardf(float /*x*/, long double /*y*/);
    /*
     * Returns the next representable number after x, in the
     * direction toward y.
     */
extern _ARMABI_SOFTFP long double nexttowardl(long double /*x*/, long double /*y*/);
    /*
     * Returns the next representable number after x, in the
     * direction toward y.
     */
extern _ARMABI double remainder(double /*x*/, double /*y*/);
    /*
     * Returns the remainder of x by y, in the IEEE 754 sense.
     */
extern _ARMABI double rint(double /*x*/);
    /*
     * Rounds x to an integer, in the IEEE 754 sense.
     */
#ifndef __STRICT_ANSI__
extern _ARMABI double scalb(double /*x*/, double /*n*/);
    /*
     * Compute x times 2^n quickly. Undefined if n is not an integer.
     * Not part of C99, please use scalb[l]n[f|l] instead
     */
#endif
extern _ARMABI_SOFTFP double scalbln(double /*x*/, long int /*n*/);
    /*
     * Compute x times 2^n quickly.
     */
extern _ARMABI_SOFTFP float scalblnf(float /*x*/, long int /*n*/);
    /*
     * Compute x times 2^n quickly.
     */
extern _ARMABI_SOFTFP long double scalblnl(long double /*x*/, long int /*n*/);
    /*
     * Compute x times 2^n quickly.
     */
extern _ARMABI_SOFTFP double scalbn(double /*x*/, int /*n*/);
    /*
     * Compute x times 2^n quickly.
     */
extern _ARMABI_SOFTFP float scalbnf(float /*x*/, int /*n*/);
    /*
     * Compute x times 2^n quickly.
     */
extern _ARMABI_SOFTFP long double scalbnl(long double /*x*/, int /*n*/);
    /*
     * Compute x times 2^n quickly.
     */
#define signbit(x) \
    ((sizeof(x) == sizeof(float)) \
        ? __ARM_signbitf(x) \
        : __ARM_signbit(x))
    /*
     * Returns the signbit of x, size independent macro
     */
extern _ARMABI double significand(double /*x*/);
    /*
     * Return the fraction part of x, in the range 1.0 to 2.0
     * (including 1.0, excluding 2.0).
     */
extern _ARMABI double y0(double /*x*/);
    /*
     * Bessel function of the second kind, order 0. Returns ERANGE
     * if x > 2^52 (total loss of significance).
     */
extern _ARMABI double y1(double /*x*/);
    /*
     * Bessel function of the second kind, order 1. Returns ERANGE
     * if x > 2^52 (total loss of significance).
     */
extern _ARMABI double yn(int /*n*/, double /*x*/);
    /*
     * Bessel function of the second kind, order n. Returns ERANGE
     * if x > 2^52 (total loss of significance).
     */
#endif

/* C99 float versions of functions.  math.h has always reserved these
   identifiers for this purpose (7.13.4). */
extern _ARMABI_PURE float _fabsf(float); /* old ARM name */
_ARMABI_INLINE _ARMABI_PURE float fabsf(float __f) { return _fabsf(__f); }
extern _ARMABI float sinf(float /*x*/);
extern _ARMABI float cosf(float /*x*/);
extern _ARMABI float tanf(float /*x*/);
extern _ARMABI float acosf(float /*x*/);
extern _ARMABI float asinf(float /*x*/);
extern _ARMABI float atanf(float /*x*/);
extern _ARMABI float atan2f(float /*y*/, float /*x*/);
extern _ARMABI float sinhf(float /*x*/);
extern _ARMABI float coshf(float /*x*/);
extern _ARMABI float tanhf(float /*x*/);
extern _ARMABI float expf(float /*x*/);
extern _ARMABI float logf(float /*x*/);
extern _ARMABI float log10f(float /*x*/);
extern _ARMABI float powf(float /*x*/, float /*y*/);
extern _ARMABI float sqrtf(float /*x*/);
extern _ARMABI float ldexpf(float /*x*/, int /*exp*/);
extern _ARMABI float frexpf(float /*value*/, int * /*exp*/);
extern _ARMABI_PURE float ceilf(float /*x*/);
extern _ARMABI_PURE float floorf(float /*x*/);
extern _ARMABI float fmodf(float /*x*/, float /*y*/);
extern _ARMABI float modff(float /*value*/, float * /*iptr*/);

#ifdef __cplusplus
  extern "C++" {
    inline float abs(float __x)   { return fabsf(__x); }
    inline float acos(float __x)  { return acosf(__x); }
    inline float asin(float __x)  { return asinf(__x); }
    inline float atan(float __x)  { return atanf(__x); }
    inline float atan2(float __y, float __x)    { return atan2f(__y,__x); }
    inline float ceil(float __x)  { return ceilf(__x); }
    inline float cos(float __x)   { return cosf(__x); }
    inline float cosh(float __x)  { return coshf(__x); }
    inline float exp(float __x)   { return expf(__x); }
    inline float fabs(float __x)  { return fabsf(__x); }
    inline float floor(float __x) { return floorf(__x); }
    inline float fmod(float __x, float __y)     { return fmodf(__x, __y); }
    inline float frexp(float __x, int* __exp)   { return frexpf(__x, __exp); }
    inline float ldexp(float __x, int __exp)    { return ldexpf(__x, __exp);}
    inline float log(float __x)   { return logf(__x); }
    inline float log10(float __x) { return log10f(__x); }
    inline float modf(float __x, float* __iptr) { return modff(__x, __iptr); }
    inline float pow(float __x, float __y)      { return powf(__x,__y); }
    inline float pow(float __x, int __y)     { return powf(__x, (float)__y); }
    inline float sin(float __x)   { return sinf(__x); }
    inline float sinh(float __x)  { return sinhf(__x); }
    inline float sqrt(float __x)  { return sqrtf(__x); }
    inline float _sqrt(float __x) { return _sqrtf(__x); }
    inline float tan(float __x)   { return tanf(__x); }
    inline float tanh(float __x)  { return tanhf(__x); }

    inline double abs(double __x) { return fabs(__x); }
    inline double pow(double __x, int __y)
                { return pow(__x, (double) __y); }

    inline long double abs(long double __x) // fabsl
                { return (long double)abs((double) __x); }
    inline long double acos(long double __x) // acosl
                { return (long double)acos((double) __x); }
    inline long double asin(long double __x) // asinl
                { return (long double)asin((double) __x); }
    inline long double atan(long double __x) // atanl
                { return (long double)atan((double) __x); }
    inline long double atan2(long double __y, long double __x)  //atan2l
                { return (long double)atan2((double) __y, (double) __x); }
    inline long double ceil(long double __x) // ceill
                { return (long double)ceil((double) __x); }
    inline long double cos(long double __x) // cosl
                { return (long double)cos((double) __x); }
    inline long double cosh(long double __x) // coshl
                { return (long double)cosh((double) __x); }
    inline long double exp(long double __x) // expl
                { return (long double)exp((double) __x); }
    inline long double fabs(long double __x) // fabsl
                { return (long double)fabs((double) __x); }
    inline long double floor(long double __x) // floorl
                { return (long double)floor((double) __x); }
    inline long double fmod(long double __x, long double __y) // fmodl
                { return (long double)fmod((double) __x, (double) __y); }
    inline long double frexp(long double __x, int* __p) // frexpl
                { return (long double)frexp((double) __x, __p); }
    inline long double ldexp(long double __x, int __exp) // ldexpl
                { return (long double)ldexp((double) __x, __exp); }
    inline long double log(long double __x)  // logl
                { return (long double)log((double) __x); }
    inline long double log10(long double __x)  // log10l
                { return (long double)log10((double) __x); }
    inline long double modf(long double __x, long double* __p) //modfl
                { return (long double)modf((double) __x, (double *) __p); }
    inline long double pow(long double __x, long double __y) // powl
                { return (long double)pow((double) __x, (double) __y); }
    inline long double pow(long double __x, int __y)
                { return (long double)pow((double) __x, __y); }
    inline long double sin(long double __x) //sinl
                { return (long double)sin((double) __x); }
    inline long double sinh(long double __x) // sinhl
                { return (long double)sinh((double) __x); }
    inline long double sqrt(long double __x) // sqrtl
                { return (long double)sqrt((double) __x); }
    inline long double _sqrt(long double __x) // _sqrtl
                { return (long double)_sqrt((double) __x); }
    inline long double tan(long double __x) // tanl
                { return (long double)tan((double) __x); }
    inline long double tanh(long double __x) // tanhl
                { return (long double)tanh((double) __x); }
  }
#endif

    #ifdef __cplusplus
        }  /* extern "C" */
      }  /* namespace std */
    #endif
  #endif /* __MATH_DECLS */

#if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)
#ifdef __ENABLE_LEGACY_MATHLIB
    /*
     * LEGACY COMPATIBILITY:
     * Legacy ilogb functions returned INT_MAX instead of INT_MIN for a NaN
     * input.  Here we override the functions declared above with macros which
     * translate the output, and redeclare FP_ILOGBNAN to return the old value.
     * Note: Not a perfect emulation of old behaviour as taking the address of
     * ilogb, ilogbf, and ilogbl will still point to the new functions.
     */
_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_legacyilogb(double __x)
{
    int __r=(__CLIBNS ilogb)(__x);
    return (__r == 0x80000000) ? 0x7fffffff : __r;
}
_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_legacyilogbf(float __x)
{
    int __r=(__CLIBNS ilogbf)(__x);
    return (__r == 0x80000000) ? 0x7fffffff : __r;
}
_ARMABI_INLINE _ARMABI_SOFTFP int __ARM_legacyilogbl(long double __x)
{
    int __r=(__CLIBNS ilogbl)(__x);
    return (__r == 0x80000000) ? 0x7fffffff : __r;
}
#define ilogb(x) __ARM_legacyilogb(x)
#define ilogbf(x) __ARM_legacyilogbf(x)
#define ilogbl(x) __ARM_legacyilogbl(x)
#undef FP_ILOGBNAN
#define FP_ILOGBNAN (0x7fffffff)

    /*
     * We used provide a variant of isfinite called finite, lets provide a
     * compatible version here.  finite(x) always assumed its parameters were
     * doubles
     */
#define finite(x) __ARM_isfinite(x)

    /*
     * The legacy isnan used to assume its arguments were doubles
     */
#undef isnan
#define isnan(x) __ARM_isnan(x)

#endif /* MATHLIB_LEGACY */
#endif

  #if _AEABI_PORTABILITY_LEVEL != 0 && !defined _AEABI_PORTABLE
    #define _AEABI_PORTABLE
  #endif

  #if defined(__cplusplus) && !defined(__MATH_NO_EXPORTS)
    using ::std::__use_accurate_range_reduction;
    using ::std::abs;
    using ::std::acos;
    using ::std::asin;
    using ::std::atan2;
    using ::std::atan;
    using ::std::ceil;
    using ::std::cos;
    using ::std::cosh;
    using ::std::exp;
    using ::std::fabs;
    using ::std::floor;
    using ::std::fmod;
    using ::std::frexp;
    using ::std::ldexp;
    using ::std::log10;
    using ::std::log;
    using ::std::modf;
    using ::std::pow;
    using ::std::sin;
    using ::std::sinh;
    using ::std::sqrt;
    using ::std::sqrtf;
    using ::std::_sqrt;
    using ::std::_sqrtf;
    using ::std::tan;
    using ::std::tanh;
    using ::std::_fabsf;
    #if !defined(__STRICT_ANSI__) || (defined(__STDC_VERSION__) && 199901L <= __STDC_VERSION__)
      /* C99 additions */
      using ::std::float_t;
      using ::std::double_t;
      using ::std::acosh;
      using ::std::asinh;
      using ::std::atanh;
      using ::std::cbrt;
      using ::std::copysign;
      using ::std::copysignf;
      using ::std::erf;
      using ::std::erfc;
      using ::std::expm1;
      using ::std::gamma;
      using ::std::gamma_r;
      using ::std::hypot;
      using ::std::ilogb;
      using ::std::ilogbf;
      using ::std::ilogbl;
      using ::std::j0;
      using ::std::j1;
      using ::std::jn;
      using ::std::lgamma;
      using ::std::lgamma_r;
      using ::std::log1p;
      using ::std::logb;
      using ::std::logbf;
      using ::std::logbl;
      using ::std::nextafter;
      using ::std::nextafterf;
      using ::std::nextafterl;
      using ::std::nexttoward;
      using ::std::nexttowardf;
      using ::std::nexttowardl;
      using ::std::remainder;
      using ::std::rint;
      using ::std::scalb;
      using ::std::scalbln;
      using ::std::scalblnf;
      using ::std::scalblnl;
      using ::std::scalbn;
      using ::std::scalbnf;
      using ::std::scalbnl;
      using ::std::significand;
      using ::std::y0;
      using ::std::y1;
      using ::std::yn;
      /* C99 float versions */
      using ::std::fabsf;
      using ::std::sinf;
      using ::std::cosf;
      using ::std::tanf;
      using ::std::acosf;
      using ::std::asinf;
      using ::std::atanf;
      using ::std::atan2f;
      using ::std::sinhf;
      using ::std::coshf;
      using ::std::tanhf;
      using ::std::expf;
      using ::std::logf;
      using ::std::log10f;
      using ::std::powf;
      using ::std::ldexpf;
      using ::std::frexpf;
      using ::std::fmodf;
      using ::std::modff;
      using ::std::ceilf;
      using ::std::floorf;
    #endif
  #endif

#endif /* __math_h */

/* end of math.h */
