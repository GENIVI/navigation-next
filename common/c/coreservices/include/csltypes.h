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

@file csltypes.h
@defgroup csltypes Common Core Types

Provides version and copyright information about the component.
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

#ifndef CSLTYPES_H
#define CSLTYPES_H

#include "paltypes.h"

typedef uint32       nb_color;

#define MAKE_NB_COLOR(r,g,b)         (nb_color)(((uint32)(r)<<8) + ((uint32)(g) << 16) + ((uint32)(b) << 24))
#define MAKE_NB_COLOR_ALPHA(r,g,b,a) (nb_color)(((uint32)(r)<<8) + ((uint32)(g) << 16) + ((uint32)(b) << 24) + ((uint32) (a)))

#define NB_GET_BYTE(dw, i)      (byte)((dw >> i*8) & 0xff)
#define NB_GET_BYTE_0(dw)       NB_GET_BYTE(dw,0)
#define NB_GET_BYTE_1(dw)       NB_GET_BYTE(dw,1)
#define NB_GET_BYTE_2(dw)       NB_GET_BYTE(dw,2)
#define NB_GET_BYTE_3(dw)       NB_GET_BYTE(dw,3)

#define NB_COLOR_GET_R(dw)      NB_GET_BYTE_1(dw)
#define NB_COLOR_GET_G(dw)      NB_GET_BYTE_2(dw)
#define NB_COLOR_GET_B(dw)      NB_GET_BYTE_3(dw)
#define NB_COLOR_GET_A(dw)      NB_GET_BYTE_0(dw)

#define NB_SET_BYTE(dw, b, i)   (dw | (((dw & 0) | b) << i*8))
#define NB_SET_BYTE_0(dw, b)    ((dw & 0xffffff00) | (((dw & 0) | b) << 0*8))
#define NB_SET_BYTE_1(dw, b)    ((dw & 0xffff00ff) | (((dw & 0) | b) << 1*8))
#define NB_SET_BYTE_2(dw, b)    ((dw & 0xff00ffff) | (((dw & 0) | b) << 2*8))
#define NB_SET_BYTE_3(dw, b)    ((dw & 0x00ffffff) | (((dw & 0) | b) << 3*8))

#define INVALID_LATLON          (-999.0)

#endif

/*! @} */

