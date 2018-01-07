/*****************************************************************************
 *
 *  tsfixed.h - Defines the interface for fixed point math.
 *
 *  Copyright (C) 2002 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSFIXED_H
#define TSFIXED_H

#include "tstypes.h"

TS_BEGIN_HEADER


/* Suppress certain Lint warnings in macros. These suppressions allowable due to the way macros are constructed */
/*lint -emacro(506, TsFixed_init)  suppress Warning -- "Constant value Boolean" */
/*lint -emacro(568, TsFixed_init)  suppress Warning -- "Non-negative quantity is never less than zero" warning for comparisons below in case 'a' is unsigned */
/*lint -emacro(703, TsFixed_init)  suppress Warning -- "Shift left of signed quantity" since macro accounts for negative numbers */
/*lint -emacro(568, TsFixed_trunc) suppress Warning -- "Non-negative quantity is never less than zero" warning for comparisons below in case 'a' is unsigned */



/*****************************************************************************
 *
 *  Typedef for a 26.6 fixed point value.
 *
 *  Description:
 *      TsFixed26_6 is a typedef for a 26.6 fixed point value, which has
 *      a range and precision appropriate for many tasks.
 *
 *  <GROUP fixedpoint>
 */
typedef TsInt32 TsFixed26_6;


/*****************************************************************************
 *
 *  Typedef for a 16.16 fixed point value.
 *
 *  Description:
 *      TsFixed is a typedef for a 16.16 fixed point value, which has
 *      a range and precision appropriate for many tasks.
 *
 *  <GROUP fixedpoint>
 */
typedef TsInt32 TsFixed;


/*****************************************************************************
 *
 *  Converts an integer argument into a TsFixed 16.16 fixed point number.
 *
 *  Description:
 *      Converts, using proper shifting, an integral argument into a
 *      TsFixed 16.16 fixed point number.
 *
 *  Parameters:
 *      a           - [in] signed integer (32-bits or less)
 *
 *  Return value:
 *      TsFixed
 *
 *  Notes:
 *      Definition intended to be safe for all implementations given the
 *      ANSI/ISO C standard that refers to shift operators (<< and >>)
 *      being undefined or implementation defined when the left argument of
 *      the operator has signed integral type and has negative value.
 *
 *      This "function" is implemented as a macro and it evaluates its
 *      argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  <GROUP fixedpoint>
 */
#define TsFixed_init(a) ((((TsFixed)(a)) < (TsFixed)0) ? -((TsFixed)(-(a)) << 16) : ((TsFixed)(a) << 16))



/*****************************************************************************
 *
 *  Converts an integer and a fractional component into a fixed point number.
 *
 *  Description:
 *      Converts, using proper shifting and addition, an integral argument
 *      and a fractional component into a TsFixed 16.16 fixed point number.
 *      The fractional component must be integral and is taken to be a
 *      signed numerator of a fraction containing 65536 as an assumed
 *      denominator.  The numerator is effectively added to the integral
 *      component thereby allowing a negative numerator to reduce the integer.
 *
 *  Parameters:
 *      a           - [in] integer
 *      f           - [in] integral numerator with implied denominator 65536
 *
 *  Return value:
 *      TsFixed
 *
 *  Notes:
 *      Definition intended to be safe for all implementations given the
 *      ANSI/ISO C standard that refers to shift operators (<< and >>)
 *      being undefined or implementation defined when the left argument of
 *      the operator has signed integral type and has negative value.
 *
 *      This "function" is implemented as a macro and it evaluates its
 *      argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  <GROUP fixedpoint>
 */
#define TsFixed_initFrac(a, f) (TsFixed_init(a) + (TsFixed)(f))


/*****************************************************************************
 *
 *  Macro that defines the value of 0.0 as a 16.16 fixed point number.
 *
 *  <GROUP fixedpoint>
 */
#define TS_FIXED_ZERO   (0L)


/*****************************************************************************
 *
 *  Macro that defines the value of 1.0 as a 16.16 fixed point number.
 *
 *  <GROUP fixedpoint>
 */
#define TS_FIXED_ONE    (TsFixed)(1L << 16)


/*****************************************************************************
 *
 *  Macro that defines the value of 2.0 as a 16.16 fixed point number.
 *
 *  <GROUP fixedpoint>
 */
#define TS_FIXED_TWO    (TsFixed_init(2L))


/*****************************************************************************
 *
 *  Macro that defines the value of 128 as a 16.16 fixed point number.
 *
 *  <GROUP fixedpoint>
 */
#define TS_FIXED_128    (1L << 23)


/*****************************************************************************
 *
 *  Macro that defines the value of 1/2 as a 16.16 fixed point number.
 *
 *  <GROUP fixedpoint>
 */
#define TS_FIXED_ONE_HALF    (TsFixed)(1L << 15)


/*****************************************************************************
 *
 *  Macro that defines the value of 1/4 as a 16.16 fixed point number.
 *
 *  <GROUP fixedpoint>
 */
#define TS_FIXED_ONE_FOURTH    (1L << 14)


/*****************************************************************************
 *
 *  Macro that defines the value of 1.5 as a 16.16 fixed point number.
 *
 *  <GROUP fixedpoint>
 */
#define TS_FIXED_1pt5    (TS_FIXED_ONE_HALF + TS_FIXED_ONE)


/*****************************************************************************
 *
 *  Truncates a fixed point 16.16 number and converts to an integer.
 *
 *  Description:
 *      Rounds a fixed point 16.16 number to the integer value nearest to
 *      but no larger in magnitude than the argument.  Otherwise known as
 *      rounding toward zero.
 *
 *  Parameters:
 *      a           - [in] 16.16 fixed point number
 *
 *  Return value:
 *      long integer value
 *
 *  Notes:
 *      Definition intended to be safe for all implementations given the
 *      ANSI/ISO C standard that refers to shift operators (<< and >>)
 *      being undefined or implementation defined when the left argument of
 *      the operator has signed integral type and has negative value.
 *
 *      This "function" is implemented as a macro and it evaluates its
 *      argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  <GROUP fixedpoint>
 */
#define TsFixed_trunc(a) (TsInt32)(((a) < (TsFixed)0) ? -(TsFixed)((((TsUInt32)(-(a)))) >> 16) : (TsFixed)(((TsUInt32)(a)) >> 16))




/*****************************************************************************
 *
 *  Rounds a fixed point 16.16 number and converts to an integer.
 *
 *  Description:
 *      Rounds a fixed point 16.16 number to the nearest integer value,
 *      rounding halfway cases away from zero.
 *
 *  Parameters:
 *      a           - [in] 16.16 fixed point number
 *
 *  Return value:
 *      long integer value
 *
 *  Notes:
 *      Definition intended to be safe for all implementations given the
 *      ANSI/ISO C standard that refers to shift operators (<< and >>)
 *      being undefined or implementation defined when the left argument of
 *      the operator has signed integral type and has negative value.
 *
 *      This "function" is implemented as a macro and it evaluates its
 *      argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  <GROUP fixedpoint>
 */
#define TsFixed_round(a) \
    (TsInt32)TsFixed_trunc((a) + (((a) < (TsFixed)0) ? -TS_FIXED_ONE_HALF : TS_FIXED_ONE_HALF))


/*****************************************************************************
 *
 *  Rounds a fixed point 16.16 number and converts to an integer.
 *
 *  Description:
 *      Rounds a fixed point 16.16 number to the nearest integer value,
 *      rounding halfway cases up (positive).
 *
 *  Parameters:
 *      a           - [in] 16.16 fixed point number
 *
 *  Return value:
 *      long integer value
 *
 *  Notes:
 *      Definition intended to be safe for all implementations given the
 *      ANSI/ISO C standard that refers to shift operators (<< and >>)
 *      being undefined or implementation defined when the left argument of
 *      the operator has signed integral type and has negative value.
 *
 *      This "function" is implemented as a macro and it evaluates its
 *      argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  <GROUP fixedpoint>
 */
#define TsFixed_round_hp(a) (TsInt32)TsFixed_trunc((a) + TS_FIXED_ONE_HALF)


/*****************************************************************************
 *
 *  Returns the ceiling of a 16.16 number as an integer.
 *
 *  Description:
 *      Returns the smallest integer value not less than the argument.
 *
 *  Parameters:
 *      a           - [in] 16.16 fixed point number
 *
 *  Return value:
 *      long integer value
 *
 *  Notes:
 *      Definition intended to be safe for all implementations given the
 *      ANSI/ISO C standard that refers to shift operators (<< and >>)
 *      being undefined or implementation defined when the left argument of
 *      the operator has signed integral type and has negative value.
 *
 *      This "function" is implemented as a macro and it evaluates its
 *      argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  <GROUP fixedpoint>
 */
#define TsFixed_ceil(a) \
    (((a) < (TsFixed)0) ? -TsFixed_trunc(-(a)) : TsFixed_trunc((a) + (TsFixed)0xFFFF))


/*****************************************************************************
 *
 *  Returns the floor of a 16.16 number as an integer.
 *
 *  Description:
 *      Returns the largest integer value not greater than the argument.
 *
 *  Parameters:
 *      a           - [in] 16.16 fixed point number
 *
 *  Return value:
 *      long integer value
 *
 *  Notes:
 *      Definition intended to be safe for all implementations given the
 *      ANSI/ISO C standard that refers to shift operators (<< and >>)
 *      being undefined or implementation defined when the left argument of
 *      the operator has signed integral type and has negative value.
 *
 *      This "function" is implemented as a macro and it evaluates its
 *      argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  <GROUP fixedpoint>
 */
#define TsFixed_floor(a) \
               (((a) < (TsFixed)0) ? -TsFixed_trunc((-(a)) + (TsFixed)0xFFFF) : TsFixed_trunc(a))


/*****************************************************************************
 *
 *  Multiplies two 16.16 fixed point numbers.
 *
 *  Parameters:
 *      a           - [in] 16.16 fixed point number
 *      b           - [in] 16.16 fixed point number
 *
 *  Return value:
 *      product of a and b as another 16.16 fixed point number
 *
 *  <GROUP fixedpoint>
 */
TS_EXPORT(TsFixed)
TsFixed_mul(TsFixed a, TsFixed b);


/*****************************************************************************
 *
 *  Divides two 16.16 fixed point numbers.
 *
 *  Parameters:
 *      a           - [in] 16.16 fixed point numerator
 *      b           - [in] 16.16 fixed point denominator
 *
 *  Return value:
 *      a/b as a 16.16 fixed point number.
 *
 *  <GROUP fixedpoint>
 */
TS_EXPORT(TsFixed)
TsFixed_div(TsFixed a, TsFixed b);


/*****************************************************************************
 *
 *  Multiplies two 16.16 fixed point numbers then divides result by a
 *  third 16.16 fixed point number.
 *
 *  Parameters:
 *      a           - [in] 16.16 fixed point number
 *      b           - [in] 16.16 fixed point number
 *      c           - [in] 16.16 fixed point number
 *
 *  Return value:
 *      (a*b)/c as a 16.16 fixed point number.
 *
 *  <GROUP fixedpoint>
 */
TS_EXPORT(TsFixed)
TsFixed_mulDiv(TsFixed a, TsFixed b, TsFixed c);



/*****************************************************************************
 *
 *  Macro that returns TRUE if a is integer, else returns FALSE
 *
 *  Parameters:
 *      a           - [in] 16.16 fixed point number
 *
 *  Return value:
 *      TRUE if a is integer, else FALSE
 *
 *  <GROUP fixedpoint>
 */
#define TsFixed_isInt(a)  ((((a) & 0x0000FFFF) == 0) ? TRUE : FALSE)



/*
   Making exported functions dependent on TS_NDEBUG will cause trouble with
   Symbian builds because the ".def" file is shared between release and
   debug builds, and the mapping to exported functions gets confused.
*/
#ifdef NEED_TSFIXED_TOSTRING
/*****************************************************************************
 *
 *  Converts a TsFixed value into a string (TsChar *).  Result will be
 *  terminated properly with a NULL character.  TsChar * array is assumed
 *  to be large enough to hold the result...32 bytes would be plenty.
 *
 *  Parameters:
 *      fix           - [in] 16.16 fixed point number
 *      str           - [out] array of at least 32 bytes for result.
 *
 *  Return value:
 *      TsChar * string pointer to provided, passed-in string.
 *
 *  <GROUP fixedpoint>
 */
TS_EXPORT(TsChar *)
TsFixed_toString(TsFixed fix, TsChar *str);
#endif

TS_END_HEADER


#endif /* TSFIXED_H */
