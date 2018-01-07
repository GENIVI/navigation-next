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

@file transformutility.h
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

#ifndef TRANSFORM_UTILITY_H
#define	TRANSFORM_UTILITY_H

#include "paltypes.h"
#include "navpublictypes.h"
#include "nbspatial.h"
#include "cslutil.h"
#include "nbexp.h"

#define RADIUS_EARTH_METERS 6378137.0

#define	DEG_TO_RAD_MUL	0.017453292519943295769222222222222 /*!< Constant for degrees to radians conversion */
#define RAD_TO_DEG_MUL	57.295779513082320876846364344191   /*!< Constant for radians to degrees conversion */

NB_DEC double TO_DEG(double rad);
NB_DEC double TO_RAD(double deg);

boolean	compare_latlon(double lat1, double lon1, double lat2, double lon2, double tolerance);

boolean IsLatitudeValid(double lat);

boolean IsLongitudeValid(double lon);

boolean IsLatitudeLongitudeValid(NB_LatitudeLongitude* point);

NB_DEC void mercatorForward(double lat, double lon, double* x, double* y);
NB_DEC void mercatorReverse(double x, double y, double* lat, double* lon);

#endif

/*! @} */
