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

    @file nbgmdevice.h
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

#ifndef _NBGM_DEVICE_H_
#define _NBGM_DEVICE_H_

#include "nbgmexp.h"
#include "paltypes.h"
/*! \addtogroup NBGM_Manager
*  @{
*/
#define NBGM_DLL_EXPORT NBGM_DEC NBGM_DEF
/*! 
This header file provides a series of NBGM C API.
*/

// Device
NBGM_DLL_EXPORT bool CreateNativeDevice(char* resourceFolder, char* tilesFolder, int width, int height, int dpi, bool useOpengl);
NBGM_DLL_EXPORT void DestroyNativeDevice();
NBGM_DLL_EXPORT void StartMainLoop();
NBGM_DLL_EXPORT int IsKeyDown(int nKey);
NBGM_DLL_EXPORT void SnapScreenShot(char* outputfile, char *info);

// NBGM Set Functions
NBGM_DLL_EXPORT void Invalidate();
NBGM_DLL_EXPORT void SetBackground(nb_boolean background);
NBGM_DLL_EXPORT void SetViewCenter(double mercatorX, double mercatorY);
NBGM_DLL_EXPORT void SetHorizonDistance(double horizonDistance);
NBGM_DLL_EXPORT void OnTouchEvent(float screenX, float screenY);
NBGM_DLL_EXPORT void OnPaning(float screenX, float screenY);
NBGM_DLL_EXPORT void SetRotateAngle(float angle);
NBGM_DLL_EXPORT void SetViewPointDistance(float distance);
NBGM_DLL_EXPORT void SetTiltAngle(float angle);
NBGM_DLL_EXPORT void SetViewSize(int32 x, int32 y, uint32 width, uint32 height);
NBGM_DLL_EXPORT void SetPerspective(float fov, float aspect);
NBGM_DLL_EXPORT void LoadCommonMaterial(const char* materialName, const char* filePath);
NBGM_DLL_EXPORT void SetCurrentCommonMaterial(const char* materialName);
NBGM_DLL_EXPORT void SetSkyDayNight(nb_boolean isDay);
NBGM_DLL_EXPORT void LoadNBMTile(const char* nbmName, uint8 baseDrawOrder, uint8 labelDrawOrder, const char* filePath);
NBGM_DLL_EXPORT void UnLoadTile(const char* nbmName);
NBGM_DLL_EXPORT void UnLoadTiles(const char* nbmNames);
NBGM_DLL_EXPORT void SetAvatarLocation(double x, double y, double z, uint64 time, double speed, double accury, double heading);
NBGM_DLL_EXPORT void SetAvatarScale(float scaleValue);
NBGM_DLL_EXPORT void SetAvatarState(int32 state);
NBGM_DLL_EXPORT void SetAvatarMode(int32 mode);
NBGM_DLL_EXPORT void SelectAndTrackAvatar(float x, float y);
NBGM_DLL_EXPORT void LoadPinMaterial(const char* pinMaterialName, const char* filePath);
NBGM_DLL_EXPORT void SelectAndTrackPoi(const char* poiId);
NBGM_DLL_EXPORT void UnselectAndStopTrackingPoi(const char* poiId);
NBGM_DLL_EXPORT void Move(float dx, float dy);
NBGM_DLL_EXPORT void SetRotateCenter(float screenX, float screenY);
NBGM_DLL_EXPORT void Rotate(float angle);
NBGM_DLL_EXPORT void Zoom(float deltaH);
NBGM_DLL_EXPORT void Tilt(float angle);

// NBGM Get Functions
NBGM_DLL_EXPORT float GetTiltAngle();
NBGM_DLL_EXPORT float GetRotateAngle();
NBGM_DLL_EXPORT float GetViewPointDistance();
NBGM_DLL_EXPORT float GetCameraHeight();


/*! @} */
#endif
