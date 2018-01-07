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

    @file nbrerendercommon.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_RENDER_COMMON_H_
#define _NBRE_RENDER_COMMON_H_

/*! \addtogroup NBRE_RenderPal
*  @{
*/

enum NBRE_BlendFunction
{
    NBRE_BF_ZERO,
    NBRE_BF_ONE,
    NBRE_BF_SRC_COLOR,
    NBRE_BF_ONE_MINUS_SRC_COLOR,
    NBRE_BF_SRC_ALPHA,
    NBRE_BF_ONE_MINUS_SRC_ALPHA,
    NBRE_BF_DST_COLOR,
    NBRE_BF_ONE_MINUS_DST_COLOR,
    NBRE_BF_DST_ALPHA,
    NBRE_BF_ONE_MINUS_DST_ALPHA,
};

enum NBRE_FogMode
{
    NBRE_FM_LINEAR,
    NBRE_FM_EXP,
    NBRE_FM_EXP2
};

enum NBRE_CompareFunction
{
    NBRE_CF_NEVER,
    NBRE_CF_LESS,
    NBRE_CF_LESS_EQUAL,
    NBRE_CF_EQUAL,
    NBRE_CF_GREATER,
    NBRE_CF_NOT_EQUAL,
    NBRE_CF_GREATER_EQUAL,
    NBRE_CF_ALWAYS
};

enum NBRE_TextureAddressMode
{
    NBRE_TAM_REPEAT,
    NBRE_TAM_CLAMP,
};

enum NBRE_TextureFilterType
{
    NBRE_TFT_POINT,
    NBRE_TFT_LINEAR,    
    NBRE_TFT_MIPMAP_POINT,
    NBRE_TFT_MIPMAP_LINEAR,
    NBRE_TFT_MIPMAP_TRILINEAR
};

enum NBRE_ClearFlag
{
    NBRE_CLF_CLEAR_COLOR            = 0x1<<0,
    NBRE_CLF_CLEAR_DEPTH            = 0x1<<1,
    NBRE_CLF_CLEAR_STENCIL          = 0x1<<2,
};

/// The rendering operation type to perform
enum NBRE_PrimitiveType
{
    /// A list of points, 1 vertex per point, not supported now
    NBRE_PMT_POINT_LIST,
    /// A list of lines, 2 vertices per line, not supported now
    NBRE_PMT_LINE_LIST,
    /// A strip of connected lines, 1 vertex per line plus 1 start vertex, not supported now, not supported now
    NBRE_PMT_LINE_STRIP,
    /// A list of triangles, 3 vertices per triangle
    NBRE_PMT_TRIANGLE_LIST,
    /// A strip of triangles, 3 vertices for the first triangle, and 1 per triangle after that 
    NBRE_PMT_TRIANGLE_STRIP,
    /// A fan of triangles, 3 vertices for the first triangle, and 1 per triangle after that
    NBRE_PMT_TRIANGLE_FAN
};

/** @} */
#endif
