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

    @file nbgmconfig.h
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

#ifndef _NBGM_CONFIG_H_
#define _NBGM_CONFIG_H_
#include "pal.h"
#include "nbgmbinarybuffer.h"

/*! NBGM configuration parameters

This struct specifies NBGM global configuration used to construct the namager instance
*/
typedef struct NBGM_Config
{
    PAL_Instance*                       pal;                                /*!< A PAL instance */
    uint8                               preferredLanguageCode;              /*!< Preferred language code for labels. 0 means local language. See NBM format doc for full code list. */
    float                               dpi;                                /*!< Device screen DPI setting */
    const char*                         workSpace;                          /*!< NBGM work space, this folder contains all NBGM internal
                                                                                 resoures such as avatar/sky model texture file, font file*/
    const char*                         binFileTexturePath;                 /*!< Path indicated NBGM where to find ECM/MJO texture files */
    uint32                              nbmOffset;                          /*!< Indicate NBGM the file offset from file header when reading
                                                                                 NBM file. In most case, this is 0, but JCC may change it to 12*/
    nb_boolean                          useRoadBuildingVisibility;          /*!< Use layer far/near visibility or not. Debug purpose function */
    nb_boolean                          showLabelDebugInfo;                 /*!< Debug option: show all languages in labels. */
    float                               relativeCameraDistance;             /*!< camera height on zoom level 17, this value comes from Map Layering - Mapkit3D.xlsx. */
    float                               metersPerPixelOnRelativeZoomLevel;  /*!< Resolution on relative zoom level */
    int8                                relativeZoomLevel;                  /*!< Relative zoom level which is used for calculating current zoom level  */
    float                               fontMagnifierFactor;                /*!< Font magnifier factor */
}NBGM_Config;

#endif
