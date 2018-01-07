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

    @file nbgm.h
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

#ifndef _NBGM_H_
#define _NBGM_H_
#include "nbgmexp.h"
#include "nbgmconfig.h"
#include "nbgmmapview.h"
#include "nbgmmapviewconfig.h"
#include "nbgmnavview.h"
#include "nbgmnavviewconfig.h"
#include "nbgmresourcemanagerconfig.h"
#include "palerror.h"
#include "pallog.h"

class NBGM_ResourceManager;
class NBGM_MapCameraHelper;

/*! \addtogroup NBGM_Manager
*  @{
*/

/*! Initialize NBGM instance.

This function initialize the NBGM instance using the config parameters supplied by the NBRE_Config param

@param config The NBGM configuration parameters
@return PAL_Error error code
*/
NBGM_DEC PAL_Error NBGM_Initialize(const NBGM_Config* config);

/*! Create  a Map view instance from NBGM.
@return PAL_Error error code
*/
NBGM_DEC PAL_Error NBGM_CreateMapView(NBGM_MapViewConfig* config, NBGM_MapView** mapView);

/*! Get a Nav view instance from NBGM.
@return PAL_Error error code
*/
NBGM_DEC PAL_Error NBGM_CreateNavView(NBGM_NavViewConfig* config, NBGM_NavView** navView);

/*! Get a Nav view instance from NBGM with particular resourcemanager.
@return PAL_Error error code
*/
NBGM_DEC PAL_Error NBGM_CreateMapViewWithSharedResource(NBGM_MapViewConfig* config, NBGM_ResourceManager* resourceManager, NBGM_MapView** mapView);

/*! Get a Nav view instance from NBGM with particular resourcemanager.
@return PAL_Error error code
*/
NBGM_DEC PAL_Error NBGM_CreateNavViewWithSharedResource(NBGM_NavViewConfig* config, NBGM_ResourceManager* resourceManager, NBGM_NavView** navView);

/*! Get current supported NBM file major version.

@return supported NBM file major version.
*/
NBGM_DEC uint32 NBGM_GetSupportedNBMFileVersion();

/*! Get nbgm config.

@return nbgm config.
*/
NBGM_DEC const NBGM_Config* NBGM_GetConfig();

/*! Set NBGM debug log severity level.

@return none.
*/
NBGM_DEC void NBGM_SetDebugLogSeverity(PAL_LogSeverity severity);


/*! Set use NBM tile layer far/nare visibility to contoll show & hide rank 4 roads
 @param enable ture to enable, false to disable
 @return none
 */
NBGM_DEC void NBGM_EnableLayerFarNearVisibility(nb_boolean enable);

/*! Set preferred language code for labels.
    Warning: Already loaded tiles will not change preferred language. Client must reload them manually.
 @param preferredLanguageCode Preferred language code for labels. 0 means local language. See NBM format doc for full code list.
 @return none
 */
NBGM_DEC void NBGM_SetPreferredLanguageCode(uint8 preferredLanguageCode);

/*! Enable/Disable label debug info.
  Warning: Loaded tiles will not change until reloaded.
 @param enable ture to enable, false to disable
 @return none
 */
NBGM_DEC void NBGM_ShowLabelDebugInfo(nb_boolean enable);

/*! Set relative zoom level.
 @param zoomlevel relative zoom level which is used to calculate current zoom level.
 @return none
 */
NBGM_DEC void NBGM_SetRelativeZoomLevel(int8 zoomlevel);

/*! Set relative camera height.
 @param distance relative distance which is used to calculate current zoom level.
 @return none
 */
NBGM_DEC void NBGM_SetRelativeCameraHeight(float distance);

/*! Set resolution on relative zoom level.
 @param resolution. It is used to calculate current zoom level.
 @return none
 */
NBGM_DEC void NBGM_SetResolutionRelativeZoomLevel(float resolution);

/*! Create an instance of ResourceManger from NBGM.
@return PAL_Error error code
*/
NBGM_DEC PAL_Error NBGM_CreateResourceManager(const NBGM_ResourceManagerConfig* resourceManagerConfig, NBGM_ResourceManager** resourceManager);

/*! Destory an instance of ResourceManger from NBGM.
@return PAL_Error error code
*/
NBGM_DEC PAL_Error NBGM_DestroyResourceManager(NBGM_ResourceManager** resourceManager);

/*! Create an instance of MapCamera from NBGM.
@return PAL_Error error code
*/
NBGM_DEC PAL_Error NBGM_CreateMapCameraHelper(NBGM_MapCameraHelper** mapCameraHelper);

/*! Set DPI.
@return none
*/
NBGM_DEC void NBGM_SetDPI(float dpi);
/*! @} */
#endif
