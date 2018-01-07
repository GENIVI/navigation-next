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

    @file     nbsolarcalculator.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBSOLARCALCULATOR_H
#define NBSOLARCALCULATOR_H

/*!
    @addtogroup nbsolarcalculator
    @{
*/

#include "paltypes.h"
#include "nberror.h"
#include "nbexp.h"

// Constants ....................................................................................


// Types ........................................................................................


// Functions ....................................................................................

/*! Get calculated time of sunrise and sunset for the date and location

    This function is used to calculate time of sunrise and sunset for specified date and location.

    @param latitude     Latitude of observer in degrees.
    @param longitude    Longitude of observer in degrees.
    @param timeZone     Time zone value
    @param year         Year
    @param month        Month
    @param day          Day
    @param sunriseTime  Pointer to double where sunrise time in minutes will be stored
    @param sunsetTime   Pointer to double where sunset time in minutes will be stored

    @return NE_OK for success
*/
NB_DEC NB_Error
NB_SolarCalculatorGetTime(double latitude,
                          double longitude,
                          int16 timeZone,
                          nb_boolean useDaySaving,
                          uint16 year,
                          uint16 month,
                          uint16 day,
                          double* sunriseTime,
                          double* sunsetTime);

/*! @} */

#endif
