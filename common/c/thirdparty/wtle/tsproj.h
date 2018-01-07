/*****************************************************************************
 *
 *  tsproj.h - Project Includes and Definitions
 *
 *  Copyright (C) 2002, 2006 Monotype Imaging Inc. All rights reserved.
 *
 ****************************************************************************/

#ifndef TSPROJ_H
#define TSPROJ_H

#include "tsport.h"     /* Port-specific file */
#include "tstypes.h"    /* Basic types */
#include "tsconfig.h"   /* Configuration options */
#include "tsresult.h"   /* Function return codes */


/*****************************************************************************
 * Some generic macro definitions follow.  We make an exception here to the
 * usual "TS" prefix because these macros are so common that clarity is improved
 * by using these generic forms.
 *
 * Be careful of side effects when using any of the macros that have arguments.
 *
 */


/*****************************************************************************
 *  Macro for boolean 'true'.
 */
#ifndef TRUE
#define TRUE  (TsBool)1
#endif


/*****************************************************************************
 *  Macro for boolean 'false'.
 */
#ifndef FALSE
#define FALSE (TsBool)0
#endif


/*****************************************************************************
 *  Macro for NULL value.
 */
#ifndef NULL
#define NULL  (void*)0
#endif


/*****************************************************************************
 *  Macro for absolute value.  Watch out for side effects!
 */
#ifndef ABS
#define ABS(x)      (((x)>0)?(x):-(x))
#endif


/*****************************************************************************
 *  Macro for maximum value.  Watch out for side effects!
 */
#ifndef MAX
#define MAX(a,b)    (((a)>(b))?(a):(b))
#endif


/*****************************************************************************
 *  Macro for minimum value.  Watch out for side effects!
 */
#ifndef MIN
#define MIN(a,b)    (((a)<(b))?(a):(b))
#endif



/*****************************************************************************
 *  Macro for determining if a flag is set.
 *
 *  Description:
 *      Returns TRUE if all bits in flag are set, else returns FALSE.
 *      This macro is safe for multibit flags and specifically returns TRUE.
 *      Watch out for side effects!
 *
 */
#define TS_GET_FLAG(bitfield, flag) (TsBool)(((bitfield & flag) == flag))


/*****************************************************************************
 *  Macro for setting a flag (in functional form).
 *
 *  Description:
 *      Turns on the bit(s) corresponding to a flag.
 *      Watch out for side effects!
 *
 */
#define TS_SET_FLAG(bitfield, flag) (bitfield = bitfield | (flag))


/*****************************************************************************
 * Macro for assigning a flag to a value.
 *
 *  Description:
 *      Assigns the bits corresponding to a single flag.
 *      Watch out for side effects!
 *
 */
#define TS_ASSIGN_FLAG(bitfield, flag, val) ((val == FALSE) ? TS_CLEAR_FLAG(bitfield, flag) : TS_SET_FLAG(bitfield, flag)


/*****************************************************************************
 * Macro for clearing a flag (in functional form).  Watch out for side effects!
 *
 * Turns off the bit(s) corresponding to a flag.
 *
 */
#define TS_CLEAR_FLAG(bitfield, flag) (bitfield = (bitfield &  ~(flag)))


/*****************************************************************************
 * Macro to safely divide an integer by 2 using bit shifting.
 *
 * Watch out for side effects!
 *
 */
#define TS_INT32_DIV2(x) (((x) < 0) ? -(TsInt32)(((TsUInt32)-(x)) >> 1) : (TsInt32)(((TsUInt32)(x)) >> 1))


/*****************************************************************************
 * Macro to safely divide an integer by 8 using bit shifting.
 *
 * Watch out for side effects!
 *
 */
#define TS_INT32_DIV8(x) (((x) < 0) ? -(TsInt32)(((TsUInt32)-(x)) >> 3) : (TsInt32)(((TsUInt32)(x)) >> 3))


#endif /* TSPROJ_H */
