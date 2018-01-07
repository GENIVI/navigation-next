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

/*--------------------------------------------------------------------------

 (C) Copyright 2012 by Networks In Motion, Inc.

 The information contained herein is confidential, proprietary
 to Networks In Motion, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
#include "nbgmdevice.h"
#include "nbgmdeviceimpl.h"

extern NBGMDeviceImpl* GetDevice();

bool CreateNativeDevice(char* resourceFolder, char* tilesFolder,  int width, int height, int dpi, bool useOpengl)
{
   bool ret = GetDevice()->CreateDevice(resourceFolder, tilesFolder, width, height, dpi, useOpengl);
   return ret?GetDevice()->CreateNBGM():false;
}

void DestroyNativeDevice()
{
	GetDevice()->DestroyNBGM();
	GetDevice()->DestroyDevice();
}

void StartMainLoop()
{  GetDevice()->Run(); }

int IsKeyDown(int c)
{ return GetDevice()->IsKeyDown(c); }

void SnapScreenShot(char* outputfile, char* info)
{ return GetDevice()->SnapScreenShot(outputfile, info); }

void Invalidate()
{ GetDevice()->Invalidate(); }

void SetBackground(nb_boolean background)
{ GetDevice()->SetBackground(background?true:false);}

void SetViewCenter(double mercatorX, double mercatorY)
{ GetDevice()->SetViewCenter(mercatorX, mercatorY); }

void SetHorizonDistance(double horizonDistance)
{ GetDevice()->SetHorizonDistance(horizonDistance); }

void OnTouchEvent(float screenX, float screenY)
{ GetDevice()->OnTouchEvent(screenX, screenY); }

void OnPaning(float screenX, float screenY)
{ GetDevice()->OnPaning(screenX, screenY); }

void SetRotateAngle(float angle)
{ GetDevice()->SetRotateAngle(angle); }

void SetViewPointDistance(float distance)
{ GetDevice()->SetViewPointDistance(distance); }

void SetTiltAngle(float angle)
{ GetDevice()->SetTiltAngle(angle); }

void SetViewSize(int32 x, int32 y, uint32 width, uint32 height)
{ GetDevice()->SetViewSize(x, y, width, height); }

void SetPerspective(float fov, float aspect)
{ GetDevice()->SetPerspective(fov, aspect); }

void LoadCommonMaterial(const char* materialName, const char* filePath)
{ GetDevice()->LoadCommonMaterial(materialName, filePath); }

void SetCurrentCommonMaterial(const char* materialName)
{ GetDevice()->SetCurrentCommonMaterial(materialName); }

void SetSkyDayNight(nb_boolean isDay)
{ GetDevice()->SetSkyDayNight(isDay); }

void LoadNBMTile(const char* nbmName, uint8 baseDrawOrder, uint8 labelDrawOrder, const char* filePath)
{ GetDevice()->LoadNBMTile(nbmName, baseDrawOrder, labelDrawOrder, filePath); }

void UnLoadTile(const char* nbmName)
{ GetDevice()->UnLoadTile(nbmName); }

void UnLoadTiles(const char* nbmNames)
{ GetDevice()->UnLoadTiles(nbmNames); }

void SetAvatarLocation(double x, double y, double z, uint64 time, double speed, double accury, double heading)
{ GetDevice()->SetAvatarLocation(x, y, z, time, speed, accury, heading); }

void SetAvatarScale(float scaleValue)
{ GetDevice()->SetAvatarScale(scaleValue); }

void SetAvatarState(int32 state)
{ GetDevice()->SetAvatarState(state); }

void SetAvatarMode(int32 mode)
{ GetDevice()->SetAvatarMode(mode); }

void SelectAndTrackAvatar(float x, float y)
{ GetDevice()->SelectAndTrackAvatar(x, y); }

void LoadPinMaterial(const char* pinMaterialName, const char* filePath)
{ GetDevice()->LoadPinMaterial(pinMaterialName, filePath); }

void SelectAndTrackPoi(const char* poiId)
{ GetDevice()->SelectAndTrackPoi(poiId); }

void UnselectAndStopTrackingPoi(const char* poiId)
{ GetDevice()->UnselectAndStopTrackingPoi(poiId); }

void Move(float dx, float dy)
{ GetDevice()->Move(dx, dy); }

void SetRotateCenter(float screenX, float screenY)
{ GetDevice()->SetRotateCenter(screenX, screenY); }

void Rotate(float angle)
{ GetDevice()->Rotate(angle); }

void Zoom(float deltaH)
{ GetDevice()->Zoom(deltaH); }

void Tilt(float angle)
{ GetDevice()->Tilt(angle); }

float GetTiltAngle()
{ return GetDevice()->GetTiltAngle(); }

float GetRotateAngle()
{ return GetDevice()->GetRotateAngle(); }

float GetViewPointDistance()
{ return GetDevice()->GetViewPointDistance(); }

float GetCameraHeight()
{ return GetDevice()->GetCameraHeight(); }
