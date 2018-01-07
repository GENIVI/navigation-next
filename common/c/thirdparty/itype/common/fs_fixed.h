
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/*  Encoding:   US_ASCII    Tab Size:   8   Indentation:    4        */

/* fs_fixed.h */


#ifndef FS_FIXED_H
#define FS_FIXED_H

/* AMT supplies assembly code for MULDIV, VARMUL and VARDIV 
* for the Microsoft Visual C compiler on x86 processors -
we recommend that you enable the HAS_ASM option in these environments.

The following code can be used to enable the HAS_ASM option automatically:

#if defined(_WIN32) && defined(_M_IX86) && !defined  (__MARM_ARMI__)
#define HAS_ASM
#endif
*/

#ifdef __cplusplus
extern "C" {
#endif


#ifndef LO_WORD
#define HI_WORD(x) ((FS_ULONG)(x) >> 16)
#define LO_WORD(x) ((x) & 0xFFFF)
#endif

#define FIXED_ONE (1L << 16)
#define FRACT_ONE (1L << 30)
#define SHORTFRACT_ONE (1 << 14)

#define FIXED_ONEHALF (1L << 15)
#define FIXED_ONEFOURTH (1L << 14)
#define FRACT_ONEHALF (1L << 29)
#define SHORTFRACT_ONEHALF (1 << 13)

#define MYINFINITY  2147483647L


/* smallest half integer >= y */
#define ABOVE(y) (((y + FIXED_ONEHALF - 1) & (-FIXED_ONE)) + FIXED_ONEHALF)

/* largest half integer < y */
#define BELOW(y) (((y - FIXED_ONEHALF - 1) & (-FIXED_ONE)) + FIXED_ONEHALF)

#define FS_FLOOR(x)    (FS_SHORT)((x)>>16)
#define FS_CEIL(x)     FS_FLOOR((x)+0xFFFF)

#define FIXED_FLOOR(x)    ((x) & ~65535)        /* largest integer <= x ... as FS_FIXED */
#define FIXED_CEILING(x) (((x)+65535) & ~65535) /* smallest integer >=x ... as FS_FIXED */
#define FIXED_ROUND(x) FIXED_FLOOR(32768+(x)) 

#define FIXEDTODOT6(n)  (((n) + (1L << 9)) >> 10)

/*****************************************************************************
 *
 *  Macro that rounds a 16.16 fixed-point number to an integer.
 *      
 *  Parameters:
 *      val           - [in] 16.16 fixed-point number
 *
 *  Return Value:
 *      Result of rounding x to an FS_SHORT integer.
 *
 *  <GROUP fsfixedpoint>
 */
#define FS_ROUND(val)    FS_FLOOR((val)+FIXED_ONEHALF)

#if !defined(HAS_ASM) && !defined(HAS_FS_INT64)    
/* 64 bit integers in 32 bit ANSI C*/
ITYPE_API FS_LONG varmul(FS_LONG a, FS_LONG b, int n);        /* (a*b)>>n */
ITYPE_API FS_LONG vardiv(FS_LONG a, FS_LONG b, int n);        /* (a<<n)/b */
ITYPE_API FS_LONG muldiv(FS_LONG a, FS_LONG b, FS_LONG c);    /* (a*b)/c */
#endif

/* 64 bit integers with compiler support */
#if defined(HAS_FS_INT64) && !defined(HAS_ASM)    
ITYPE_API FS_LONG varmul_64(FS_LONG a, FS_LONG b, int n);
ITYPE_API FS_LONG vardiv_64(FS_LONG a, FS_LONG b, int n);
ITYPE_API FS_LONG muldiv_64(FS_LONG a, FS_LONG b, FS_LONG c);
#endif

/* user supplied assembly code */
#ifdef HAS_ASM
ITYPE_API FS_LONG varmul_asm(FS_LONG a, FS_LONG b, int n);
ITYPE_API FS_LONG vardiv_asm(FS_LONG a, FS_LONG b, int n);
ITYPE_API FS_LONG muldiv_asm(FS_LONG a, FS_LONG b, FS_LONG c);
#endif

/* normalize a fixed point vector */
FS_VOID fixed_norm(FIXED_VECTOR *p);

/* sqrt of 2.30 number */
FRACT FracSqrt(FRACT x);

/* sqrt of a 16.16 number */
FS_FIXED FixSqrt(FS_FIXED x);

/* ROUGH approximation to sqrt(dx*dx+dy*dy) */
FS_FIXED norm(FS_FIXED dx,FS_FIXED dy);

#define ShortFracDot(a,b) (ShortFrac)(((FS_LONG)a*b)>>14)

/* select the proper variant of MULDIV, VARMUL, and VARDIV */
#if SIZEOF_LONG==8
/* first: native 64 bit ints */
#define VarMul(a,b,c) varmul_64(a,b,c)
#define VarDiv(a,b,c) vardiv_64(a,b,c)
#define FixMul(a,b) (FS_FIXED)varmul_64(a,b,16)
#define FixDiv(a,b) (FS_FIXED)vardiv_64(a,b,16)
#define Mul26Dot6(a,b) (F26DOT6)varmul_64(a,b,6)
#define Div26Dot6(a,b) (F26DOT6)vardiv_64(a,b,6)
#define LongMulDiv(a,b,c) muldiv_64(a,b,c)
#define ShortMulDiv(a,b,c) muldiv_64(a,(FS_LONG)b,(FS_LONG)c)
#define FracMul(a,b) (FRACT)varmul_64(a,b,30)
#define FracDiv(a,b) (FRACT)vardiv_64(a,b,30)
#define ShortFracMul(a,b) (F26DOT6)varmul_64(a,(FS_LONG)b,14)

#elif defined(HAS_ASM)
/* second: user supplied assembly code */
#define VarMul(a,b,c) varmul_asm(a,b,c)
#define VarDiv(a,b,c) vardiv_asm(a,b,c)
#define FixMul(a,b) (FS_FIXED)varmul_asm(a,b,16)
#define FixDiv(a,b) (FS_FIXED)vardiv_asm(a,b,16)
#define Mul26Dot6(a,b) (F26DOT6)varmul_asm(a,b,6)
#define Div26Dot6(a,b) (F26DOT6)vardiv_asm(a,b,6)
#define LongMulDiv(a,b,c) muldiv_asm(a,b,c)
#define ShortMulDiv(a,b,c) muldiv_asm(a,(FS_LONG)b,(FS_LONG)c)
#define FracMul(a,b) (FRACT)varmul_asm(a,b,30)
#define FracDiv(a,b) (FRACT)vardiv_asm(a,b,30)
#define ShortFracMul(a,b) (F26DOT6)varmul_asm(a,(FS_LONG)b,14)

#elif defined(HAS_FS_INT64)
/* third: compiler supported 64 bit integers */
#define VarMul(a,b,c) varmul_64(a,b,c)
#define VarDiv(a,b,c) vardiv_64(a,b,c)
#define FixMul(a,b) (FS_FIXED)varmul_64(a,b,16)
#define FixDiv(a,b) (FS_FIXED)vardiv_64(a,b,16)
#define Mul26Dot6(a,b) (F26DOT6)varmul_64(a,b,6)
#define Div26Dot6(a,b) (F26DOT6)vardiv_64(a,b,6)
#define LongMulDiv(a,b,c) muldiv_64(a,b,c)
#define ShortMulDiv(a,b,c) muldiv_64(a,(FS_LONG)b,(FS_LONG)c)
#define FracMul(a,b) (FRACT)varmul_64(a,b,30)
#define FracDiv(a,b) (FRACT)vardiv_64(a,b,30)
#define ShortFracMul(a,b) (F26DOT6)varmul_64(a,(FS_LONG)b,14)

#else
/* otherwise 32 bit ANSI C */
#define VarMul(a,b,c) varmul(a,b,c)
#define VarDiv(a,b,c) vardiv(a,b,c)


/*****************************************************************************
 *
 *  Returns the product of two 16.16 fixed-point numbers.
 *      
 *  Parameters:
 *      a           - [in] 16.16 fixed-point number
 *      b           - [in] 16.16 fixed-point number
 *
 *  Return Value:
 *      Product of a*b as a 16.16 fixed-point number
 *
 *  <GROUP fsfixedpoint>
 */
#define FixMul(a,b) (FS_FIXED)varmul(a,b,16)


/*****************************************************************************
 *
 *  Returns the quotient of two 16.16 fixed-point numbers.
 *      
 *  Parameters:
 *      a           - [in] numerator as 16.16 fixed-point number
 *      b           - [in] denominator as 16.16 fixed-point number
 *
 *  Return Value:
 *      Quotient of a/b as a 16.16 fixed-point number
 *
 *  <GROUP fsfixedpoint>
 */
#define FixDiv(a,b) (FS_FIXED)vardiv(a,b,16)


#define Mul26Dot6(a,b) (F26DOT6)varmul(a,b,6)
#define Div26Dot6(a,b) (F26DOT6)vardiv(a,b,6)


/*****************************************************************************
 *
 *  Returns ((a*b)/c) of three numbers of the same radix.
 *
 *  Description:
 *      a, b, and c can be integers or FS_FIXED, but must all be of same
 *      radix.
 *      
 *  Parameters:
 *      a           - [in] integer or 16.16 fixed-point number
 *      b           - [in] integer or 16.16 fixed-point number
 *      c           - [in] integer or 16.16 fixed-point number
 *
 *  Return Value:
 *      Result of ((a*b)/c)in same radix as a, b, and c.
 *
 *  <GROUP fsfixedpoint>
 */
#define LongMulDiv(a,b,c) muldiv(a,b,c)


#define ShortMulDiv(a,b,c) muldiv(a,(FS_LONG)b,(FS_LONG)c)
#define FracMul(a,b) (FRACT)varmul(a,b,30)
#define FracDiv(a,b) (FRACT)vardiv(a,b,30)
#define ShortFracMul(a,b) (F26DOT6)varmul(a,(FS_LONG)b,14)
#endif /* SIZEOF_LONG */

#ifdef __cplusplus
    }
#endif


#endif /* FS_FIXED_H */
