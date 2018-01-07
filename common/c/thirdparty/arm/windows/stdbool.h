/* stdbool.h: ISO/IEC 9899:1999 (C99), section 7.16 */

/* Copyright (C) ARM Ltd., 2002
 * All rights reserved
 * RCS $Revision: 91721 $
 * Checkin $Date: 2005-08-08 19:30:07 +0100 (Mon, 08 Aug 2005) $
 * Revising $Author: drodgman $
 */

#ifndef __bool_true_false_are_defined
#define __bool_true_false_are_defined 1

  #ifndef __cplusplus /* In C++, 'bool', 'true' and 'false' and keywords */
    #define bool _Bool
    #define true 1
    #define false 0
  #endif

#endif /* __bool_true_false_are_defined */
