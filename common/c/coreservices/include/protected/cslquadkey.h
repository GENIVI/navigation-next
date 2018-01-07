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

    This API provides an interface for map tile coordinate conversions from/to
    x/y/z coordinates to/from quadkey coordinates.

    See more about the quadkey coordinate system here:
    http://msdn.microsoft.com/en-us/library/bb259689.aspx

    For x/y/z to quadkey conversion see:
    http://www.maptiler.org/google-maps-coordinates-tile-bounds-projection/

*/
/*
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

#ifndef CSL_QUADKEY_H
#define	CSL_QUADKEY_H

#include "paltypes.h"
#include "pal.h"
#include "palstdlib.h"
#include "cslexp.h"
#include "nberror.h"


#define CSL_MAXIMUM_QUADKEY_LENGTH 32
#define CSL_MINIMUM_ZOOM 1              // quadkey coordinates start at zoom level 1 and not zero
#define CSL_MAXIMUM_ZOOM 30             // 23 or 24 should be sufficient, but it doesn't hurt to go bigger.


/*! Convert map tile quadkey coordinates to x/y/z coordinates.

    @return NE_OK on success
*/
CSL_DEC NB_Error
CSL_QuadkeyConvertToXYZ(const char* quadkey,            /*<! Quadkey to convert */
                        uint32* x,                      /*<! On return the converted x-coordinate */
                        uint32* y,                      /*<! On return the converted y-coordinate */
                        uint8* zoom                     /*<! On return the converted zoom */
                        );

/*! Convert map tile x/y/z coordinates to quadkey coordinates.

    @return NE_OK on success
*/
CSL_DEC NB_Error
CSL_QuadkeyConvertFromXYZ(uint32 x,                     /*<! x-coordinate to convert */
                          uint32 y,                     /*<! y-coordinate to convert */
                          uint8 zoom,                   /*<! zoom to convert */
                          char* quadkey                 /*<! On return contains the converted quadkey. Has to be at least of length CSL_MAXIMUM_QUADKEY_LENGTH */
                          );


#endif // CSL_QUADKEY_H

/*! @} */


