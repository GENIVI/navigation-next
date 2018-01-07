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
 @file     MapViewAndroid.h
 */
/*
 (C) Copyright 2013 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef MAPVIEWANDROID_H_
#define MAPVIEWANDROID_H_

#include <pthread.h>
#include "mapcontroller.h"

class AndroidNativeUIListener
{
public:
    AndroidNativeUIListener(jobject androidObj);
    virtual ~AndroidNativeUIListener();

    virtual bool OnSingleTap(float x, float y, std::set<nbmap::Geographic*>& graphics);
    virtual bool OnGeoGraphicSelect(nbmap::Geographic* graphic, bool isSelected);
    virtual void OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt);
    virtual void OnTap(double latitude, double longitude);
    void UpdateOptionalLayers(const std::vector<nbmap::LayerAgentPtr>& layers);
    void OnPinClicked(nbmap::MapPin* pin);
    void OnStaticPOIClicked(const string &id, const string &name, double latitude, double longitude);
    void OnTrafficIncidentPinClicked(double latitude, double longitude);
    void OnAvatarClicked(double lat, double lon);
    void OnMarkerClicked(int markerId);
    void OnMapCreate();
    void OnMapReady();
    void OnCameraAnimationDone(int animationId, int animationStatus);
    void OnNightModeChanged(bool isNightMode);
    void OnCameraLimited(int limitationType);

private:
    jobject mAndroidObject;
};

class JniMapListener;
class JniGeographicSelectedListener;

struct MapContext
{
    nbmap::MapController* mapController;
    JniMapListener* jniMapListener;
    std::map<nbmap::Geographic*, shared_ptr<JniGeographicSelectedListener> > graphicListeners;
    AndroidNativeUIListener* androidObject;
    float scaleFactor;
};

extern "C" int register_mapview(JavaVM* vm, JNIEnv* env);
extern "C" int unregister_mapview(JNIEnv* );

#endif /* MAPVIEWANDROID_H_ */
