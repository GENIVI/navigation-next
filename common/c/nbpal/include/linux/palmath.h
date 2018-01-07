/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------

@file palmath.h
@defgroup PALMATH_H PALMATH

@brief Provides an interface to PAL math functions

PAL uses the standard Linux <math.h> functions for the Linux platform.

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALMATH_H
#define PALMATH_H

#ifdef __cplusplus
#include <cmath>
#else
#include <math.h>
#endif

#ifndef PI
#define PI_OVER_TWO     1.57079632679489661923                  /*!< PI/2 constant */
#define PI              3.14159265358979323846                  /*!< PI constant */
#define TWO_PI          6.28318530717958647692                  /*!< PI*2 constant */
#endif

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))                      /*!< Macro that returns the max between 2 numbers */
#endif
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))                      /*!< Macro that returns the min between 2 numbers */
#endif

#ifdef __cplusplus
#define nsl_sin(x) std::sin(x)                                       /*!< sine function macro */
#define nsl_cos(x) std::cos(x)                                       /*!< cosine function macro */
#define nsl_tan(x) std::tan(x)                                       /*!< tangent function macro */
#define nsl_asin(x) std::asin(x)                                     /*!< arc sine function macro */
#define nsl_atan(x) std::atan(x)                                     /*!< arc tangent function macro */
#define nsl_acos(x) std::acos(x)                                     /*!< arc cosine function macro */
#define nsl_sqrt(x) std::sqrt(x)                                     /*!< square root function macro */
#define nsl_fabs(x) std::fabs(x)                                     /*!< absolute function macro */
#define nsl_log(x) std::log(x)                                       /*!< logarithm function macro */
#define nsl_log10(x) std::log10(x)                                   /*!< common logarithm function macro */
#define nsl_log2(x) (std::log(x)/std::log(2.0))                           /*!< base 2 logarithm function macro */
#define nsl_sec(x) (1.0/std::cos(x))                                 /*!< inverse cosine function macro */
#define nsl_sinh(x) std::sinh(x)                                     /*!< sinh function macro */
#define nsl_add(a,b) ((a) + (b))                                /*!< addition function macro */
#define nsl_sub(a,b) ((a) - (b))                                /*!< subtraction function macro */
#define nsl_mul(a,b) ((a) * (b))                                /*!< multiplication function macro */
#define nsl_div(a,b) ((a) / (b))                                /*!< division function macro */
#define nsl_pow(a,p) std::pow((double)(a),(double)(p))               /*!< involution function macro */
#define nsl_floor(x) std::floor((double)(x))                         /*!< computes the largest integral value not greater than x */
#define nsl_isnan(x) std::isnan(x)                                   /*!< determine whether its argument value is a NaN. */
#else
#define nsl_sin(x) sin(x)                                       /*!< sine function macro */
#define nsl_cos(x) cos(x)                                       /*!< cosine function macro */
#define nsl_tan(x) tan(x)                                       /*!< tangent function macro */
#define nsl_asin(x) asin(x)                                     /*!< arc sine function macro */
#define nsl_atan(x) atan(x)                                     /*!< arc tangent function macro */
#define nsl_acos(x) acos(x)                                     /*!< arc cosine function macro */
#define nsl_sqrt(x) sqrt(x)                                     /*!< square root function macro */
#define nsl_fabs(x) fabs(x)                                     /*!< absolute function macro */
#define nsl_log(x) log(x)                                       /*!< logarithm function macro */
#define nsl_log10(x) log10(x)                                   /*!< common logarithm function macro */
#define nsl_log2(x) (log(x)/log(2.0))                           /*!< base 2 logarithm function macro */
#define nsl_sec(x) (1.0/cos(x))                                 /*!< inverse cosine function macro */
#define nsl_sinh(x) sinh(x)                                     /*!< sinh function macro */
#define nsl_add(a,b) ((a) + (b))                                /*!< addition function macro */
#define nsl_sub(a,b) ((a) - (b))                                /*!< subtraction function macro */
#define nsl_mul(a,b) ((a) * (b))                                /*!< multiplication function macro */
#define nsl_div(a,b) ((a) / (b))                                /*!< division function macro */
#define nsl_pow(a,p) pow((double)(a),(double)(p))               /*!< involution function macro */
#define nsl_floor(x) floor((double)(x))                         /*!< computes the largest integral value not greater than x */
#define nsl_isnan(x) isnan(x)                                   /*!< determine whether its argument value is a NaN. */
#endif
#define NSL_INFINITY INFINITY                                   /*!< wrapper of INFINITY */
#endif

/*! @} */
