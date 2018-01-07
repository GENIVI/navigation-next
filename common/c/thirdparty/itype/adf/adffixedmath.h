/**
 *-----------------------------------------------------------------------------------
 *    Filename: ADFFixedMath.h
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Copyright 2004-2007 Mitsubishi Electric Research Laboratories (MERL)
 *    An interface and implementation for fixed point math functions
 *    Eric Chan, Ronald Perry, and Sarah Frisken
 *-----------------------------------------------------------------------------------
 */


/**
 *-----------------------------------------------------------------------------------
 *    To avoid multiple inclusion of header files
 *-----------------------------------------------------------------------------------
 */
#ifndef ADFFIXEDMATH_H
#define ADFFIXEDMATH_H

#if defined(FS_EDGE_HINTS) || defined(FS_EDGE_RENDER)


/**
 *-----------------------------------------------------------------------------------
 *    To make functions accessible from C++ code
 *-----------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif


/**
 *-------------------------------------------------------------------------------
 *    If ADF_MATH_MODE is not defined, issue a preprocessor error
 *-------------------------------------------------------------------------------
 */
#ifndef ADF_MATH_MODE
#error "ADF_MATH_MODE must be defined in ADFTypeSystem.h."
#endif


/**
 *-------------------------------------------------------------------------------
 *    If ADF_MATH_FIXED_C_32 is not defined, issue a preprocessor error
 *-------------------------------------------------------------------------------
 */
#ifndef ADF_MATH_FIXED_C_32
#error "ADF_MATH_FIXED_C_32 must be defined in ADFTypeSystem.h"
#endif


/**
 *-------------------------------------------------------------------------------
 *    If ADF_MATH_FIXED_C_64 is not defined, issue a preprocessor error
 *-------------------------------------------------------------------------------
 */
#ifndef ADF_MATH_FIXED_C_64
#error "ADF_MATH_FIXED_C_64 must be defined in ADFTypeSystem.h"
#endif


/**
 *-------------------------------------------------------------------------------
 *    If ADF_MATH_FIXED_ASM_X86 is not defined, issue a preprocessor error
 *-------------------------------------------------------------------------------
 */
#ifndef ADF_MATH_FIXED_ASM_X86
#error "ADF_MATH_FIXED_ASM_X86 must be defined in ADFTypeSystem.h"
#endif


/**
 *-------------------------------------------------------------------------------
 *    If ADF_MATH_MODE is invalid, issue a preprocessor error
 *-------------------------------------------------------------------------------
 */
#if ((ADF_MATH_MODE != ADF_MATH_FIXED_C_32) &&        \
(ADF_MATH_MODE != ADF_MATH_FIXED_C_64) &&        \
(ADF_MATH_MODE != ADF_MATH_FIXED_ASM_X86))        
#error "ADF_MATH_MODE is set to an invalid value."                                      
#endif

    
/**
 *-------------------------------------------------------------------------------
 *    START: FIXED POINT MATH ONLY
 *-------------------------------------------------------------------------------
 */

/**
 *-------------------------------------------------------------------------------
 *    If ADF_INLINE is not defined in ADFTypeSystem.h, issue a preprocessor error
 *-------------------------------------------------------------------------------
 */
#ifndef ADF_INLINE
#error "ADF_INLINE must be defined in ADFTypeSystem.h."
#endif

/**
 *-----------------------------------------------------------------------------------
 *    USEFUL MATHEMATICAL MACROS
 *-----------------------------------------------------------------------------------
 */
#define ADF_ABS(a)      (((a) <   0) ? -(a) : (a))
#define ADF_MIN(a,b)    (((a) < (b)) ?  (a) : (b))
#define ADF_MAX(a,b)    (((a) > (b)) ?  (a) : (b))
#define I32_TO_I1616(a) ((a) << 16)
/**
 *-----------------------------------------------------------------------------------
 *    TERMINOLOGY
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    The documentation in this file and ADFFixedMath.c uses the following terminology.
 *    In a N-bit integer, the least significant bit (i.e., the LSB) is bit 0 and the
 *    most significant bit (i.e., the MSB) is bit N-1. Binary representations are
 *    written with the MSB to the left and the LSB to the right. For example, the
 *    binary representation of a 32-bit unsigned integer with the mathematical value 1
 *    is 00000000000000000000000000000001, and the binary representation of a 32-bit
 *    unsigned integer with the mathematical value 2147483648 (hexadecimal 0x80000000)
 *    is 10000000000000000000000000000000.
 *
 *    A range of consecutive bits is denoted by M:L, where M is the most significant
 *    bit of the range and L is the least significant bit. For example, bits 15:0 of a
 *    32-bit integer are the 16 least significant bits of the integer, and bits 23:8
 *    are the middle 16 bits of the integer.
 *
 *    The most significant bits of an integer are called the "high" bits and the least
 *    significant bits of an integer are called the "low" bits. For example, bits 15:0
 *    of a 32-bit integer are called the low 16 bits, and bits 31:16 are called the
 *    high 16 bits.
 *
 *    A fixed point number with I integer bits and F fractional bits is called an I.F
 *    fixed point number. For example, a 32-bit fixed point number with 24 integer bits
 *    and 8 fractional bits is called a 24.8 fixed point number. Fixed point numbers
 *    are unsigned unless stated otherwise.
 *-----------------------------------------------------------------------------------
 */

/**
 *-----------------------------------------------------------------------------------
 *    FIXED POINT DATA TYPES 
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    An ADF_I1616 is a 32-bit two's complement signed fixed point data type with 1
 *    sign bit, 15 integer bits, and 16 fractional bits. The MSB (i.e., bit 31) is the
 *    sign bit, bits 30:16 are the integer bits, and bits 15:0 are the fractional bits.
 *    An ADF_I1616 value R represents the mathematical value (R / 65536.0).
 *
 *    Example 1:
 *    ADF_I1616 hexadecimal value:       0x00000000
 *    ADF_I1616 binary representation:   0 000000000000000 0000000000000000 (LSB)
 *    Mathematical value:                   0
 *
 *    Example 2:
 *    ADF_I1616 hexadecimal value:       0x00010000
 *    ADF_I1616 binary representation:   0 000000000000001 0000000000000000 (LSB)
 *    Mathematical value:                   1
 *
 *    Example 3:
 *    ADF_I1616 hexadecimal value:       0xffff0000
 *    ADF_I1616 binary representation:   1 111111111111111 0000000000000000 (LSB)
 *    Mathematical value:                   -1
 *
 *    Example 4:
 *    ADF_I1616 hexadecimal value:       0x0000ffff
 *    ADF_I1616 binary representation:   0 000000000000000 1111111111111111 (LSB)
 *    Mathematical value:                   65535 / 65536
 *
 *    Example 5:
 *    ADF_I1616 hexadecimal value:       0x00000001
 *    ADF_I1616 binary representation:   0 000000000000000 0000000000000001 (LSB)
 *    Mathematical value:                   1 / 65536
 *    Remark:                               minimum representable positive value
 *
 *    Example 6:
 *    ADF_I1616 hexadecimal value:       0xffffffff
 *    ADF_I1616 binary representation:   1 111111111111111 1111111111111111 (LSB)
 *    Mathematical value:                   -1 / 65536
 *    Remark:                               maximum representable negative value
 *
 *    Example 7:
 *    ADF_I1616 hexadecimal value:       0x7fffffff
 *    ADF_I1616 binary representation:   0 111111111111111 1111111111111111 (LSB)
 *    Mathematical value:                   32767 + (65535 / 65536)
 *    Remark:                               maximum representable value
 *
 *    Example 8:
 *    ADF_I1616 hexadecimal value:       0x80000000
 *    ADF_I1616 binary representation:   1 000000000000000 0000000000000000 (LSB)
 *    Mathematical value:                   -32768
 *    Remark:                               minimum representable value
 *
 *    Example 9:
 *    ADF_I1616 hexadecimal value:       0x0003243f
 *    ADF_I1616 binary representation:   0 000000000000011 0010010000111111 (LSB)
 *    Mathematical value:                   3.1415863037109375
 *    Remark:                               approximation to pi
 *-----------------------------------------------------------------------------------
 */
typedef ADF_I32 ADF_I1616;
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    An ADF_U0032 is an unsigned fixed point data type with 32 fractional bits. An
 *    ADF_U0032 value R represents the mathematical value (R / 4294967296.0) (i.e., (R
 *    / (2 ^ 32))).
 *
 *    Example 1:
 *    ADF_U0032 hexadecimal value:       0x00000000
 *    ADF_U0032 binary representation:   00000000000000000000000000000000 (LSB)
 *    Mathematical value:                   0
 *    Remark:                               minimum representable value
 *
 *    Example 2:
 *    ADF_U0032 hexadecimal value:       0x00000001
 *    ADF_U0032 binary representation:   00000000000000000000000000000001 (LSB)
 *    Mathematical value:                   1 / 4294967296
 *    Remark:                               minimum representable positive value
 *
 *    Example 3:
 *    ADF_U0032 hexadecimal value:       0xffffffff
 *    ADF_U0032 binary representation:   11111111111111111111111111111111 (LSB)
 *    Mathematical value:                   4294967295 / 4294967296
 *    Remark:                               maximum representable value
 *
 *    Example 4:
 *    ADF_U0032 hexadecimal value:       0x80000000
 *    ADF_U0032 binary representation:   10000000000000000000000000000000 (LSB)
 *    Mathematical value:                   0.5
 *-----------------------------------------------------------------------------------
 */
typedef ADF_U32 ADF_U0032;
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    An ADF_I2408 is a 32-bit two's complement signed fixed point data type with 1
 *    sign bit, 23 integer bits, and 8 fractional bits. The MSB (i.e., bit 31) is the
 *    sign bit, bits 30:8 are the integer bits, and bits 7:0 are the fractional bits.
 *    An ADF_I2408 value R represents the mathematical value (R / 256.0).
 *
 *    Example 1:
 *    ADF_I2408 hexadecimal value:       0x00000000
 *    ADF_I2408 binary representation:   0 00000000000000000000000 00000000 (LSB)
 *    Mathematical value:                   0
 *
 *    Example 2:
 *    ADF_I2408 hexadecimal value:       0x00000100
 *    ADF_I2408 binary representation:   0 00000000000000000000001 00000000 (LSB)
 *    Mathematical value:                   1
 *
 *    Example 3:
 *    ADF_I2408 hexadecimal value:       0xffffff00
 *    ADF_I2408 binary representation:   1 11111111111111111111111 00000000 (LSB)
 *    Mathematical value:                   -1
 *
 *    Example 4:
 *    ADF_I2408 hexadecimal value:       0x000000ff
 *    ADF_I2408 binary representation:   0 00000000000000000000000 11111111 (LSB)
 *    Mathematical value:                   255 / 256
 *
 *    Example 5:
 *    ADF_I2408 hexadecimal value:       0x00000001
 *    ADF_I2408 binary representation:   0 00000000000000000000000 00000001 (LSB)
 *    Mathematical value:                   1 / 256
 *    Remark:                               minimum representable positive value
 *
 *    Example 6:
 *    ADF_I2408 hexadecimal value:       0xffffffff
 *    ADF_I2408 binary representation:   1 11111111111111111111111 11111111 (LSB)
 *    Mathematical value:                   -1 / 256
 *    Remark:                               maximum representable negative value
 *
 *    Example 7:
 *    ADF_I2408 hexadecimal value:       0x7fffffff
 *    ADF_I2408 binary representation:   0 11111111111111111111111 11111111 (LSB)
 *    Mathematical value:                   8388607 + (255 / 256)
 *    Remark:                               maximum representable value
 *
 *    Example 8:
 *    ADF_I2408 hexadecimal value:       0x80000000
 *    ADF_I2408 binary representation:   1 00000000000000000000000 00000000 (LSB)
 *    Mathematical value:                   -8388608
 *    Remark:                               minimum representable value
 *
 *    Example 9:
 *    ADF_I2408 hexadecimal value:       0x0003243f
 *    ADF_I2408 binary representation:   0 00000000000000000000011 00100100 (LSB)
 *    Mathematical value:                   3.140625
 *    Remark:                               approximation to pi
 *-----------------------------------------------------------------------------------
 */
typedef ADF_I32 ADF_I2408;


/**
 *-----------------------------------------------------------------------------------
 *    ADF_I1616 FIXED POINT CONSTANTS
 *-----------------------------------------------------------------------------------
 */
#define I1616_CONST_1            ((ADF_I1616) 0x00010000) /*     1.0000000000000000 */
#define I1616_CONST_NEG_1        ((ADF_I1616) 0xfffe0000) /*    -1.0000000000000000 */
#define I1616_CONST_NEG_20       ((ADF_I1616) 0xffec0000) /*   -20.0000000000000000 */
#define I1616_CONST_NEG_128      ((ADF_I1616) 0xff800000) /*  -128.0000000000000000 */
#define I1616_CONST_NEG_512      ((ADF_I1616) 0xfe000000) /*  -512.0000000000000000 */
#define I1616_CONST_NEG_2048     ((ADF_I1616) 0xf8000000) /* -2048.0000000000000000 */
#define I1616_CONST_3            ((ADF_I1616) 0x00030000) /*     3.0000000000000000 */
#define I1616_CONST_20           ((ADF_I1616) 0x00140000) /*    20.0000000000000000 */
#define I1616_CONST_80           ((ADF_I1616) 0x00500000) /*    80.0000000000000000 */
#define I1616_CONST_108          ((ADF_I1616) 0x006c0000) /*   108.0000000000000000 */
#define I1616_CONST_128          ((ADF_I1616) 0x00800000) /*   128.0000000000000000 */
#define I1616_CONST_ONE_HALF     ((ADF_I1616) 0x00008000) /*     0.5000000000000000 */
#define I1616_CONST_ONE_THIRD    ((ADF_I1616) 0x00005555) /*     0.3333282470703125 */
#define I1616_CONST_ONE_SIXTH    ((ADF_I1616) 0x00002aaa) /*     0.1666564941406250 */
#define I1616_CONST_ONE_FIFTH    ((ADF_I1616) 0x00003333) /*     0.1999969482421875 */
#define I1616_CONST_FIVE_THIRDS  ((ADF_I1616) 0x0001aaaa) /*     1.6666564941406250 */
#define I1616_CONST_FOUR_FIFTHS  ((ADF_I1616) 0x0000cccc) /*     0.7999877929687500 */
#define I1616_CONST_EPS          ((ADF_I1616) 0x00000002) /*     0.0000305175781250 */
#define I1616_CONST_DELTA        ((ADF_I1616) 0x00000001) /*     0.0000152587890625 */
#define I1616_CONST_PI           ((ADF_I1616) 0x0003243f) /*     3.1415863037109375 */
#define I1616_CONST_TWO_PI       ((ADF_I1616) 0x0006487e) /*     6.2831726074218750 */
#define I1616_CONST_HALF_PI      ((ADF_I1616) 0x0001921f) /*     1.5707855224609375 */
#define I1616_CONST_TWO_OVER_PI  ((ADF_I1616) 0x0000a2f9) /*     0.6366119384765625 */
#define I1616_CONST_SQRT5        ((ADF_I1616) 0x00023c6e) /*     2.2360534667968750 */


/**
 *-----------------------------------------------------------------------------------
 *    ADF_I2408 FIXED POINT CONSTANTS
 *-----------------------------------------------------------------------------------
 */
#define I2408_CONST_32             ((ADF_I2408) 0x00002000) /*    32.0000000000000000 */

#ifdef FS_EDGE_RENDER


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return (f * 255), where f is an ADF_U0032 fixed point value that lies
 *    in the mathematical range [0, 1) and the computed result is an 8-bit unsigned
 *    integer that lies in the range [0, 255].
 *
 *    U0032_TO_U8() is implemented as a macro. The first part of the macro (i.e., f -
 *    (f >> 8)) treats f as a 8.24 fixed point integer and evaluates ((f * 256) - f) =
 *    (f * (256 - 1)) = (f * 255). Note that converting f to an 8.24 fixed point value
 *    can be implemented by shifting f to the right by 8 bits (i.e., f >> 8).
 *    Multiplying the 8.24 fixed point value by 256 is equivalent to shifting the
 *    value to the left by 8 bits (i.e., (f >> 8) << 8). Therefore, converting f from
 *    a 0.32 fixed point value to an 8.24 fixed point value and then multiplying the
 *    result by 256 is simply f. 
 *
 *    The second part of the macro converts the 8.24 fixed point product (f * 255) to
 *    an 8-bit integer by shifting the product to the right by 24 bits.
 *
 *    The following examples omit some of the macro's parentheses to make the examples
 *    easier to follow.
 *
 *    Example 1. Consider the case where f = 0:
 *
 *        (f - (f >> 8)) >> 24 = (0 - (0 >> 8)) >> 24
 *                             = (0 - 0) >> 24
 *                             = 0 >> 24
 *                             = 0
 *
 *    Example 2. Consider the case where f = 0xffffffff (i.e., f has the mathematical
 *    value (0xffffffff / 2^32) = 0.99999999976716935634613037109375):
 *
 *        (f - (f >> 8)) >> 24 = (0xffffffff - (0xffffffff >> 8)) >> 24
 *                             = (0xffffffff - 0x00ffffff) >> 24
 *                             = 0xff000000 >> 24
 *                             = 0xff
 *                             = 255
 *
 *    Example 3. Consider the case where f = 0x80000000 (i.e., f has the mathematical
 *    value (0x80000000 / 2^32) = 0.5):
 *
 *        (f - (f >> 8)) >> 24 = (0x80000000 - (0x80000000 >> 8)) >> 24
 *                             = (0x80000000 - 0x00800000) >> 24
 *                             = 0x7f800000 >> 24
 *                             = 0x7f
 *                             = 127
 *-----------------------------------------------------------------------------------
 */
#define U0032_TO_U8(f) (((f) - ((f) >> 8)) >> 24)


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return (f * 255), where f is an ADF_I1616 fixed point value that lies
 *    in the mathematical range [0, 1) and the computed result is an 8-bit unsigned
 *    integer that lies in the range [0, 254]. Note that the maximum value of the
 *    computed result is 254 and not 255 because the fractional bits of the computed
 *    result are truncated (i.e., the computed result is rounded towards zero); see
 *    Example 2 below.
 *
 *    I1616_TO_U8() is implemented as a macro. The first part of the macro (i.e., (f <<
 *    8) - f) evaluates ((f * 256) - f) = (f * (256 - 1)) = (f * 255). The second part
 *    of the macro converts the 16.16 fixed point product (f * 255) to an 8-bit integer
 *    by shifting the product to the right by 16 bits.
 *
 *    The following examples omit some of the macro's parentheses to make the examples
 *    easier to follow.
 *
 *    Example 1. Consider the case where f = 0:
 *
 *        ((f << 8) - f) >> 16 = ((0 << 8) - 0) >> 16
 *                             = (0 - 0) >> 16
 *                             = 0 >> 16
 *                             = 0
 *
 *    Example 2. Consider the case where f = 0x0000ffff (i.e., f has the mathematical
 *    value (0x0000ffff / 2^16) = 0.9999847412109375):
 *
 *        ((f << 8) - f) >> 16 = ((0x0000ffff << 8) - 0x0000ffff) >> 16
 *                             = (0x00ffff00 - 0x0000ffff) >> 16
 *                             = 0xfeff01 >> 16
 *                             = 0xfe
 *                             = 254
 *
 *    Example 3. Consider the case where f = 0x00008000 (i.e., f has the mathematical
 *    value (0x00008000 / 2^16) = 0.5:
 *
 *        ((f << 8) - f) >> 16 = ((0x00008000 << 8) - 0x00008000) >> 16
 *                             = (0x00800000 - 0x00008000) >> 16
 *                             = 0x7f8000 >> 16
 *                             = 0x7f
 *                             = 127
 *-----------------------------------------------------------------------------------
 */
#define I1616_TO_U8(f) ((((f) << 8) - (f)) >> 16)

#endif /* FS_EDGE_RENDER */

/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the number of leading zeroes in a 32-bit unsigned integer. The
 *    implementation is optimized using an 8-bit lookup table.
 *
 *    Example 1:
 *    Input u:     0x00000000 (i.e., all zeroes)
 *    Result:         32
 *
 *    Example 2:
 *    Input u:     0x8000ffff (i.e., the MSB is set)
 *    Result:         0
 *
 *    Example 3:
 *    Input u:     0x200100ff (i.e., the leading 1 is bit 29)
 *    Result:         2
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    clzTable is a lookup table used by the CountLeadingZeroes() function to optimize
 *    the computation of the number of leading zeroes in a 32-bit unsigned integer.
 *    clzTable[i] contains the number of leading zeroes for an 8-bit unsigned integer i
 *    (i.e., i is an integer that lies in the range [0, 255]). The table contains 256
 *    elements.
 *
 *    Example 1: clzTable[0] is 8 because the 8-bit unsigned integer 0 (binary
 *    representation 00000000) contains 8 leading zeroes.
 *
 *    Example 2: clzTable[9] is 4 because the 8-bit unsigned integer 9 (binary
 *    representation 00001001) contains 4 leading zeroes.
 *-----------------------------------------------------------------------------------
 */
static FS_CONST ADF_I32 clzTable[] = {
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/**
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I32 CountLeadingZeroes (ADF_U32 u)
{
    /**
     *----Initialize the number of leading zeroes to 0 to prepare for incremental
     *----updates below
     */
    ADF_I32 numLeadingZeroes = 0;

    
    /**
     *----If the most significant 16 bits of u are clear (i.e., u < 0x00010000), then
     *----increment numLeadingZeroes by 16 and shift the least significant 16 bits of
     *----u to the most significant 16 bits
     */
    if (u < 0x00010000) {
        numLeadingZeroes += 16;
        u <<= 16;
    }


    /**
     *----If the most significant 8 bits of u are clear (i.e., u < 0x01000000), then
     *----increment numLeadingZeroes by 8 and shift the least significant 24 bits of
     *----u to the most significant 24 bits
     */
    if (u < 0x01000000) {
        numLeadingZeroes += 8;
        u <<= 8;
    }


    /**
     *----If u contains any remaining 1 bits, they must be in the most significant 8
     *----bits (i.e., bits 31:24). Use a precomputed 8-bit table to determine the
     *----number of leading zeroes in these 8 bits and increment numLeadingZeroes.
     */
    numLeadingZeroes += clzTable[u >> 24];


    /**
     *----Return the number of leading zeroes
     */
    return(numLeadingZeroes);
}


/**
 *-----------------------------------------------------------------------------------
 *    Multiply two 32-bit unsigned integers x and y and return only the most
 *    significant 32 bits of the 64-bit product (i.e., bits 63:32).
 *
 *    All assembly and C implementations produce bit-identical results.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is ~3.0x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~1.0x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~3.0x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_U32 UMUL64_HIGH32 (ADF_U32 x, ADF_U32 y)
{
#if (ADF_MATH_MODE == ADF_MATH_FIXED_C_64)
    {
        /**
         *----C implementation (64-bit). Compute (x * y) and return the most
         *----significant 32 bits of the 64-bit product.
         */
        return((ADF_U32) ((((ADF_I64) x) * ((ADF_I64) y)) >> 32));
    }    
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_C_32)
    {
        /**
         *----C implementation (32-bit). Separate x into two 16-bit values.
         */
        ADF_U32 xLow  = x & 0xffff;
        ADF_U32 xHigh = x >> 16;


        /**
         *----Separate y into two 16-bit values
         */
        ADF_U32 yLow  = y & 0xffff;
        ADF_U32 yHigh = y >> 16;


        /**
         *----Compute partial products
         */
        ADF_U32 zLow  = xLow * yLow;
        ADF_U32 zMid1 = xLow * yHigh;
        ADF_U32 zMid2 = xHigh * yLow;
        ADF_U32 zHigh = xHigh * yHigh;


        /**
         *----Add the middle 32-bit values. If the resulting value of zMid1 is less
         *----than zMid2 (i.e., zMid1 < zMid2), then a 32-bit unsigned integer
         *----overflow occurred, which means that a carry bit must be added to zHigh
         *----below.
         */
        zMid1 += zMid2;


        /**
         *----Compute the most significant 32 bits of the 64-bit product and add the
         *----carry bit
         */
        zHigh += (((ADF_U32)(zMid1 < zMid2) << 16) + (zMid1 >> 16));


        /**
         *----Move the low 16 bits of the middle sum to the high 16 bits
         */
        zMid1 <<= 16;


        /**
         *----Compute the least significant 32 bits of the 64-bit product. If the
         *----resulting value of zLow is less than zMid1 (i.e., zLow < zMid1), then a
         *----32-bit unsigned integer overflow occurred, which means that a carry bit
         *----must be added to zHigh below.
         */
        zLow += zMid1;


        /**
         *----Add the carry bit
         */
        zHigh += (ADF_U32)(zLow < zMid1);


        /**
         *----Return the high 32 bits of the product
         */
        return(zHigh);
    }     
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_ASM_X86)
    {
        /**
         *----x86 assembly implementation
         */
        __asm {


            /**
             *----Get parameters x and y
             */
            mov eax, x;
            mov ebx, y;


            /**
             *----[edx:eax] = 64-bit product of x and y
             */
            mul ebx;


            /**
             *----Return the high 32 bits (i.e., edx) of the product
             */
            mov eax, edx;
        }
    }
#endif
}


/**
 *-----------------------------------------------------------------------------------
 *    Multiply two 32-bit unsigned integers x and y and return the middle 32 bits of
 *    the 64-bit product (i.e., bits 47:16)
 *
 *    All assembly and C implementations produce bit-identical results.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is ~2.3x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~1.5x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~3.5x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_U32 UMUL64_MID32 (ADF_U32 x, ADF_U32 y)
{
#if (ADF_MATH_MODE == ADF_MATH_FIXED_C_64)
    {
        /**
         *----C implementation (64-bit). Compute (x * y) and return the middle 32
         *----bits (i.e., bits 47:16) of the 64-bit product.
         */
        return((ADF_U32) ((((ADF_I64) x) * ((ADF_I64) y)) >> 16));
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_C_32)
    {
        /**
         *----C implementation (32-bit). Separate x into two 16-bit values.
         */
        ADF_U32 xLow = x & 0xffff;
        ADF_U32 xHigh = x >> 16;


        /**
         *----Separate y into two 16-bit values
         */
        ADF_U32 yLow = y & 0xffff;
        ADF_U32 yHigh = y >> 16;


        /**
         *----Compute partial products
         */
        ADF_U32 zLow = xLow * yLow;
        ADF_U32 zMid1 = xLow * yHigh;
        ADF_U32 zMid2 = xHigh * yLow;
        ADF_U32 zHigh = xHigh * yHigh;


        /**
         *----Add the middle 32-bit values. If the resulting value of zMid1 is less
         *----than zMid2 (i.e., zMid1 < zMid2), then a 32-bit unsigned integer
         *----overflow occurred, which means that a carry bit must be added to zHigh
         *----below.
         */
        zMid1 += zMid2;


        /**
         *----Compute the most significant 32 bits of the 64-bit product and add the
         *----carry bit
         */
        zHigh += (((ADF_U32)(zMid1 < zMid2) << 16) + (zMid1 >> 16));


        /**
         *----Move the low 16 bits of the middle sum to the high 16 bits
         */
        zMid1 <<= 16;


        /**
         *----Compute the least significant 32 bits of the 64-bit product. If the
         *----resulting value of zLow is less than zMid1 (i.e., zLow < zMid1), then a
         *----32-bit unsigned integer overflow occurred, which means that a carry bit
         *----must be added to zHigh below.
         */
        zLow += zMid1;


        /**
         *----Add the carry bit
         */
        zHigh += (ADF_U32)(zLow < zMid1);


        /**
         *----Return the middle 32 bits (i.e., bits 47:16) of the product
         */
        return((zHigh << 16) | (zLow >> 16));
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_ASM_X86)
    {
        /**
         *----x86 assembly implementation
         */
        __asm {


            /**
             *----Get parameters x and y
             */
            mov eax, x;
            mov ebx, y;


            /**
             *----[edx:eax] = 64-bit product of x and y
             */
            mul ebx;


            /**
             *----Extract the middle 32 bits (i.e., bits 47:16) of the 64-bit product
             */
            shl edx, 16;
            shr eax, 16;
            or    eax, edx;
        }
    }
#endif
}


/**
 *-----------------------------------------------------------------------------------
 *    Multiply two 32-bit unsigned integers x and y and return the 64-bit product in
 *    two 32-bit unsigned integers. On output, zHighOut contains the high 32 bits of
 *    the product and zLowOut contains the low 32 bits of the product.
 *
 *    All assembly and C implementations produce bit-identical results.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64 is ~2.9x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_Void UMUL64 (ADF_U32 x, ADF_U32 y, ADF_U32 *zHighOut, ADF_U32
*zLowOut)
{
#if ((ADF_MATH_MODE == ADF_MATH_FIXED_C_64) || \
    (ADF_MATH_MODE == ADF_MATH_FIXED_ASM_X86))
    {
        /**
         *----C implementation (64-bit)
         */
        ADF_I64 z64 = (((ADF_I64) x) * ((ADF_I64) y));
        *zLowOut    = (ADF_U32) (z64);
        *zHighOut    = (ADF_U32) (z64 >> 32);
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_C_32)
    {
        /**
         *----C implementation (32-bit). Separate x into two 16-bit values.
         */
        ADF_U32 xLow = x & 0xffff;
        ADF_U32 xHigh = x >> 16;


        /**
         *----Separate y into two 16-bit values
         */
        ADF_U32 yLow = y & 0xffff;
        ADF_U32 yHigh = y >> 16;


        /**
         *----Compute partial products
         */
        ADF_U32 zLow = xLow * yLow;
        ADF_U32 zMid1 = xLow * yHigh;
        ADF_U32 zMid2 = xHigh * yLow;
        ADF_U32 zHigh = xHigh * yHigh;


        /**
         *----Add the middle 32-bit values. If the resulting value of zMid1 is less
         *----than zMid2 (i.e., zMid1 < zMid2), then a 32-bit unsigned integer
         *----overflow occurred, which means that a carry bit must be added to zHigh
         *----below.
         */
        zMid1 += zMid2;


        /**
         *----Compute the most significant 32 bits of the 64-bit product and add the
         *----carry bit
         */
        zHigh += (((ADF_U32)(zMid1 < zMid2) << 16) + (zMid1 >> 16));


        /**
         *----Move the low 16 bits of the middle sum to the high 16 bits
         */
        zMid1 <<= 16;


        /**
         *----Compute the least significant 32 bits of the 64-bit product. If the
         *----resulting value of zLow is less than zMid1 (i.e., zLow < zMid1), then a
         *----32-bit unsigned integer overflow occurred, which means that a carry bit
         *----must be added to zHigh below.
         */
        zLow += zMid1;


        /**
         *----Add the carry bit
         */
        zHigh += (ADF_U32)(zLow < zMid1);


        /**
         *----Store the product
         */
        *zLowOut = zLow;
        *zHighOut = zHigh;
    }
#endif
}

/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the absolute value of x (i.e., abs(x)). Both the input x and
 *    the computed result are ADF_I1616 fixed point values.
 *
 *    Note that I1616_ABS() does not compute the correct answer when x has the
 *    mathematical value -32768 (hexadecimal value 0x80000000). The correct answer is
 *    32768, which overflows the ADF_I1616 fixed point representation. In this case,
 *    I1616_ABS() returns -32768 (i.e., the same value as the input).
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I1616 I1616_ABS (ADF_I1616 x)
{
    return((x < 0) ? -x : x);
}


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the negative absolute of x (i.e., -abs(x)). Both the input x
 *    and the computed result are ADF_I1616 fixed point values.
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I1616 I1616_NEGABS (ADF_I1616 x)
{
    return((x < 0) ? x : -x);
}


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return -x. Both the input x and the computed result are ADF_I1616
 *    fixed point values.
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I1616 I1616_NEGATE (ADF_I1616 x)
{
    return(-x);
}


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return floor(x). Both the input x and the computed result are
 *    ADF_I1616 fixed point values.
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I1616 I1616_FLOOR (ADF_I1616 x)
{
    return (x & 0xffff0000);
}


/**
 *-----------------------------------------------------------------------------------
 *    Convert an IEEE-754 32-bit floating point value cf to an ADF_I1616 fixed point
 *    value and return the result. Special cases are handled as follows:
 *
 *      1) If cf is a denormalized floating point number, FLOAT_TO_I1616() returns
 *         zero.
 *
 *      2) If cf is NaN, FLOAT_TO_I1616() returns an undefined value.
 *
 *      3) If cf represents a value that overflows the ADF_I1616 fixed point
 *         representation, FLOAT_TO_I1616() returns an undefined value.
 *
 *      4) If cf represents a value that underflows the ADF_I1616 fixed point
 *         representation, FLOAT_TO_I1616() returns zero.
 *
 *      5) If cf cannot be represented exactly as an ADF_I1616 fixed point value, the
 *         converted fixed point value is rounded towards zero.
 *
 *    Note that some ADF_I1616 values (e.g., 32767 + (65535 / 65536)) cannot be
 *    represented exactly as IEEE-754 32-bit floating point values. Conversely, some
 *    IEEE-754 32-bit floating point values (e.g., 2 ^ -20) cannot be represented
 *    exactly as ADF_I1616 values. Consequently, care must be taken when using the
 *    FLOAT_TO_I1616() and I1616_TO_FLOAT() functions to convert between these two
 *    numerical representations. In particular, FLOAT_TO_I1616() and I1616_TO_FLOAT()
 *    are not inverses of each other (i.e., there exist values of x such that x differs
 *    from FLOAT_TO_I1616(I1616_TO_CFLOAT(x)), and there exist values of y such that y
 *    differs from I1616_TO_FLOAT(FLOAT_TO_I1616(y))).
 *
 *    All assembly and C implementations produce bit-identical results when cf is zero
 *    or a normalized floating point number.
 *-----------------------------------------------------------------------------------
 */
#define FLOAT_TO_I1616(a) ((ADF_I1616)(a))

/**
 *-----------------------------------------------------------------------------------
 *    Convert an ADF_I1616 fixed point value f to an IEEE-754 32-bit floating point
 *    value and return the result. If f cannot be represented exactly as an IEEE-754
 *    32-bit floating point value, the converted floating point value is rounded
 *    towards zero.
 *
 *    Note that some ADF_I1616 values (e.g., 32767 + (65535 / 65536)) cannot be
 *    represented exactly as IEEE-754 32-bit floating point values. Conversely, some
 *    IEEE-754 32-bit floating point values (e.g., 2 ^ -20) cannot be represented
 *    exactly as ADF_I1616 values. Consequently, care must be taken when using the
 *    FLOAT_TO_I1616() and I1616_TO_FLOAT() functions to convert between these two
 *    numerical representations. In particular, FLOAT_TO_I1616() and I1616_TO_FLOAT()
 *    are not inverses of each other (i.e., there exist values of x such that x differs
 *    from FLOAT_TO_I1616(I1616_TO_CFLOAT(x)), and there exist values of y such that y
 *    differs from I1616_TO_FLOAT(FLOAT_TO_I1616(y))).
 *-----------------------------------------------------------------------------------
 */
#define I1616_TO_FLOAT(a) ((ADF_F32)(a))

/**
 *-----------------------------------------------------------------------------------
 *    Convert an ADF_I1616 fixed point value f to a 32-bit unsigned integer and return
 *    the result. If f has a non-zero fractional component, the converted value is
 *    rounded towards negative infinity.
 *
 *    All assembly and C implementations produce bit-identical results.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is ~1.0x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~2.7x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~2.7x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_U32 I1616_TO_U32 (ADF_I1616 f)
{
#if (ADF_MATH_MODE == ADF_MATH_FIXED_C_64)
    {
        /**
         *----C implementation (64-bit). Perform an arithmetic right shift of 16
         *----bits, which has the effect of rounding f towards negative infinity.
         */
        return((ADF_U32) (f >> 16));
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_C_32)
    {
        /**
         *----C implementation (32-bit). Perform an arithmetic right shift of 16
         *----bits, which has the effect of rounding f towards negative infinity.
         *----Note that ANSI C does not require that sign extension be performed when
         *----the left operand of a right shift is a signed integer. Therefore, sign
         *----extension is performed manually to ensure a portable implementation.
         */
        ADF_I32 sign = (f & 0x80000000);
        f >>= 16;
        if (sign) f |= 0xffff0000;
        return((ADF_U32) f);
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_ASM_X86)
    {
        /**
         *----x86 assembly implementation
         */
        _asm {


            /**
             *----Set eax to the parameter f
             */
            mov eax, f;


            /**
             *----Arithmetic right shift by 16 bits, which has the effect of rounding
             *----f towards negative infinity
             */
            sar eax, 16;
        }
    }
#endif
}


/**
 *-----------------------------------------------------------------------------------
 *    Convert a 32-bit unsigned integer u to an ADF_I1616 fixed point value and return
 *    the result. If u overflows the ADF_I1616 representation, the result is undefined.
 *
 *    All assembly and C implementations produce bit-identical results.
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I1616 U32_TO_I1616 (ADF_U32 u)
{
#if ((ADF_MATH_MODE == ADF_MATH_FIXED_C_32) || \
    (ADF_MATH_MODE == ADF_MATH_FIXED_C_64))
    {
        /**
         *----C implementation (32-bit and 64-bit)
         */
        return((ADF_I1616) (u << 16));
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_ASM_X86)
    {
        /**
         *----x86 assembly implementation
         */
        __asm {


            /**
             *----Set eax to the parameter u
             */
            mov eax, u;


            /**
             *----Return (u << 16)
             */
            shl eax, 16;
        }
    }
#endif
}


/**
 *-----------------------------------------------------------------------------------
 *    Convert an ADF_I1616 fixed point value f to a 32-bit signed integer and return
 *    the result. If f has a non-zero fractional component, the value is rounded
 *    towards negative infinity.
 *
 *    All assembly and C implementations produce bit-identical results.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is  1.0x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~2.7x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~2.7x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I32 I1616_TO_I32 (ADF_I1616 f)
{
#if (ADF_MATH_MODE == ADF_MATH_FIXED_C_64)
    {
        /**
         *----C implementation (64-bit). Perform an arithmetic right shift of 16
         *----bits, which has the effect of rounding f towards negative infinity.
         */
        return((ADF_I32) (f >> 16));
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_C_32)
    {
        /**
         *----C implementation (32-bit and 64-bit). Perform an arithmetic right shift
         *----of 16 bits, which has the effect of rounding f towards negative
         *----infinity. Note that ANSI C does not require that sign extension be
         *----performed when the left operand of a right shift is a signed integer.
         *----Therefore, sign extension is performed manually to ensure a portable
         *----implementation.
         */
        ADF_I32 sign = (f & 0x80000000);
        f >>= 16;
        if (sign) f |= 0xffff0000;
        return(f);
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_ASM_X86)
    {
        /**
         *----x86 assembly implementation
         */
        _asm {


            /**
             *----Set eax to the parameter f
             */
            mov eax, f;


            /**
             *----Arithmetic right shift by 16 bits, which has the effect of rounding
             *----f towards negative infinity
             */
            sar eax, 16;
        }
    }
#endif
}

/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the signed sum (a + b). The inputs a and b and the computed
 *    sum are ADF_I1616 fixed point values. If the sum overflows the ADF_I1616
 *    representation, the result is undefined.
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I1616 I1616_ADD (ADF_I1616 a, ADF_I1616 b)
{
    return(a + b);
}


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the signed difference (a - b). The inputs a and b and the
 *    computed difference are ADF_I1616 fixed point values. If the difference overflows
 *    the ADF_I1616 representation, the result is undefined.
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I1616 I1616_SUB (ADF_I1616 a, ADF_I1616 b)
{
    return(a - b);
}


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the square of a (i.e., a^2). The input a and the computed
 *    square of a are ADF_I1616 fixed point values. If the computed square overflows
 *    the ADF_I1616 fixed point representation, the result is undefined.
 *
 *    All assembly and C implementations produce bit-identical results.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is ~1.5x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~2.2x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~3.4x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I1616 I1616_SQR (ADF_I1616 a)
{
    /**
     *----Optimize for a common case which occurs during implicit ADF rendering
     *----(i.e., -128 is the initialization value for the distance buffer)
     */
    if (a == (ADF_I1616)0xff800000) return(0x40000000);
    

    /**
     *----Compute the square of a
     */
#if (ADF_MATH_MODE == ADF_MATH_FIXED_C_64)
    {
        /**
         *----C implementation (64-bit). Compute the 64-bit product and normalize the
         *----result to the ADF_I1616 fixed point format. The low 16 bits of the
         *----64-bit product are truncated, which effectively rounds the result
         *----towards negative infinity.
         */
        return((ADF_I32) (((((ADF_I64) a) * ((ADF_I64) a)) >> 16)));
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_C_32)
    {
        /**
         *----C implementation (32-bit)
         */
        ADF_U32 z, zHigh, zLow;


        /**
         *----Determine the sign of parameter a
         */
        ADF_I32 aSign = (a & 0x80000000);


        /**
         *----Set a to abs(a)
         */
        a = (aSign) ? -a : a;


        /**
         *----Compute the 64-bit unsigned product 
         */
        UMUL64(*(ADF_U32*) &a, *(ADF_U32*) &a, &zHigh, &zLow);


        /**
         *----Merge the integer and fractional bits of the unsigned product. The low
         *----16 bits of the 64-bit product are truncated, effectively rounding the
         *----result towards negative infinity.
         */
        z = ((zLow >> 16) | (zHigh << 16));


        /**
         *----Return the result
         */
        return(z);
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_ASM_X86)
    {
        /**
         *----x86 assembly implementation
         */
        _asm {


            /**
             *----Set eax to the parameter a
             */
            mov      eax, a;


            /**
             *----Set [edx:eax] to the 64-bit product (a * a)
             */
            imul  eax;


            /**
             *----Shift the product's 16 integer bits into bits 31:16 of edx
             */
            shl      edx, 16;


            /**
             *----Shift the product's 16 fractional bits into bits 15:0 of eax. This
             *----truncates the low 16 bits of the product, effectively rounding the
             *----result towards negative infinity.
             */
            shr      eax, 16;


            /**
             *----Merge and return the integer and fractional bits (i.e., bits 47:16
             *----of the 64-bit product)
             */
            or      eax, edx;
        }
    }
#endif
}


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the signed product of a and b (i.e., a * b). The inputs a and
 *    b are ADF_I1616 fixed point values, and the computed result is an ADF_I2408 fixed
 *    point value. The signed product is rounded towards negative infinity.
 *
 *    All assembly and C implementations produce bit-identical results.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is ~2.6x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~1.6x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~4.2x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I2408 I1616_MUL_I2408 (ADF_I1616 a, ADF_I1616 b)
{
#if (ADF_MATH_MODE == ADF_MATH_FIXED_C_64)
    {
        /**
         *----C implementation (64-bit). Compute the 64-bit product and normalize the
         *----result to the ADF_I2408 fixed point format. The low 24 fractional bits
         *----of the 64-bit product are discarded, effectively rounding the value
         *----towards negative infinity.
         */
        return((ADF_I32) (((((ADF_I64) a) * ((ADF_I64) b)) >> 24)));
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_C_32)
    {
        /**
         *----C implementation (32-bit)
         */
        ADF_U32 z, zHigh, zLow;
        ADF_I32 result;


        /**
         *----Determine the signs of parameters a and b
         */
        ADF_I32 aSign = (a & 0x80000000);
        ADF_I32 bSign = (b & 0x80000000);


        /**
         *----Make parameters a and b unsigned
         */
        a = (aSign) ? -a : a;
        b = (bSign) ? -b : b;


        /**
         *----Compute the 64-bit unsigned product 
         */
        UMUL64(*(ADF_U32*) &a, *(ADF_U32*) &b, &zHigh, &zLow);


        /**
         *----Merge the integer and fractional bits of the unsigned product
         */
        z = (zLow >> 24) | (zHigh << 8);


        /**
         *----Compute the signed product
         */
        result = ((aSign ^ bSign) ? -((ADF_I32) z) : z);


        /**
         *----If the signed product is negative and the low 24 bits of the 64-bit
         *----product are not all zero, then subtract 1 from the result to round the
         *----result towards negative infinity. To see why it is necessary to
         *----subtract 1, consider rounding the value -1.5 towards negative infinity.
         *----Simply dropping the fractional portion produces the value -1.0, which
         *----has the effect of rounding -1.5 towards zero. However, dropping the
         *----fractional portion and then subtracting 1 (i.e., -1.0 - 1 = -2.0)
         *----produces the desired result of rounding -1.5 towards negative infinity.
         */
        if ((aSign ^ bSign) && (zLow & 0x00ffffff)) result -= 1;


        /**
         *----Return the signed product
         */
        return(result);
    }     
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_ASM_X86)
    {
        /**
         *----x86 assembly implementation
         */
        _asm {


            /**
             *----Set eax and edi to the parameters a and b, respectively
             */
            mov      eax, a;
            mov      edi, b;


            /**
             *----Set [edx:eax] to the 64-bit product of a and b
             */
            imul  edi;


            /**
             *----Shift the product's 24 integer bits into bits 31:8 of edx
             */
            shl      edx, 8;


            /**
             *----Shift the product's 8 fractional bits into bits 7:0 of eax. The low
             *----24 fractional bits of the 64-bit product are discarded, effectively
             *----rounding the value towards negative infinity.
             */
            shr      eax, 24;


            /**
             *----Merge and return the integer and fractional bits
             */
            or      eax, edx;
        }
    }
#endif
}


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the signed product of a and b (i.e., a * b). The inputs a and
 *    b and the computed product (a * b) are ADF_I1616 fixed point values. The signed
 *    product is rounded towards negative infinity.
 *
 *    All assembly and C implementations produce bit-identical results.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is ~2.7x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~1.5x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~4.1x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_INLINE static ADF_I1616 I1616_MUL (ADF_I1616 a, ADF_I1616 b)
{
    /**
     *----Optimize for a common case which occurs during implicit ADF rendering
     *----(i.e., b often has the mathematical value 1)
     */
    if (b == 0x00010000) return(a);


    /**
     *----Compute the signed product of a and b
     */
#if (ADF_MATH_MODE == ADF_MATH_FIXED_C_64)
    {
        /**
         *----C implementation (64-bit). Compute the 64-bit product and normalize the
         *----result to the ADF_I1616 fixed point format. The low 16 fractional bits
         *----of the 64-bit product are discarded, effectively rounding the value
         *----towards negative infinity.
         */
        return((ADF_I32) (((((ADF_I64) a) * ((ADF_I64) b)) >> 16)));
    }     
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_C_32)
    {     
        /**
         *----C implementation (32-bit)
         */
        ADF_U32 z, zHigh, zLow;
        ADF_I32 result;


        /**
         *----Determine the signs of parameters a and b
         */
        ADF_I32 aSign = (a & 0x80000000);
        ADF_I32 bSign = (b & 0x80000000);


        /**
         *----Make parameters a and b unsigned
         */
        a = (aSign) ? -a : a;
        b = (bSign) ? -b : b;


        /**
         *----Compute the 64-bit unsigned product 
         */
        UMUL64(*(ADF_U32*) &a, *(ADF_U32*) &b, &zHigh, &zLow);


        /**
         *----Merge the integer and fractional bits of the unsigned product
         */
        z = (zLow >> 16) | (zHigh << 16);


        /**
         *----Compute the signed result
         */
        result = ((aSign ^ bSign) ? -((ADF_I32) z) : z);


        /**
         *----If the signed product is negative and the low 16 bits of the 64-bit
         *----product are not all zero, then subtract 1 from the result to round the
         *----result towards negative infinity. To see why it is necessary to
         *----subtract 1, consider rounding the value -1.5 towards negative infinity.
         *----Simply dropping the fractional portion produces the value -1.0, which
         *----has the effect of rounding -1.5 towards zero. However, dropping the
         *----fractional portion and then subtracting 1 (i.e., -1.0 - 1 = -2.0)
         *----produces the desired result of rounding -1.5 towards negative infinity.
         */
        if ((aSign ^ bSign) && (zLow & 0x0000ffff)) result -= 1;

        
        /**
         *----Return the signed product
         */
        return(result);
    }
#elif (ADF_MATH_MODE == ADF_MATH_FIXED_ASM_X86)
    {
        /**
         *----x86 assembly implementation
         */
        _asm {


            /**
             *----Set eax and edi to the parameters a and b, respectively
             */
            mov      eax, a;
            mov      edi, b;


            /**
             *----Set [edx:eax] to the 64-bit product of a and b
             */
            imul  edi;


            /**
             *----Shift the product's 16 integer bits into bits 31:16 of edx
             */
            shl      edx, 16;


            /**
             *----Shift the product's 16 fractional bits into bits 15:0 of eax. The
             *----low 16 fractional bits of the 64-bit product are discarded,
             *----effectively rounding the value towards negative infinity.
             */
            shr      eax, 16;

        
            /**
             *----Merge and return the integer and fractional bits
             */
            or      eax, edx;
        }
    }     
#endif
}

#ifdef FS_EDGE_RENDER

/**
 *-----------------------------------------------------------------------------------
 *    Compute and return x^y, where x must lie in the range (0,1) (note the exclusion
 *    of 0 and 1) and y must be positive. The inputs x and y and the computed result
 *    are ADF_I1616 fixed point values.
 *-----------------------------------------------------------------------------------
 */
ADF_I1616 I1616_POW01 (ADF_I1616 x, ADF_I1616 y);


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the reciprocal square root of f (i.e., 1 / sqrt(f)), where the
 *    input f is an ADF_I2408 fixed point value and the computed value is an ADF_I1616
 *    fixed point value. The computed result is rounded towards negative infinity. If
 *    the input f is zero, the result is undefined.
 *
 *    All assembly and C implementations produce bit-identical results, except in the
 *    case that the input f is zero.
 *-----------------------------------------------------------------------------------
 */
ADF_I1616 I2408_RSQ_I1616 (ADF_I2408 f);


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the reciprocal square root of f (i.e., 1 / sqrt(f)), where
 *    both the input f and the computed result are ADF_I1616 fixed point values. The
 *    computed result is rounded towards negative infinity. If the input f is zero, the
 *    result is undefined.
 *
 *    All assembly and C implementations produce bit-identical results, except in the
 *    case that the input f is zero.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is ~1.7x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~1.3x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~2.2x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_I1616 I1616_RSQ (ADF_I1616 f);


/**
 *-----------------------------------------------------------------------------------
 *    Normalize the 2D vector (nx, ny) using high-precision arithmetic and return the
 *    result in (nxOut, nyOut). The inputs nx and ny and the outputs nxOut and nyOut
 *    are ADF_I1616 fixed point values. The following six cases are possible:
 *
 *      1) If nx and ny are both zero (i.e., the vector has zero length),
 *         I1616_NORMALIZE() returns zero and the contents of nxOut and nyOut are
 *         undefined. 
 *
 *      2) If nx is zero and ny is positive, I1616_NORMALIZE() returns one, stores zero
 *         into nxOut, and stores the ADF_I1616 representation of 1 (i.e., 0x00010000)
 *         into nyOut.
 *
 *      3) If nx is zero and ny is negative, I1616_NORMALIZE() returns one, stores zero
 *         into nxOut, and stores the ADF_I1616 representation of -1 (i.e., 0xffff0000)
 *         into nyOut.
 *
 *      4) If nx is positive and ny is zero, I1616_NORMALIZE() returns one, stores the
 *         ADF_I1616 representation of 1 (i.e., 0x00010000) into nxOut, and stores zero
 *         into nyOut.
 *
 *      5) If nx is negative and ny is zero, I1616_NORMALIZE() returns one, stores the
 *         ADF_I1616 representation of -1 (i.e., 0xffff0000) into nxOut, and stores
 *         zero into nyOut.
 *
 *      6) If nx and ny are both non-zero, I1616_NORMALIZE() returns one, stores the x
 *         component of the normalized vector into nxOut, and stores the y component of
 *         the normalized vector into nyOut. 
 *
 *    All assembly and C implementations produce bit-identical results for nxOut and
 *    nyOut, except in the case where inputs nx and ny are both zero (in which case the
 *    contents of nxOut and nyOut are undefined).
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz), MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is ~1.4x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~1.6x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~2.2x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 */
ADF_I1616 I1616_NORMALIZE (ADF_I1616 nx, ADF_I1616 ny, ADF_I1616 *nxOut,
ADF_I1616 *nyOut);


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the positive square root of f, where both f and the computed
 *    result are non-negative ADF_I1616 fixed point values.
 *
 *    Special cases are handled as follows. If f is zero, I1616_SQRT() returns exactly
 *    zero. If f is negative, the result is undefined.
 *
 *    All assembly and C implementations produce bit-identical results.
 *-----------------------------------------------------------------------------------
 */
ADF_I1616 I1616_SQRT (ADF_I1616 f);


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the positive square root of f, where f is a non-negative
 *    ADF_I2408 fixed point value and the computed result is an ADF_I1616 fixed point
 *    value.
 *
 *    Special cases are handled as follows. If f is zero, I2408_SQRT_I1616() returns
 *    exactly zero. If f is negative, the result is undefined.
 *
 *    All assembly and C implementations produce bit-identical results.
 *-----------------------------------------------------------------------------------
 */
ADF_I1616 I2408_SQRT_I1616 (ADF_I2408 f);

#endif /* FS_EDGE_RENDER */

/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the signed quotient (n / d). The input numerator n, the input
 *    denominator d, and the computed quotient are ADF_I1616 fixed point values. The
 *    quotient is rounded towards zero.
 *
 *    On output, I1616_DIV() sets status to ADF_FIXED_MATH_NO_ERROR,
 *    ADF_FIXED_MATH_OVERFLOW, ADF_FIXED_MATH_UNDERFLOW, or ADF_FIXED_MATH_NAN,
 *    depending on the outcome of the quotient computation. The possible cases are as
 *    follows:
 *
 *      1. If n is any value and d is zero, I1616_DIV() returns zero and sets status to
 *         ADF_FIXED_MATH_NAN.
 *
 *      2. If n is zero and d is non-zero, I1616_DIV() returns zero and sets status to
 *         ADF_FIXED_MATH_NO_ERROR.
 *
 *      3. If n is non-zero and d is 0x10000 (i.e., the mathematical value 1),
 *         I1616_DIV() returns n and sets status to ADF_FIXED_MATH_NO_ERROR.
 *
 *      4. If n is non-zero and d is 0xffffffff (i.e., the mathematical value -1),
 *         I1616_DIV() returns -n and sets status to ADF_FIXED_MATH_NO_ERROR.
 *
 *      5. If n and d are both non-zero and the quotient (n / d) overflows the
 *         ADF_I1616 fixed point representation, I1616_DIV() returns zero and sets
 *         status to ADF_FIXED_MATH_OVERFLOW.
 *
 *      6. If n and d are both non-zero and the quotient (n / d) underflows the
 *         ADF_I1616 fixed point representation, I1616_DIV() returns zero and sets
 *         status to ADF_FIXED_MATH_UNDERFLOW.
 *
 *      7. In all other cases, I1616_DIV() computes and returns the signed fixed point
 *         quotient (n / d) and sets status to ADF_FIXED_MATH_NO_ERROR.
 *    
 *    All assembly and C implementations produce bit-identical results.
 *
 *    Performance notes:
 *
 *    Intel Centrino Core Duo T2500 (2 MB L2, 2.0 GHz, FSB 677 MHz): MSVC 6 compiler,
 *    Release mode:
 *      - ADF_MATH_FIXED_C_64       is ~1.2x as fast as ADF_MATH_FIXED_C_32
 *      - ADF_MATH_FIXED_ASM_X86 is ~1.4x as fast as ADF_MATH_FIXED_C_64
 *      - ADF_MATH_FIXED_ASM_X86 is ~1.7x as fast as ADF_MATH_FIXED_C_32
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
#define ADF_FIXED_MATH_NO_ERROR        0
#define ADF_FIXED_MATH_OVERFLOW        1
#define ADF_FIXED_MATH_UNDERFLOW    2
#define ADF_FIXED_MATH_NAN            3
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
ADF_I1616 I1616_DIV (ADF_I1616 n, ADF_I1616 d, ADF_I32 *status);

#ifdef FS_EDGE_RENDER

/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the sine of x, where x is expressed in radians. Both x and the
 *    computed sine of x are ADF_I1616 fixed point values.
 *
 *    All assembly and C implementations produce bit-identical results.
 *-----------------------------------------------------------------------------------
 */
ADF_I1616 I1616_SIN (ADF_I1616 x);


/**
 *-----------------------------------------------------------------------------------
 *    Compute and return the cosine of x, where x is expressed in radians. Both x and
 *    the computed cosine of x are ADF_I1616 fixed point values.
 *
 *    All assembly and C implementations produce bit-identical results.
 *-----------------------------------------------------------------------------------
 */
ADF_I1616 I1616_COS (ADF_I1616 x);

#endif /* FS_EDGE_RENDER */

/**
 *-------------------------------------------------------------------------------
 *    END: FIXED POINT MATH ONLY
 *-------------------------------------------------------------------------------
 */


/**
 *-----------------------------------------------------------------------------------
 *    End of C++ wrapper
 *-----------------------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif

#endif /* FS_EDGE_HINTS or FS_EDGE_RENDER */

/**
 *-----------------------------------------------------------------------------------
 *    End of ADFFIXEDMATH_H
 *-----------------------------------------------------------------------------------
 */
#endif

