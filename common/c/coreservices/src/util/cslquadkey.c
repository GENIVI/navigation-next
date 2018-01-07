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

    @file     cslquadkey.h
    @date     01/20/2010
    @defgroup COREUTIL_API_GROUP         NIM Core Quadkey Util
*/
/*
    See description in header file.

    (C) Copyright 2010 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/* @{ */


#include "cslquadkey.h"


/* See description in header file */
CSL_DEF NB_Error
CSL_QuadkeyConvertToXYZ(const char* quadkey, uint32* x, uint32* y, uint8* zoom)
{
    // Currently not needed. Implement if needed.
    return NE_NOSUPPORT;
}

/* See description in header file */
CSL_DEF NB_Error
CSL_QuadkeyConvertFromXYZ(uint32 x, uint32 y, uint8 zoom, char* quadkey)
{
    int digits = 0;
    uint32 maxXY = 2 << (zoom - 1);

    if ((zoom > CSL_MAXIMUM_ZOOM) || (zoom < CSL_MINIMUM_ZOOM) || (x >= maxXY) || (y >= maxXY))
    {
        return NE_INVAL;
    }

    // We have one quadkey digit for every zoom level
    for (digits = (signed)zoom - 1; digits >= 0; --digits)
    {
        // The quadkey is determined by the bit-values of the x/y coordinates. Very simple/elegant conversion (once figured out).
        quadkey[digits] = (char)(0x30 + (0x01 & x) + ((0x01 & y) * 2));

        x = x >> 1;
        y = y >> 1;
    }

    // Null-terminate string
    quadkey[zoom] = '\0';

    return NE_OK;
}


/*! @} */
