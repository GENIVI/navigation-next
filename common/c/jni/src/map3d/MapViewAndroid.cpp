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
 @file     MapViewAndroid.cpp
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

#include "MapViewAndroid.h"
#include "paldebuglog.h"
#include "palstdlib.h"
#include "palclock.h"
#include <assert.h>
#include <jni.h>
#include "MapServicesConfiguration.h"
#include <nbnetwork.h>
#include <nbnetworkoptions.h>
#include <palfile.h>
#include "MetadataConfiguration.h"
#include <android/log.h>
#include "JNIUtility.h"
#include "nativeltkcontext.h"
#include "nblocationandroid.h"
#include <algorithm>
#include "custompinbubbleresolverandroid.h"
#include "BubbleAndroid.h"
#include "datastreamimplementation.h"
#include "camerahelper.h"
#include "mapcircle.h"
#include "maprect2d.h"
#include "maptile.h"
#include "tileproviderandroid.h"
#include "maplayer.h"
#include "AndroidNativeHostView.h"

#define  MAX_PATH_LENGTH 512

static const char PIN_MATERIAL_CONFIG_FILENAME[] = "pin_materials.xml";
static const char PERSISTENT_DATA_FILENAME[] = "/mnt/sdcard/nbserviceresource/ccc.bin";

using namespace nbmap;

/** Memory region:
 +------------------------------------+
 |      Layer Numer (4 bytes)         |
 +------+-----------------+-----------+
 | Flag | LayerNameLength | LayerName |
 +------+-----------------+-----------+
 | Flag | LayerNameLength | LayerName |
 +------+-----------------+-----------+
 |               ...                  |
 +------+-----------------+-----------+

*/

class LayerAgentSerializer
{
public:
    LayerAgentSerializer(const std::vector<LayerAgentPtr>& layers)
            : mBuffer(NULL),
              mSize(0),
              mValid(false)
    {
        mSize   = sizeof(int);
        mBuffer = (byte*)nsl_malloc(mSize);
        nsl_memset(mBuffer, 0, mSize);
        if (mBuffer)
        {
            mValid = true;
            vector<LayerAgentPtr>::const_iterator iter = layers.begin();
            vector<LayerAgentPtr>::const_iterator end  = layers.end();
            for (; iter != end; ++iter)
            {
                SerializeLayer(*iter);
            }
        }
    }

    virtual ~LayerAgentSerializer()
    {
        if (mBuffer)
        {
            nsl_free(mBuffer);
        }
    }

    byte* GetBuffer() const
    {
        return mBuffer;
    }

    uint32 GetBufferSize() const
    {
        return mSize;
    }

private:
    void SerializeLayer(LayerAgentPtr layer)
    {
        shared_ptr<string> layerName = layer ? layer->GetName():shared_ptr<string>();
        if (!mValid || !layerName || layerName->empty())
        {
            return;
        }

        int    size    = sizeof(int) + sizeof(bool)  + layerName->size() + 1;
        uint32 offset  = mSize;
        mSize         += size;
        mBuffer        = (byte*)nsl_realloc(mBuffer, mSize);

        unsigned char* ptr = mBuffer + offset; // seek to appended region
        nsl_memset(ptr, 0, size);

        // Flag, sizeof(string), realString

        *((bool*)ptr) = layer->IsEnabled();
        ptr += sizeof(bool);

        *((int*)ptr) = size - sizeof(int) - sizeof(bool);
        ptr += sizeof(int);

        nsl_memcpy(ptr, layerName->c_str(), layerName->size());

        (*(int*)mBuffer)++; // update layer count.
    }


    byte*  mBuffer;
    uint32 mSize;
    bool mValid;
};

struct fields_t
{
    jclass clazz;
    jfieldID context;
    jmethodID setMapViewContextMethodID;
    jmethodID onSingleTapMethodID;
    jmethodID onCameraUpdateMethodID;
    jmethodID onGeoGraphicSelectMethodID;
    jmethodID onMapCreatedMethodID;
    jmethodID onOptionalLayerUpdatedMethodID;
    jmethodID onPinClickedMethodID;
    jmethodID onStaticPOIClickedMethodID;
    jmethodID onTraficInsidentPinClickedMethodID;
    jmethodID onAvatarClickedID;
    jmethodID onMapReadyID;
    jmethodID onCameraAnimationDoneID;
    jmethodID onCameraLimitedID;
    jmethodID onNightModeChangedID;
    jmethodID onMarkerClickedMethodID;
    jmethodID onTapMethodID;

    jclass place_clazz;
    jfieldID longitude;
    jfieldID latitude;

    jclass snapshotwrapper_clazz;
    jmethodID onSnapShotSuccessed;
    jmethodID onSnapShotError;
};

static fields_t fields;
static JavaVM* cachedVM = NULL;

JNIEnv* getJNIEnv()
{
    JNIEnv* env = NULL;
    if (cachedVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        // Should never happen
        assert(false);
    }
    return env;
}

class LayerAgentFindByNameFunctor
{
public:
    LayerAgentFindByNameFunctor(const char* name)
            : mName(name) {}
    virtual ~LayerAgentFindByNameFunctor(){}
    bool operator()(const LayerAgentPtr& layer) const
    {
        return mName && layer && layer->GetName() && !(layer->GetName()->compare(mName));
    }
private:
    const char* mName;
};

/*! Helper function to enable or disable an optional layer specified by layerName. */
void EnableOptionalLayer(MapContext* context, const char* layerName, bool enabled)
{
    if (context && context->mapController && layerName)
    {
        vector<LayerAgentPtr> layers = context->mapController->GetOptionalLayers();
        if (!layers.empty())
        {
            vector<LayerAgentPtr>::iterator iter = find_if(layers.begin(), layers.end(),
                                                           LayerAgentFindByNameFunctor(layerName));
            if (iter != layers.end())
            {
                (*iter)->SetEnabled(enabled);
            }
        }
    }
}

class MapInitializeCallback : public AsyncCallback<void*>
{
public:
    MapInitializeCallback(){};
    virtual ~MapInitializeCallback(){};

    /* Overwrites from AsyncCallback. This gets called once the map is fully initialized (or the initialization failed) */
    virtual void Success(void*){};
    virtual void Error(NB_Error error){};

};

class MapDestroyCallback : public AsyncCallback<void*>
{
public:
    MapDestroyCallback(){};
    virtual ~MapDestroyCallback(){};

    /* Overwrites from AsyncCallback. This gets called once the map is fully initialized (or the initialization failed) */
    virtual void Success(void*){};
    virtual void Error(NB_Error error){};

};

class JniMapListener:
    public MapListener
{
public:
    JniMapListener():mAndroidObject(NULL) {}
    virtual ~JniMapListener() {}

    void SetAndroidNativeUIListener(AndroidNativeUIListener* androidObject) { mAndroidObject = androidObject; }

    virtual bool OnSingleTap( float screenX, float screenY, std::set<Geographic*>& graphics);
    virtual void OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt);
    virtual void OnOptionalLayerUpdated(const std::vector<LayerAgentPtr>& layers)
    {
        mAndroidObject->UpdateOptionalLayers(layers);
    }
    virtual void OnPinClicked(MapPin* pin);
    virtual void OnStaticPOIClicked(const string &id, const string &name, double lat, double lon);
    virtual void OnTrafficIncidentPinClicked(double lat, double lon);
    virtual void OnAvatarClicked(double lat, double lon);
    virtual void OnMapCreate();
    virtual void OnMapReady();
    virtual void OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus);
    virtual void OnNightModeChanged(bool isNightMode);
    virtual void OnTap(double latitude, double longitude);
    virtual void OnCameraLimited(MapViewCameraLimitedType type);
    virtual void OnMarkerClicked(int id);
	virtual void OnLongPressed(double lat, double lon);

private:
    AndroidNativeUIListener* mAndroidObject;
};


bool
JniMapListener::OnSingleTap(float screenX, float screenY, std::set<Geographic*>& graphics)
{
    return mAndroidObject ? mAndroidObject->OnSingleTap(screenX, screenY, graphics) : false;
}

void
JniMapListener::OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt)
{
    if(mAndroidObject)
    {
        mAndroidObject->OnCameraUpdate(lat, lon, zoomLevel, heading, tilt);
    }
}

void
JniMapListener::OnPinClicked(MapPin* pin)
{
    if(mAndroidObject)
    {
        mAndroidObject->OnPinClicked(pin);
    }
}

void
JniMapListener::OnAvatarClicked(double lat, double lon)
{
    if(mAndroidObject)
    {
        mAndroidObject->OnAvatarClicked(lat, lon);
    }
}

void
JniMapListener::OnMarkerClicked(int id)
{
    if(mAndroidObject)
    {
        mAndroidObject->OnMarkerClicked(id);
    }
}

void
JniMapListener::OnStaticPOIClicked(const string &id, const string &name, double lat, double lon)
{
    if(mAndroidObject)
    {
        mAndroidObject->OnStaticPOIClicked(id, name, lat, lon);
    }
}

void
JniMapListener::OnTrafficIncidentPinClicked(double lat, double lon)
{
    if(mAndroidObject)
    {
        mAndroidObject->OnTrafficIncidentPinClicked(lat, lon);
    }
}

void
JniMapListener::OnMapCreate()
{
    if(mAndroidObject)
    {
        mAndroidObject->OnMapCreate();
    }
}

void
JniMapListener::OnMapReady()
{
    if(mAndroidObject)
    {
        mAndroidObject->OnMapReady();
    }
}

void
JniMapListener::OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus)
{
    if(mAndroidObject)
    {
        mAndroidObject->OnCameraAnimationDone(animationId, static_cast<int>(animationStatus));
    }
}

void
JniMapListener::OnCameraLimited(MapViewCameraLimitedType type)
{
    if(mAndroidObject)
    {
        mAndroidObject->OnCameraLimited(static_cast<int>(type));
    }
}

void JniMapListener::OnNightModeChanged(bool isNightMode)
{
    if(mAndroidObject)
    {
        mAndroidObject->OnNightModeChanged(isNightMode);
    }
}

void
JniMapListener::OnTap(double latitude, double longitude)
{
    mAndroidObject->OnTap(latitude, longitude);
}

void
JniMapListener::OnLongPressed(double lat, double lon)
{
}

class JniGeographicSelectedListener:
    public GeographicSelectedListener
{
public:
    JniGeographicSelectedListener(MapContext& context, Geographic* graphic)
        :mContext(context), mGraphic(graphic) {}
    virtual ~JniGeographicSelectedListener() {}

    virtual bool OnSelected( bool isSelected )
    {
        return mContext.androidObject ? mContext.androidObject->OnGeoGraphicSelect(mGraphic, isSelected) : false;
    }

private:
    MapContext& mContext;
    Geographic* mGraphic;
};

class JniSnapShotCallback : public SnapShotCallback
{
public:
    JniSnapShotCallback(jobject obj);
    virtual ~JniSnapShotCallback();

public:
    virtual void OnSuccessed(nbcommon::DataStreamPtr dataStream, int width, int height);
    virtual void OnError(int code);

private:
    jobject mObject;
};

JniSnapShotCallback::JniSnapShotCallback(jobject obj)
    :mObject(NULL)
{
    if(obj)
    {
        JNIEnv* env = getJNIEnv();
        mObject = env->NewGlobalRef(obj);
    }
}

JniSnapShotCallback::~JniSnapShotCallback()
{
    if(mObject)
    {
        JNIEnv* env = getJNIEnv();
        env->DeleteGlobalRef(mObject);
    }
}

void
JniSnapShotCallback::OnSuccessed(nbcommon::DataStreamPtr dataStream, int width, int height)
{
    if(mObject)
    {
        JNIEnv* env = getJNIEnv();
        uint32 size = sizeof(uint8) * dataStream->GetDataSize();
        uint8* stream = (uint8*) nsl_malloc(size);
        dataStream->GetData(stream, 0, size);
        jbyteArray array = env->NewByteArray(size);
        env->SetByteArrayRegion(array, 0, size, (jbyte*) (stream));
        nsl_free(stream);
        env->CallVoidMethod(mObject, fields.onSnapShotSuccessed, array, static_cast<jint>(width), static_cast<jint>(height));
        env->DeleteLocalRef(array);
    }
}

void
JniSnapShotCallback::OnError(int code)
{
    if(mObject)
    {
        JNIEnv* env = getJNIEnv();
        env->CallVoidMethod(mObject, fields.onSnapShotError, static_cast<jint>(code));
    }
}

//static NativeLTKContext*
//GetNativeNBContext(JNIEnv* env, jobject nativeContext)
//{
//    return reinterpret_cast<NativeLTKContext*>(env->CallIntMethod(nativeContext, fields.getContextMethodID));
//}

static void JNI_CreateMapContext(JNIEnv* env, jobject thiz, jint ltkContext)
{
    NativeLTKContext* nativenbcontext = reinterpret_cast<NativeLTKContext*>(ltkContext);
    if(nativenbcontext)
    {
        MapContext* context = new MapContext();
        context->mapController = new MapController(nativenbcontext->nbContext, nativenbcontext->pal);
        context->jniMapListener = new JniMapListener();
        context->mapController->SetMapListener(context->jniMapListener);
        context->androidObject = NULL;
        context->scaleFactor = 1.f;
        env->SetIntField(thiz, fields.context, (int)context);
    }
}

static void JNI_DestroyMapContext(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context)
    {
        delete context->mapController;
        delete context;
    }
}

static void JNI_CreateMap(JNIEnv* env, jobject thiz, jdouble latitude, jdouble longitude, jdouble tilt, jint maximumCachingTileCount, jint maximumTileRequestCountPerLayer, jint zorderLevel,
                          jboolean enableFullScreenAntiAliasing, jboolean enableAnisotropicFiltering, jboolean cleanMetadataFlag, jint fontMaginfierLevel, jstring workFolder, jstring resourceFolder,  jstring languageCode, jfloat scaleFactor,
                          jobject frameLayout, jobject nativeContent, jobject nativeMapController, jobject mapController, jobject mapToolbar, jstring productClass)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context)
    {
        AndroidNativeUIListener* uiObject = new AndroidNativeUIListener(env->NewGlobalRef(thiz));
        context->androidObject = uiObject;
        if (context->jniMapListener)
        {
            context->jniMapListener->SetAndroidNativeUIListener(uiObject);
        }
        context->scaleFactor = scaleFactor;

        MapViewConfiguration config;
        nsl_memset(&config, 0, sizeof(MapViewConfiguration));
        config.defaultLatitude = latitude;
        config.defaultLongitude = longitude;
        config.defaultTiltAngle = tilt;
        config.maximumCachingTileCount = maximumCachingTileCount;
        config.maximumTileRequestCountPerLayer = maximumTileRequestCountPerLayer;
        config.workFolder = NewStdString(env, workFolder);
        config.resourceFolder = NewStdString(env, resourceFolder);
        config.zorderLevel = zorderLevel;
        config.languageCode = NewStdString(env, languageCode);
        config.productClass = NewStdString(env, productClass);
        config.enableFullScreenAntiAliasing = enableFullScreenAntiAliasing;
        config.enableAnisotropicFiltering = enableAnisotropicFiltering;
        config.fontMaginfierLevel = static_cast<MapViewFontMagnifierLevel>(fontMaginfierLevel);
        config.cleanMetadataFlag = cleanMetadataFlag;
        config.scaleFactor = scaleFactor;

        MapSurfaceViewContext* surfaceViewContext = new MapSurfaceViewContext();
        surfaceViewContext->frameLayoutObject = env->NewGlobalRef(frameLayout);
        surfaceViewContext->nativeContentObject = env->NewGlobalRef(nativeContent);
        surfaceViewContext->nativeMapControllerObject = env->NewGlobalRef(nativeMapController);
        surfaceViewContext->mapControllerObject = env->NewGlobalRef(mapController);
        surfaceViewContext->mapToolbarObject = env->NewGlobalRef(mapToolbar);
        surfaceViewContext->renderingConfig.enableFullScreenAntiAliasing = config.enableFullScreenAntiAliasing;
        surfaceViewContext->renderingConfig.enableAnisotropicFiltering = config.enableAnisotropicFiltering;
        context->mapController->Create(config, surfaceViewContext);
    }
}

static void JNI_SetPosition(JNIEnv* env, jobject thiz, jdouble latitude, jdouble longitude)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->SetPosition(latitude, longitude);

    }
}

static jint JNI_AddPin(JNIEnv* env, jobject thiz, double lat, double lon,
                       jfloat unselectedImageWidth, jfloat unselectedImageHeight,
                       jfloat selectedImageWidth, jfloat selectedImageHeight,
                       jbyteArray selectedImage, jbyteArray unselectedImage,
                       jfloatArray selectedPinOffset, jfloatArray selectedBubbleOffset,
                       jfloatArray unselectedPinOffset, jfloatArray unselectedBubbleOffset,
                       jint radiusTotalWidth, jint radiusBorderWidth,
                       jint radiusFillColor, jint radiusBorderColor,
                       jobject bubbleObj)
{
    jint ret = 0;
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        jsize count = 0;
        jbyte* selectedImagebufferData = NULL;
        jbyte* unselectedImageBufferData = NULL;
        jfloat* selectedPinOffsetData = NULL;
        jfloat* selectedBubbleOffsetData = NULL;
        jfloat* unselectedPinOffsetData = NULL;
        jfloat* unselectedBubbleOffsetData = NULL;
        PinType type = PT_COMMON;

        //selected image data
        bool selectedImageValid = false;
        nbcommon::DataStreamPtr selectedImageData;
        if(selectedImage)
        {
            count = env->GetArrayLength(selectedImage);
            selectedImagebufferData = env->GetByteArrayElements(selectedImage, NULL);
            if(selectedImagebufferData && count > 0)
            {
                selectedImageData.reset(new nbcommon::DataStreamImplementation());
                if(selectedImageData)
                {
                    selectedImageData->AppendData((const uint8*)(selectedImagebufferData),
                                                  (uint32)(count));
                    selectedImageValid = true;
                }
            }
        }
        //unselected image data
        bool unselectedImageValid = false;
        nbcommon::DataStreamPtr unselectedImageData;
        if(unselectedImage)
        {
            count = env->GetArrayLength(unselectedImage);
            unselectedImageBufferData = env->GetByteArrayElements(unselectedImage, NULL);
            if(unselectedImageBufferData && count > 0)
            {
                unselectedImageData.reset(new nbcommon::DataStreamImplementation());
                if(unselectedImageData)
                {
                    unselectedImageData->AppendData((const uint8*)(unselectedImageBufferData),
                                                    (uint32)(count));
                    unselectedImageValid = true;
                }
            }
        }

        int8 calloutOffsetX = 0;
        int8 calloutOffsetY = 0;
        if(selectedPinOffset) {
            count = env->GetArrayLength(selectedPinOffset);
            selectedPinOffsetData = env->GetFloatArrayElements(selectedPinOffset, NULL);
            if(count >= 2 && selectedPinOffsetData)
            {
                calloutOffsetX = (int8)(selectedPinOffsetData[0]*100);
                calloutOffsetY = (int8)(selectedPinOffsetData[1]*100);
            }
        }

        int8 bubbleOffsetX = 0;
        int8 bubbleOffsetY = 0;
        if(selectedBubbleOffset) {
            count = env->GetArrayLength(selectedBubbleOffset);
            selectedBubbleOffsetData = env->GetFloatArrayElements(selectedBubbleOffset, NULL);
            if(count >= 2 && selectedBubbleOffsetData)
            {
                bubbleOffsetX = (int8)(selectedBubbleOffsetData[0]*100);
                bubbleOffsetY = (int8)(selectedBubbleOffsetData[1]*100);
            }
        }

        int8 unslectedCalloutOffsetX = 0;
        int8 unslectedCalloutOffsetY = 0;
        if(unselectedPinOffset) {
            count = env->GetArrayLength(unselectedPinOffset);
            unselectedPinOffsetData = env->GetFloatArrayElements(unselectedPinOffset, NULL);
            if(count >= 2 && unselectedPinOffsetData)
            {
                unslectedCalloutOffsetX = (int8)(unselectedPinOffsetData[0]*100);
                unslectedCalloutOffsetY = (int8)(unselectedPinOffsetData[1]*100);
            }
        }

        int8 unslectedBubbleOffsetX = 0;
        int8 unslectedBubbleOffsetY = 0;
        if(unselectedBubbleOffset) {
            count = env->GetArrayLength(unselectedBubbleOffset);
            unselectedBubbleOffsetData = env->GetFloatArrayElements(unselectedBubbleOffset, NULL);
            if(count >= 2 && unselectedBubbleOffsetData)
            {
                unslectedBubbleOffsetX = (int8)(unselectedBubbleOffsetData[0]*100);
                unslectedBubbleOffsetY = (int8)(unselectedBubbleOffsetData[1]*100);
            }
        }

        CustomPinInformationPtr customPinInformation(new CustomPinInformation(unslectedCalloutOffsetX,
                                                                              unslectedCalloutOffsetY,
                                                                              unslectedBubbleOffsetX,
                                                                              unslectedBubbleOffsetY,
                                                                              calloutOffsetX,
                                                                              calloutOffsetY,
                                                                              bubbleOffsetX,
                                                                              bubbleOffsetY,
                                                                              unselectedImageData,
                                                                              selectedImageData,
                                                                              (float)unselectedImageWidth,
                                                                              (float)unselectedImageHeight,
                                                                              (float)selectedImageWidth,
                                                                              (float)selectedImageHeight,
                                                                              (uint32)radiusFillColor,
                                                                              (uint32)radiusBorderColor,
                                                                              (uint32)radiusFillColor,
                                                                              (uint32)radiusBorderColor,
                                                                              (uint8)radiusBorderWidth));

        shared_ptr<nbmap::BubbleInterface> bubble;
        if(bubbleObj != NULL)
        {
            jobject bubbleGRef = (jobject) env->NewGlobalRef(bubbleObj);
            env->DeleteLocalRef(bubbleObj);
            if(bubbleGRef)
            {
                bubble.reset(new BubbleAndroid(bubbleGRef));
            }
        }
        // TODO:
        // There is no way for users to set radius color only.
        // Because CustomPinInformation is valid if selected image or unselcted image is supplied.
        // Now, Pin will be allowed to custom only if selectedImageValid and unselectedImageValid.
        if(selectedImageValid && unselectedImageValid)
        {
            type = PT_CUSTOM;
        }
        nbmap::PinParameters<nbmap::BubbleInterface> para(type,
                                                          bubble,
                                                          customPinInformation,
                                                          lat,
                                                          lon,
                                                          (uint16)radiusTotalWidth);
        ret = reinterpret_cast<int>(context->mapController->AddPin(para));
        if(selectedImagebufferData)
        {
            env->ReleaseByteArrayElements(selectedImage, selectedImagebufferData, JNI_ABORT);
        }
        if(unselectedImageBufferData)
        {
            env->ReleaseByteArrayElements(unselectedImage, unselectedImageBufferData, JNI_ABORT);
        }
        if(selectedPinOffsetData)
        {
            env->ReleaseFloatArrayElements(selectedPinOffset, selectedPinOffsetData, JNI_ABORT);
        }
        if(selectedBubbleOffsetData)
        {
            env->ReleaseFloatArrayElements(selectedBubbleOffset, selectedBubbleOffsetData, JNI_ABORT);
        }
        if(unselectedPinOffsetData)
        {
            env->ReleaseFloatArrayElements(unselectedPinOffset, unselectedPinOffsetData, JNI_ABORT);
        }
        if(unselectedBubbleOffsetData)
        {
            env->ReleaseFloatArrayElements(unselectedBubbleOffset, unselectedBubbleOffsetData, JNI_ABORT);
        }
    }
    return ret;
}

static void JNI_RemoveAllPins(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->RemoveAllPins();
    }
}

static void JNI_ShowTraffic(JNIEnv* env, jobject thiz, jboolean enable)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->ShowTraffic(enable == TRUE);

    }
}

static void JNI_EnableDebugView(JNIEnv* env, jobject thiz, jboolean enable)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->EnableDebugView(enable == TRUE);
    }
}

static void JNI_SetAvatarLocation(JNIEnv* env, jobject thiz, jobject locationObj)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        NB_GpsLocation gpslocation;
        nsl_memset(&gpslocation, 0, sizeof(gpslocation));

        if(GetNativeNBGpsLocation(env, locationObj, gpslocation))
        {
            context->mapController->SetAvatarLocation(gpslocation);
        }
    }
}

static void JNI_SetAvatarMode(JNIEnv* env, jobject thiz, jint mode)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->SetAvatarMode(static_cast<MapViewAvatarMode>(mode));
    }
}

static void JNI_SetAvatarScale(JNIEnv* env, jobject thiz, jfloat scale)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->SetAvatarScale(scale);
    }
}

static void JNI_SetGpsMode(JNIEnv* env, jobject thiz, jint mode)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->SetGpsMode(static_cast<NB_GpsMode>(mode));
    }
}

// @todo: Check on which thread this function is called.
static void JNI_ShowDoppler(JNIEnv* env, jobject thiz, jboolean enable)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context)
    {
        //@note: Layer name "Weather" may change according to language setting which is sent to server,
        //       App should get all optional layers, and then expose the names of layer to user,
        //       and then retrieve selected layer, and pass it back. Since this is a
        EnableOptionalLayer(context, "Weather", enable);
    }

}

static void JNI_SetCamera(JNIEnv* env, jobject thiz, jdouble latitude, jdouble longitude, jfloat zoomLevel, jfloat heading, jfloat tilt, jboolean animated, jint duration, jint accelerationtype, jint id)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        CameraAnimationAccelerationType type = CAAT_LINEAR;
        switch(accelerationtype)
        {
        case 0:
            type = CAAT_LINEAR;
            break;
        case 1:
            type = CAAT_DECELERATION;
            break;
        case 2:
            type = CAAT_ACCELERATION;
            break;
        default:
            break;
        }
        context->mapController->SetCamera(latitude, longitude, zoomLevel, heading, tilt, animated, duration, type, id);
    }
}

static void JNI_SetAnimationLayerOpacity(JNIEnv* env, jobject thiz, jint opacity)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->SetAnimationLayerOpacity(opacity);
    }
}

static void JNI_ConfigureMetadata(JNIEnv* env, jobject thiz, jint metaDataType, jboolean enable)
{
    MapContext *context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->ConfigureMetadata(metaDataType, (enable == TRUE));
    }
}
static void JNI_SetNightMode(JNIEnv* env, jobject thiz, jint mode)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->SetNightMode(mode);
    }
}

static jboolean JNI_IsNightMode(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        return context->mapController->IsNightMode();
    }
    return FALSE;
}

static GeoPolylineCapPtr GetPolylineCap(JNIEnv* env, jintArray capData)
{
    nsl_assert(env && capData);
    GeoPolylineCapPtr ret;
    jsize elementCount = env->GetArrayLength (capData);
    if(elementCount > 0)
    {
        jint* data = env->GetIntArrayElements(capData, NULL);
        if(data == NULL)
        {
            return ret;
        }
        int capType = data[0];
        switch(capType)
        {
        case PCT_Arrow:
            {
                // the content of PCT_Arrow array should be organized into type,width,lenght
                if(elementCount >= 3)
                {
                    uint32 width = static_cast<uint32>(data[1]);
                    uint32 length = static_cast<uint32>(data[2]);
                    ret = GeoPolylineCap::CreateArrowCap(width, length);
                }
            }
            break;
        case PCT_Round:
            {
                // the content of PCT_Round array should be organized into type,radius
                if(elementCount >= 2)
                {
                    uint32 radius = static_cast<uint32>(data[1]);
                    ret = GeoPolylineCap::CreateRoundCap(radius);
                }
            }
            break;
        default:
            break;
        }
        env->ReleaseIntArrayElements(capData, data, JNI_ABORT);
    }
    return ret;
}

static jint JNI_AddPolyline(JNIEnv* env, jobject thiz,
        jfloat width, jint zOrder, jdoubleArray points,
        jintArray segmentAttributeTypes,
        jintArray endPointIndexs,
        jintArray colors,
        jfloatArray distances,
        jobjectArray bitmaps,
        jintArray bitmapSizes,
        int unselectColor, jintArray startCap, jintArray endCap, jint outlineColor, jbyte outlineWidth)
{
    Geographic* g = NULL;
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        jsize ptCount = env->GetArrayLength (points) / 2;
        if (ptCount > 0)
        {
            jdouble* locationData = env->GetDoubleArrayElements(points, NULL);

            vector<GeoPoint>* vp = new vector<GeoPoint>();
            vector<GeoPolylineAttributeEx*>* va = new vector<GeoPolylineAttributeEx*>();
            GeoPoint p = {0};
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline ========================="));
            for (jsize i = 0; i < ptCount; ++i)
            {
                p.latitude = locationData[i * 2];
                p.longitude = locationData[i * 2 + 1];
                p.altitude = 0.0;
                vp->push_back(p);
                //DEBUGLOG("###Polyline jni, %f %f, ", i, p.latitude, p.longitude);
            }

            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline total count[%d]", vp->size()));

            jsize colorCount = colors ? env->GetArrayLength (colors) : 0;
            jint* pColor = colors ? env->GetIntArrayElements(colors, NULL) : NULL;
            jsize colorCounter = 0;

            jsize distanceCount = distances ? env->GetArrayLength (distances) : 0;
            jfloat* pDistance = distances ? env->GetFloatArrayElements(distances, NULL) : NULL;
            jsize distanceCounter = 0;
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline distance Count[%d]", distanceCount));

            jsize bitmapCount = bitmaps ? env->GetArrayLength (bitmaps) : 0;
            jsize bitmapCounter = 0;
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline bitmap Count[%d]", bitmapCount));

            jsize bitmapSizeCount = bitmapSizes ? (env->GetArrayLength (bitmapSizes) / 2) : 0;
            jint* pBitmapSize = bitmapSizes ? env->GetIntArrayElements(bitmapSizes, NULL) : NULL;
            jsize bitmapSizeCounter = 0;
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline bitmap Size Count[%d]", bitmapSizeCount * 2));

            jsize endPointIndexCount = endPointIndexs ? env->GetArrayLength (endPointIndexs) : 0;
            jint* pEndPointIndex = endPointIndexs ? env->GetIntArrayElements(endPointIndexs, NULL) : NULL;
            jsize endPointIndexCounter = 0;
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline end Point Index Count[%d]", endPointIndexCount));

            jsize segmentAttributeTypeCount = env->GetArrayLength (segmentAttributeTypes);
            jint* segmentAttributeType = env->GetIntArrayElements(segmentAttributeTypes, NULL);
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline segment Attribute Type Count[%d]", segmentAttributeTypeCount));

            for (jsize i = 0; i < segmentAttributeTypeCount; ++i)
            {
                int endPointIndex = 0;
                if(endPointIndexCounter < endPointIndexCount)
                {
                    endPointIndex =  pEndPointIndex[endPointIndexCounter];
                    endPointIndexCounter++;
                }

                switch (segmentAttributeType[i]) {
                    case PT_SOLID_LINE://color attribute
                    {
                        int color = 0;
                        if(colorCounter < colorCount)
                        {
                            color = pColor[colorCounter];
                            colorCounter++;
                            va->push_back(new GeoPolylineColorAttribute(color, endPointIndex));
                            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline segmentAttributeType[%d]=%s, color[%d], endPointIndex[%d]", i, "PT_SOLID_LINE", color, endPointIndex));

                        }
                        break;
                    }

                    case PT_DASH_LINE://pattern attribute
                    {
                        float distance = 0;
                        if(distanceCounter < distanceCount)
                        {
                            distance = pDistance[distanceCounter];
                            distanceCounter++;
                        }

                        jbyteArray bitmap = NULL;
                        jbyte* textureBufData = NULL;
                        uint32 textureBufLen = 0;
                        if(bitmapCounter < bitmapCount)
                        {
                            bitmap = (jbyteArray)env->GetObjectArrayElement(bitmaps, bitmapCounter);
                            textureBufData = env->GetByteArrayElements(bitmap, NULL);
                            textureBufLen = env->GetArrayLength(bitmap);
                            bitmapCounter++;
                        }

                        int bitmapWidth = 0;
                        int bitmapHeight = 0;
                        if(bitmapSizeCounter < bitmapSizeCount)
                        {
                            bitmapWidth = pBitmapSize[bitmapSizeCounter * 2];
                            bitmapHeight = pBitmapSize[bitmapSizeCounter * 2 + 1];
                            bitmapSizeCounter++;
                            //GeoPolylinePatternAttribute(uint32 endIndex, uint32 distance, string textureName, char* textureBuf,
                                    //uint32 textureBufLen, int textureWidth, int textureHeight)
                            //TODO: 1. need think about the bitmap byte array.
                            //TODO: 2. Need think about the construct the GeoPolylineAttributeEx, add a type property in the bass class.
                            //TODO: 3. Need do deep copy for bmp texture. also need release the byte array in jni side.
                            nbcommon::DataStreamPtr textureData;
                            textureData.reset(new nbcommon::DataStreamImplementation());
                            if(textureData)
                            {
                                textureData->AppendData((const uint8*)(textureBufData), textureBufLen);
                            }
                            va->push_back(new GeoPolylinePatternAttribute(endPointIndex, distance, "textureName", textureData, bitmapWidth, bitmapHeight));
                            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline segmentAttributeType[%d]=%s, distance[%f], endPointIndex[%d], textureDataSize[%d], bitmapWidth[%d], bitmapHeight[%d]", i, "PT_DASH_LINE", distance, endPointIndex, textureBufLen, bitmapWidth, bitmapHeight));
                        }

                        break;
                    }
                    default:
                        break;
                }
            }

            //Make the polyline ASC sort base on the 'endIndex'
            std::sort(va->begin(), va->end(), GeoPolylineAttributeEx::sort);

            // start polyline cap
            GeoPolylineCapPtr startCapPtr;
            if(startCap)
            {
                startCapPtr = GetPolylineCap(env, startCap);
                DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline Has startCap"));
            }
            // end polyline cap
            GeoPolylineCapPtr endCapPtr;
            if(endCap)
            {
                endCapPtr = GetPolylineCap(env, endCap);
                DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline Has endCap"));
            }

            g = context->mapController->AddPolyline(GeoPolylineOption(
                shared_ptr<const vector<GeoPoint> >(vp),
                shared_ptr<const vector<GeoPolylineAttributeEx*> >(va),
                width, unselectColor, zOrder, outlineColor, (uint8)outlineWidth, startCapPtr, endCapPtr)
                );
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("###Polyline route width[%f], unselectColor[%d], zOrder[%d]", width, unselectColor, zOrder));
            env->ReleaseDoubleArrayElements(points, locationData, JNI_ABORT);
            if(pColor)
            {
                env->ReleaseIntArrayElements(colors, pColor, JNI_ABORT);
            }

            if(pDistance)
            {
                env->ReleaseFloatArrayElements(distances, pDistance, JNI_ABORT);
            }

            if(pBitmapSize)
            {
                env->ReleaseIntArrayElements(bitmapSizes, pBitmapSize, JNI_ABORT);
            }

            if(pEndPointIndex)
            {
                env->ReleaseIntArrayElements(endPointIndexs, pEndPointIndex, JNI_ABORT);
            }

            if(segmentAttributeType)
            {
                env->ReleaseIntArrayElements(segmentAttributeTypes, segmentAttributeType, JNI_ABORT);
            }
        }
    }
    return reinterpret_cast<jint>(g);
}

static void JNI_RemoveGeoGraphics(JNIEnv* env, jobject thiz, jintArray ids)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        jsize idCount = env->GetArrayLength(ids);
        if (idCount > 0)
        {
            jint* idData = env->GetIntArrayElements(ids, NULL);
            vector<Geographic*> idList;

            for (jsize i = 0; i < idCount; ++i)
            {
                Geographic* g = reinterpret_cast<Geographic*>(idData[i]);
                if (g)
                {
                    idList.push_back(g);
                }
            }

            context->mapController->RemoveGeographic(idList);
            env->ReleaseIntArrayElements(ids, idData, JNI_ABORT);
        }
    }
}

static void JNI_GeographicsSetSelected(JNIEnv* env, jobject thiz, jint id, jboolean selected)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if (context && context->mapController)
    {
        //Geographic* g = reinterpret_cast<Geographic*>(id);
        GeoPolyline* g = reinterpret_cast<GeoPolyline*>(id);
        if (g)
        {
            context->mapController->SetPolylineSelected(g, (selected ? true : false));
        }
    }
}

static void JNI_PolylineSetWidth(JNIEnv* env, jobject thiz, jint id, jfloat width)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if (context && context->mapController)
    {
        GeoPolyline* g = reinterpret_cast<GeoPolyline*>(id);
        if (g)
        {
            g->SetWidth(static_cast<uint32>(width));
        }
    }
}

static void JNI_PolylineSetColors(JNIEnv* env, jobject thiz, jint id, jintArray colors)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
//        GeoPolyline* g = reinterpret_cast<GeoPolyline*>(id);
//        if (g)
//        {
//            jsize colorCount = env->GetArrayLength (colors) / 2;
//
//            if (colorCount > 0)
//            {
//                jint* colorData = env->GetIntArrayElements(colors, NULL);
//                vector<GeoPolylineAttribute>* va = new vector<GeoPolylineAttribute>();
//                for (jsize i = 0; i < colorCount; ++i)
//                {
//                    va->push_back(GeoPolylineAttribute(colorData[i * 2], colorData[i * 2 + 1]));
//                }
//                g->SetColors(shared_ptr<const vector<GeoPolylineAttribute> >(va));
//                env->ReleaseIntArrayElements(colors, colorData, JNI_ABORT);
//            }
//        }
    }
}

static void JNI_PolylineSetZOrder(JNIEnv* env, jobject thiz, jint id, jint zOrder)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        GeoPolyline* g = reinterpret_cast<GeoPolyline*>(id);
        if (g)
        {
            context->mapController->SetPolylineZorder(g, zOrder);
           //g->SetDrawOrder(zOrder);
        }
    }
}

static void JNI_PolylineSetVisible(JNIEnv* env, jobject thiz, jint id, jboolean visible)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        GeoPolyline* g = reinterpret_cast<GeoPolyline*>(id);
        if (g)
        {
            context->mapController->SetPolylineVisible(g, (visible ? true : false));
            //g->Show(visible);
        }
    }
}

static void JNI_GeographicsEnableSelectedListner(JNIEnv* env, jobject thiz, jint id, jboolean enable)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if (context && context->mapController && context->androidObject)
    {
        Geographic* g = reinterpret_cast<Geographic*>(id);
        if (g)
        {
            if (enable)
            {
                JniGeographicSelectedListener* l = new JniGeographicSelectedListener(*context, g);
                g->SetSelectedListner(l);
                context->graphicListeners[g] = shared_ptr<JniGeographicSelectedListener>(l);
            }
            else
            {
                g->SetSelectedListner(NULL);
                shared_ptr<JniGeographicSelectedListener> nullPtr;
                context->graphicListeners[g] = nullPtr;
            }
        }
    }
}

static jintArray JNI_GeographicsGetSelectedElements(JNIEnv* env, jobject thiz)
{
    jintArray ids = NULL;
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if (context && context->mapController)
    {
        std::vector<nbmap::Geographic*> graphics;
        context->mapController->GetSelectedElements(graphics);

        jsize idCount = graphics.size();
        ids = env->NewIntArray(idCount);
        jint* buf = new jint[idCount];
        int i = 0;
        for (std::vector<Geographic*>::iterator it = graphics.begin(); it != graphics.end(); ++it)
        {
            buf[i++] = reinterpret_cast<jint>(*it);
        }
        env->SetIntArrayRegion(ids, 0, idCount, buf);
        delete[] buf;
    }
    return ids;
}

static void JNI_EnableCustomLayerCollisionDetection(JNIEnv* env, jobject thiz, jint layerId, jboolean enable)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && layerId != 0)
    {
        MapLayer *layer = reinterpret_cast<MapLayer*>(layerId);
        context->mapController->EnableCustomLayerCollisionDetection(layer, enable);
    }
}

bool
GetNativePolylineParameters(JNIEnv* env, jobject polylineParameters, GeoPolylineOption &option)
{
    if(env == NULL || polylineParameters == NULL)
    {
        return false;
    }


    return true;
}

struct fields_polylineParameters
{
    jmethodID getWidth;
    jmethodID getColor;
    jmethodID getZOrder;
    jmethodID isVisible;
    jmethodID getPoints;
};
static fields_polylineParameters polylineParametersFields;

static void JNI_SetCompassPosition(JNIEnv* env, jobject thiz, jfloat screenX, jfloat screenY)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        context->mapController->SetCompassPosition(screenX, screenY);
    }
}

static void JNI_SetCompassEnabled(JNIEnv* env, jobject thiz, jboolean enable)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        context->mapController->ShowCompass(enable);
    }
}

static void JNI_SetCompassIcons(JNIEnv* env, jobject thiz, jstring dayModeIconPath, jstring nightModeIconPath)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        std::string dmName = NewStdString(env, dayModeIconPath);
        std::string nmName = NewStdString(env, nightModeIconPath);
        context->mapController->SetCompassIcons(dmName, nmName);
    }
}

static void JNI_GetCompassBoundingBox(JNIEnv* env, jobject thiz, jfloatArray box)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        float boundingBox[4] = {0};
        context->mapController->GetCompassBoundingBox(boundingBox[0], boundingBox[1], boundingBox[2], boundingBox[3]);
        env->SetFloatArrayRegion(box, 0, 4, boundingBox);
    }
}

static void JNI_SetNavigationModeEnabled(JNIEnv* env, jobject thiz, jboolean enable)
{

}

static void JNI_SetPinBubbleResolver(JNIEnv* env, jobject thiz, jobject bubbleResolver)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context->mapController)
    {
        context->mapController->SetPinBubbleResolver(shared_ptr<CustomPinBubbleResolverAndroid>(new CustomPinBubbleResolverAndroid(bubbleResolver)));
    }
}

static void JNI_CreateCameraForNav(JNIEnv* env, jobject thiz, jdouble lat, jdouble lon, jdouble heading, jint width, jint height,
        jfloat cameraHeight, jfloat cameraToAvatar, jfloat avatarToHorizon,
        jfloat hFov, jfloat avatarToBottom, jfloat avatarScale,
        jdoubleArray _lat, jdoubleArray _lon, jfloatArray _zoomLevel, jfloatArray _heading, jfloatArray _tilt)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        CameraSetting cameraSetting = {0};
        cameraSetting.avatarScale = static_cast<float>(avatarScale);
        cameraSetting.avatarToBottom = static_cast<float>(avatarToBottom);
        cameraSetting.avatarToHorizon = static_cast<float>(avatarToHorizon);
        cameraSetting.cameraHeight = static_cast<float>(cameraHeight);
        cameraSetting.cameraToAvatar = static_cast<float>(cameraToAvatar);
        cameraSetting.hFov = static_cast<float>(hFov);

        Camera camera;
        CameraHelper::CreateCamera(static_cast<double>(lon), static_cast<double>(lat), static_cast<float>(heading),
                static_cast<int>(width), static_cast<int>(height),
                cameraSetting, camera, context->scaleFactor);

        double centerLat = 0;
        double centerLon = 0;
        camera.GetViewCenter(centerLat, centerLon);
        float zoomLevel= camera.GetZoomLevel();
        float heading = camera.GetRotateAngle();
        float tilt = camera.GetTiltAngle();

        env->SetDoubleArrayRegion(_lat, 0, 1, &centerLat);
        env->SetDoubleArrayRegion(_lon, 0, 1, &centerLon);
        env->SetFloatArrayRegion(_zoomLevel, 0, 1, &zoomLevel);
        env->SetFloatArrayRegion(_heading, 0, 1, &heading);
        env->SetFloatArrayRegion(_tilt, 0, 1, &tilt);
    }
}

static void JNI_CreateCamera(JNIEnv* env, jobject thiz, jint width, jint height, jfloat hFov,
        jdouble topLeftLatitude, jdouble topLeftLongitude, jdouble bottomRightLatitude, jdouble bottomRightLongitude,
        jdoubleArray _lat, jdoubleArray _lon, jfloatArray _zoomLevel, jfloatArray _heading, jfloatArray _tilt)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        LatLngBound latlngBound = {0};
        latlngBound.topLeftLatitude = static_cast<double>(topLeftLatitude);
        latlngBound.topLeftLongitude = static_cast<double>(topLeftLongitude);
        latlngBound.bottomRightLatitude = static_cast<double>(bottomRightLatitude);
        latlngBound.bottomRightLongitude = static_cast<double>(bottomRightLongitude);

        Camera camera;

        CameraHelper::CreateCamera(static_cast<int>(width), static_cast<int>(height), static_cast<float>(hFov),
                latlngBound, camera, context->scaleFactor);

        double centerLat = 0;
        double centerLon = 0;
        camera.GetViewCenter(centerLat, centerLon);
        float zoomLevel= camera.GetZoomLevel();
        float heading = camera.GetRotateAngle();
        float tilt = camera.GetTiltAngle();

        env->SetDoubleArrayRegion(_lat, 0, 1, &centerLat);
        env->SetDoubleArrayRegion(_lon, 0, 1, &centerLon);
        env->SetFloatArrayRegion(_zoomLevel, 0, 1, &zoomLevel);
        env->SetFloatArrayRegion(_heading, 0, 1, &heading);
        env->SetFloatArrayRegion(_tilt, 0, 1, &tilt);
    }
}

static jboolean JNI_ToScreenPosition(JNIEnv* env, jobject thiz, jdouble latitude, jdouble longitude, jintArray _x, jintArray _y)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController && context->mapController->GetDefaultProjection())
    {
        double lat = static_cast<double>(latitude);
        double lon = static_cast<double>(longitude);
        int x = 0;
        int y = 0;
        if(context->mapController->GetDefaultProjection()->ToScreenPosition(lat, lon, &x, &y))
        {
            env->SetIntArrayRegion(_x, 0, 1, &x);
            env->SetIntArrayRegion(_y, 0, 1, &y);
            return JNI_TRUE;
        }
    }
    return JNI_FALSE;
}

static jboolean JNI_FromScreenPosition(JNIEnv* env, jobject thiz, jint x, jint y, jdoubleArray _latitude, jdoubleArray _longitude)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController && context->mapController->GetDefaultProjection())
    {
        int screenX = static_cast<int>(x);
        int screenY = static_cast<int>(y);
        double lat = 0.0;
        double lon = 0.0;
        if(context->mapController->GetDefaultProjection()->FromScreenPosition(screenX, screenY, &lat, &lon))
        {
            env->SetDoubleArrayRegion(_latitude, 0, 1, &lat);
            env->SetDoubleArrayRegion(_longitude, 0, 1, &lon);
            return JNI_TRUE;
        }
    }
    return JNI_FALSE;
}

static jboolean JNI_GetCamera(JNIEnv* env, jobject thiz, jdoubleArray _latitude, jdoubleArray _longitude, jfloatArray _zoomLevel, jfloatArray _heading, jfloatArray _tilt)
{
    jboolean ret = JNI_FALSE;
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        double lat = 0.0;
        double lon = 0.0;
        float zoomLevel = 0.0;
        float heading = 0.f;
        float tilt = 0.f;

        if(context->mapController->GetCamera(lat, lon, zoomLevel, heading, tilt))
        {
            env->SetDoubleArrayRegion(_latitude, 0, 1, &lat);
            env->SetDoubleArrayRegion(_longitude, 0, 1, &lon);
            env->SetFloatArrayRegion(_zoomLevel, 0, 1, &zoomLevel);
            env->SetFloatArrayRegion(_heading, 0, 1, &heading);
            env->SetFloatArrayRegion(_tilt, 0, 1, &tilt);
            ret = JNI_TRUE;
        }
    }
    return ret;
}

static void JNI_RemovePin(JNIEnv* env, jobject thiz, jint pinId)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->RemovePin(reinterpret_cast<MapPin*>(pinId));
    }
}

static void JNI_UpdatePinPosition(JNIEnv* env, jobject thiz, jint pinId, double lat, double lon)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->UpdatePinPosition(reinterpret_cast<MapPin*>(pinId), lat, lon);
    }
}

static void JNI_SelectPin(JNIEnv* env, jobject thiz, jint pinId, jboolean selected)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->SelectPin(reinterpret_cast<MapPin*>(pinId), selected);
    }
}

static jdouble JNI_MetersPerPixel(JNIEnv* env, jobject thiz)
{
    jdouble ret = 0;
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        ret = context->mapController->GetDefaultProjection()->MetersPerPixel();
    }
    return ret;
}

static void JNI_ScrollCamera(JNIEnv* env, jobject thiz, jdouble lat, jdouble lon, jfloat zoomLevel, jfloat heading,
        jfloat tilt, jint width, jint height, jint xPixel, jint yPixel,
        jdoubleArray _lat, jdoubleArray _lon, jfloatArray _zoomLevel, jfloatArray _heading, jfloatArray _tilt)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        Camera camera;
        Camera result;

        camera.SetViewCenter(lat, lon);
        camera.SetZoomLevel(zoomLevel);
        camera.SetRotateAngle(heading);
        camera.SetTiltAngle(tilt);

        CameraHelper::ScrollCamera(camera, width, height, xPixel, yPixel, result, context->scaleFactor);

        double centerLat = 0;
        double centerLon = 0;
        result.GetViewCenter(centerLat, centerLon);
        float resultZoomLevel = result.GetZoomLevel();
        float resultHeading = result.GetRotateAngle();
        float resultTilt = result.GetTiltAngle();

        env->SetDoubleArrayRegion(_lat, 0, 1, &centerLat);
        env->SetDoubleArrayRegion(_lon, 0, 1, &centerLon);
        env->SetFloatArrayRegion(_zoomLevel, 0, 1, &resultZoomLevel);
        env->SetFloatArrayRegion(_heading, 0, 1, &resultHeading);
        env->SetFloatArrayRegion(_tilt, 0, 1, &resultTilt);
    }
}

static void JNI_ShowOptionalLayer(JNIEnv* env, jobject thiz, jbyteArray layerName, jboolean enable)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context)
    {
        jbyte* text_input = env->GetByteArrayElements(layerName, NULL);
        size_t size = (size_t)(env->GetArrayLength(layerName));
        char* buffer = (char*)nsl_malloc(size + 1);
        nsl_memset(buffer, 0, size+1);
        nsl_memcpy(buffer, text_input, size);
        std::string name(buffer);
        nsl_free(buffer);
        EnableOptionalLayer(context, name.c_str(), enable);
    }
}

static void JNI_BeginAtomicUpdate(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->BeginAtomicUpdate();
    }
}

static void JNI_EndAtomicUpdate(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context->mapController)
    {
        context->mapController->EndAtomicUpdate();
    }
}

static void JNI_PrefetchForNav(JNIEnv* env, jobject thiz,
        jdoubleArray polylineDatas,
        jdouble prefetchExtensionLengthMeters, jdouble prefetchExtensionWidthMeters, jfloat zoomLevel)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        //The client already guarantee that (array length % 2 == 0)
        jsize polylineCount = env->GetArrayLength (polylineDatas) / 2;
        if (polylineCount > 0)
        {
            shared_ptr<vector<pair<double, double> > > vecPolylineListPtr(new vector<pair<double, double> >());
            if(!vecPolylineListPtr)
            {
                return;
            }
            jdouble* polylineList = env->GetDoubleArrayElements(polylineDatas, NULL);
            double lat = 0;
            double lon = 0;
            for(jsize i = 0; i<polylineCount; i++)
            {
                lat = polylineList[i * 2];
                lon = polylineList[i * 2 + 1];
                vecPolylineListPtr->push_back(make_pair(lat, lon));
            }
            context->mapController->Prefetch(vecPolylineListPtr,
                    prefetchExtensionLengthMeters,
                    prefetchExtensionWidthMeters, zoomLevel);
        }
    }
}

static void JNI_Prefetch(JNIEnv* env, jobject thiz,
        jdouble lat, jdouble lon, jfloat zoomLevel, jfloat heading, jfloat tilt)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        context->mapController->Prefetch(static_cast<double>(lat),
                static_cast<double>(lon), static_cast<float>(zoomLevel),
                static_cast<float>(heading), static_cast<float>(tilt));
    }
}

static void JNI_StopAnimations(JNIEnv* env, jobject thiz, jint animationType)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));

    if(context && context->mapController)
    {
        context->mapController->StopAnimations(animationType);
    }
}

static void JNI_SetBackground(JNIEnv* env, jobject thiz, jboolean background)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->SetBackground(background);
    }
}

static void JNI_PlayDoppler(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->PlayDoppler();
    }
}

static void JNI_PauseDoppler(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->PauseDoppler();
    }
}

static jint JNI_AddCircle(JNIEnv* env, jobject thiz, jdouble lat, jdouble lon, jfloat radius, jint fillClr, jint strokeClr, jint zOrder, jboolean visible)
{
    jint id = 0;
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        id = reinterpret_cast<jint>(context->mapController->AddCircle(lat, lon, radius, fillClr, strokeClr, zOrder, visible));
    }
    return id;
}

static void JNI_RemoveCircle(JNIEnv* env, jobject thiz, jint circleId)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapCircle *c = reinterpret_cast<MapCircle*>(circleId);
        if(c != NULL)
        {
            context->mapController->RemoveCircle(c);
        }
    }
}

static void JNI_RemoveAllCircles(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->RemoveAllCircles();
    }
}

static void JNI_SetCircleCenter(JNIEnv* env, jobject thiz, jint circleId, jdouble lat, jdouble lon)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapCircle *c = reinterpret_cast<MapCircle*>(circleId);
        if(c != NULL)
        {
            c->SetCenter(lat, lon);
        }
    }
}

static void JNI_SetCircleVisible(JNIEnv* env, jobject thiz, jint circleId, jboolean visible)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapCircle *c = reinterpret_cast<MapCircle*>(circleId);
        if(c != NULL)
        {
            c->SetVisible(visible);
        }
    }
}

static void JNI_SetCircleRadius(JNIEnv* env, jobject thiz, jint circleId, jfloat radius)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapCircle *c = reinterpret_cast<MapCircle*>(circleId);
        if(c != NULL)
        {
            c->SetRadius(radius);
        }
    }
}

static void JNI_SetCircleFillColor(JNIEnv* env, jobject thiz, jint circleId, jint fillClr)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapCircle *c = reinterpret_cast<MapCircle*>(circleId);
        if(c != NULL)
        {
            c->SetFillColor(fillClr);
        }
    }
}

static void JNI_SetCircleStrokeColor(JNIEnv* env, jobject thiz, jint circleId, jint strokeClr)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapCircle *c = reinterpret_cast<MapCircle*>(circleId);
        if(c != NULL)
        {
            c->SetStrokeColor(strokeClr);
        }
    }
}

static void JNI_SetReferenceCenter(JNIEnv* env, jobject thiz, jfloat x, jfloat y)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->SetReferenceCenter(x, y);
    }
}

static void JNI_EnableReferenceCenter(JNIEnv* env, jobject thiz, jboolean enable)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->EnableReferenceCenter(enable == TRUE);
    }
}

static jint JNI_AddRect2D(JNIEnv* env, jobject thiz, jdouble lat, jdouble lon, jfloat heading, jfloat width, jfloat height, jint texId, jbyteArray texData, jboolean visible)
{
    jint id = 0;
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        nbcommon::DataStreamPtr textureData;
        jbyte* texBuf = NULL;
        if(texData != NULL)
        {
            jsize count = env->GetArrayLength(texData);
            texBuf = env->GetByteArrayElements(texData, NULL);
            if(texBuf && count > 0)
            {
                textureData.reset(new nbcommon::DataStreamImplementation());
                if(textureData)
                {
                    textureData->AppendData((const uint8*)(texBuf), (uint32)(count));
                }
            }
        }
        id = reinterpret_cast<jint>(context->mapController->AddRect2d(lat, lon, heading, width, height, texId, textureData, visible));
        if(texData && texBuf)
        {
            env->ReleaseByteArrayElements(texData, texBuf, JNI_ABORT);
        }
    }
    return id;
}

static void JNI_RemoveRect2D(JNIEnv* env, jobject thiz, jint rectId)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapRect2d *rect = reinterpret_cast<MapRect2d*>(rectId);
        if(rect != NULL)
        {
            context->mapController->RemoveRect2d(rect);
        }
    }
}

static void JNI_SetRect2DVisible(JNIEnv* env, jobject thiz, jint rectId, jboolean visible)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapRect2d *rect = reinterpret_cast<MapRect2d*>(rectId);
        if(rect != NULL)
        {
             rect->SetVisible(visible);
        }
    }
}

static void JNI_SetRect2DSize(JNIEnv* env, jobject thiz, jint rectId, jfloat width, jfloat height)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapRect2d *rect = reinterpret_cast<MapRect2d*>(rectId);
        if(rect != NULL)
        {
             rect->SetSize(width, height);
        }
    }
}

static void JNI_UpdateRect2D(JNIEnv* env, jobject thiz, jint rectId, jdouble lat, jdouble lon, jfloat heading)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        MapRect2d *rect = reinterpret_cast<MapRect2d*>(rectId);
        if(rect != NULL)
        {
             rect->Update(lat, lon, heading);
        }
    }
}

static void JNI_RemoveAllRect2Ds(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->RemoveAllRect2ds();
    }
}

static void JNI_AddTexture(JNIEnv* env, jobject thiz, jint textureId, jbyteArray textureBuffer)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && textureId != 0 && textureBuffer != NULL)
    {
        jsize count = env->GetArrayLength(textureBuffer);
        jbyte* texData = env->GetByteArrayElements(textureBuffer, NULL);
        if(texData && count > 0)
        {
            nbcommon::DataStreamPtr textureData;
            textureData.reset(new nbcommon::DataStreamImplementation());
            if(textureData)
            {
                textureData->AppendData((const uint8*)(texData), (uint32)(count));
                context->mapController->AddTexture(textureId, textureData);
            }
            env->ReleaseByteArrayElements(textureBuffer, texData, JNI_ABORT);
        }
    }
}

static void JNI_RemoveTexture(JNIEnv* env, jobject thiz, jint textureId)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && textureId != 0)
    {
        context->mapController->RemoveTexture(textureId);
    }
}

static void JNI_RemoveAllTextures(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->RemoveAllTextures();
    }
}



static void JNI_MasterClear(JNIEnv* env, jobject thiz)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        context->mapController->MasterClear();
    }
}

static jint JNI_CreateTile(JNIEnv* env, jobject thiz, jint x, jint y, jint zoom, jint count,
        jintArray ids, jfloatArray margins, jfloatArray anchors, jdoubleArray positions, jobjectArray bitmaps, jintArray sizes)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(!context|| !(context->mapController) || count <= 0)
    {
        return 0;
    }

    if(env->GetArrayLength (ids) != count || env->GetArrayLength (margins) != count ||
       env->GetArrayLength (anchors) != count*2 || env->GetArrayLength (positions) != count*2 ||
       env->GetArrayLength (bitmaps) != count || env->GetArrayLength (sizes) != count*2)
    {
        return 0;
    }

    jint tileId = 0;
    jint* idDada = env->GetIntArrayElements(ids, NULL);
    jfloat* marginData = env->GetFloatArrayElements(margins, NULL);
    jfloat* anchorData = env->GetFloatArrayElements(anchors, NULL);
    jdouble* positionData = env->GetDoubleArrayElements(positions, NULL);
    jint* sizeData = env->GetIntArrayElements(sizes, NULL);
    jbyte* bitmapData = NULL;
    jsize byteCount = 0;

    vector<nbmap::PinParameters<nbmap::BubbleInterface> > parametersVector;
    shared_ptr<nbmap::BubbleInterface> bubble;
    for(int i = 0; i < count; ++i)
    {
        jbyteArray bitmap = (jbyteArray)env->GetObjectArrayElement(bitmaps, i);
        if(bitmap == NULL)
        {
            continue;
        }
        byteCount = env->GetArrayLength(bitmap);
        if(byteCount <= 0)
        {
            continue;
        }
        nbcommon::DataStreamPtr imageData(new nbcommon::DataStreamImplementation());
        if(!imageData)
        {
            continue;
        }
        bitmapData = env->GetByteArrayElements(bitmap, NULL);
        if(bitmapData == NULL)
        {
            continue;
        }
        imageData->AppendData((const uint8*)(bitmapData), (uint32)(byteCount));
        env->ReleaseByteArrayElements(bitmap, bitmapData, JNI_ABORT);
        env->DeleteLocalRef(bitmap);
        const int idx = i*2;
        int id = idDada[i];
        float margin = marginData[i];
        double lat = positionData[idx];
        double lon = positionData[idx+1];
        int8 anchorX = (int8)(anchorData[idx]*100);
        int8 anchorY = (int8)(anchorData[idx+1]*100);
        float width = (float)sizeData[idx];
        float height = (float)sizeData[idx+1];
        CustomPinInformationPtr customPinInformation(new CustomPinInformation(anchorX,
                                                                              anchorY,
                                                                              0,
                                                                              0,
                                                                              anchorX,
                                                                              anchorY,
                                                                              0,
                                                                              0,
                                                                              imageData,
                                                                              imageData,
                                                                              width,
                                                                              height,
                                                                              width,
                                                                              height,
                                                                              0x6721D826,
                                                                              0,
                                                                              0x6721D826,
                                                                              0,
                                                                              0,
                                                                              id,
                                                                              margin));
        if(customPinInformation)
        {
            parametersVector.push_back(PinParameters<BubbleInterface>(PT_CUSTOM,
                                            bubble,
                                            customPinInformation,
                                            lat,
                                            lon));
        }
    }

    if(!parametersVector.empty())
    {
        MapTile *tile = context->mapController->CreateCustomTile(x, y, zoom, parametersVector);
        tileId = reinterpret_cast<jint>(tile);
    }

    env->ReleaseIntArrayElements(ids, idDada, JNI_ABORT);
    env->ReleaseFloatArrayElements(margins, marginData, JNI_ABORT);
    env->ReleaseFloatArrayElements(anchors, anchorData, JNI_ABORT);
    env->ReleaseDoubleArrayElements(positions, positionData, JNI_ABORT);
    env->ReleaseIntArrayElements(sizes, sizeData, JNI_ABORT);
    return tileId;
}

static void JNI_DestroyTile(JNIEnv* env, jobject thiz, jint tileId)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && tileId != 0)
    {
        MapTile *tile = reinterpret_cast<MapTile*>(tileId);
        if(tile != NULL)
        {
            context->mapController->DestroyCustomTile(tile);
        }
    }
}

static jint JNI_AddLayer (JNIEnv* env, jobject thiz, jobject provider, jint mainOrder, jint subOrder, jint refZoom, jint minZoom, jint maxZoom, jboolean visible)
{
    jint layerId = 0;
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && provider)
    {
        TileProviderPtr providerAndroid(new TileProviderAndroid(provider, *(context->mapController)));
        if(providerAndroid)
        {
            MapLayer* layer = context->mapController->AddCustomLayer(providerAndroid, mainOrder, subOrder, refZoom, minZoom, maxZoom, visible);
            layerId = reinterpret_cast<jint>(layer);
        }
    }
    return layerId;
}

static void JNI_RemoveLayer (JNIEnv* env, jobject thiz, jint layerId)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && layerId != 0)
    {
        MapLayer *layer = reinterpret_cast<MapLayer*>(layerId);
        context->mapController->RemoveCustomLayer(layer);
    }
}

static void JNI_SetLayerVisible (JNIEnv* env, jobject thiz, jint layerId, jboolean visible)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && layerId != 0)
    {
        MapLayer *layer = reinterpret_cast<MapLayer*>(layerId);
        if(layer)
        {
            layer->SetVisible(visible);
        }
    }
}

static void JNI_InvalidTile (JNIEnv* env, jobject thiz, jint layerId, jint x, jint y, jint zoom)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && layerId != 0)
    {
        MapLayer *layer = reinterpret_cast<MapLayer*>(layerId);
        if(layer)
        {
            layer->Invalidate(x, y, zoom);
        }
    }
}

static void JNI_InvalidTilesByZoom (JNIEnv* env, jobject thiz, jint layerId, jint zoom)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && layerId != 0)
    {
        MapLayer *layer = reinterpret_cast<MapLayer*>(layerId);
        if(layer)
        {
            layer->Invalidate(zoom);
        }
    }
}

static void JNI_InvalidAllTiles (JNIEnv* env, jobject thiz, jint layerId)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController && layerId != 0)
    {
        MapLayer *layer = reinterpret_cast<MapLayer*>(layerId);
        if(layer)
        {
            layer->Invalidate();
        }
    }
}

static void JNI_SnapShot (JNIEnv* env, jobject thiz, jobject obj, jint x, jint y, jint width, jint height)
{
    MapContext* context = reinterpret_cast<MapContext*>(env->GetIntField(thiz, fields.context));
    if(context && context->mapController)
    {
        shared_ptr<JniSnapShotCallback> callback = shared_ptr<JniSnapShotCallback>(new JniSnapShotCallback(obj));
        context->mapController->SnapShot(callback, x, y, width, height);
    }
}

static JNINativeMethod gMethods[] =
{
    { "createMapContext",     "(I)V",                                       (void *) &JNI_CreateMapContext      },
    { "destroyMapContext",    "()V",                                        (void *) &JNI_DestroyMapContext     },
    { "createMap",            "(DDDIIIZZZILjava/lang/String;Ljava/lang/String;Ljava/lang/String;FLjava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/String;)V", (void *) &JNI_CreateMap },
    { "setPosition",          "(DD)V",                                      (void *) &JNI_SetPosition           },
    { "addPin",               "(DDFFFF[B[B[F[F[F[FIIIILcom/locationtoolkit/map3d/model/Bubble;)I", (void *) &JNI_AddPin },
    { "removePin",            "(I)V",                                       (void *) &JNI_RemovePin             },
    { "selectPin",            "(IZ)V",                                      (void *) &JNI_SelectPin             },
    { "removeAllPins",        "()V",                                        (void *) &JNI_RemoveAllPins         },
    { "updatePinPosition",     "(IDD)V",                                    (void *) &JNI_UpdatePinPosition     },
    { "showTraffic",          "(Z)V",                                       (void *) &JNI_ShowTraffic           },
    { "enableDebugView",      "(Z)V",                                       (void *) &JNI_EnableDebugView       },
    { "setAvatarLocation",    "(Ljava/lang/Object;)V",                      (void *) &JNI_SetAvatarLocation     },
    { "setAvatarMode",        "(I)V",                                       (void *) &JNI_SetAvatarMode         },
    { "setGpsMode",           "(I)V",                                       (void *) &JNI_SetGpsMode            },
    { "showDoppler",          "(Z)V",                                       (void *) &JNI_ShowDoppler           },
    { "setCamera",            "(DDFFFZIII)V",                               (void *) &JNI_SetCamera             },
    { "createCamera",         "(DDDIIFFFFFF[D[D[F[F[F)V",                   (void *) &JNI_CreateCameraForNav    },
    { "createCamera",         "(IIFDDDD[D[D[F[F[F)V",                       (void *) &JNI_CreateCamera          },
    { "createCamera",         "(DDFFFIIII[D[D[F[F[F)V",                     (void *) &JNI_ScrollCamera          },
    { "prefetch",             "([DDDF)V",                                   (void *) &JNI_PrefetchForNav        },
    { "prefetch",             "(DDFFF)V",                                   (void *) &JNI_Prefetch              },
    { "setAnimationLayerOpacity", "(I)V",                                   (void *) &JNI_SetAnimationLayerOpacity},
    { "configureMetadata",    "(IZ)V",                                      (void *) &JNI_ConfigureMetadata     },
    { "setNightMode",         "(I)V",                                       (void *) &JNI_SetNightMode          },
    { "isNightMode",          "()Z",                                        (void *) &JNI_IsNightMode           },
    { "addPolyline",          "(FI[D[I[I[I[F[Ljava/lang/Object;[II[I[IIB)I",(void *) &JNI_AddPolyline          },
    { "removeGeoGraphics",    "([I)V",                                      (void *) &JNI_RemoveGeoGraphics     },
    { "selectGeoGraphics",    "(IZ)V",                                      (void *) &JNI_GeographicsSetSelected},
    { "enableGeoGraphicsSelectedListner", "(IZ)V",                          (void *) &JNI_GeographicsEnableSelectedListner},
    { "getSelectedElements",  "()[I",                                       (void *) &JNI_GeographicsGetSelectedElements},
    { "enableCustomLayerCollisionDetection", "(IZ)V",                      (void *) &JNI_EnableCustomLayerCollisionDetection},
    { "setPolylineWidth",     "(IF)V",                                      (void *) &JNI_PolylineSetWidth      },
    { "setPolylineColors",    "(I[I)V",                                     (void *) &JNI_PolylineSetColors     },
    { "setPolylineZOrder",    "(II)V",                                      (void *) &JNI_PolylineSetZOrder     },
    { "setPolylineVisible",   "(IZ)V",                                       (void *) &JNI_PolylineSetVisible     },
    { "setCompassPosition",   "(FF)V",                                      (void *) &JNI_SetCompassPosition    },
    { "setCompassEnabled",    "(Z)V",                                       (void *) &JNI_SetCompassEnabled     },
    { "setPinBubbleResolver", "(Ljava/lang/Object;)V",                      (void *) &JNI_SetPinBubbleResolver  },
    { "setNavigationModeEnabled", "(Z)V",                                   (void *) &JNI_SetNavigationModeEnabled},
    { "toScreenPosition",     "(DD[I[I)Z",                                  (void *) &JNI_ToScreenPosition      },
    { "fromScreenPosition",   "(II[D[D)Z",                                  (void *) &JNI_FromScreenPosition    },
    { "metersPerPixel",       "()D",                                        (void *) &JNI_MetersPerPixel        },
    { "getCamera",            "([D[D[F[F[F)Z",                              (void *) &JNI_GetCamera             },
    { "setCompassIcons",      "(Ljava/lang/String;Ljava/lang/String;)V",    (void *) &JNI_SetCompassIcons       },
    { "showOptionalLayer",    "([BZ)V",                                     (void *) &JNI_ShowOptionalLayer     },
    { "getCompassBoundingBox","([F)V",                                      (void *) &JNI_GetCompassBoundingBox },
    { "beginAtomicUpdate",    "()V",                                        (void *) &JNI_BeginAtomicUpdate     },
    { "endAtomicUpdate",      "()V",                                        (void *) &JNI_EndAtomicUpdate       },
    { "stopAnimations",       "(I)V",                                       (void *) &JNI_StopAnimations        },
    { "setBackground",        "(Z)V",                                       (void *) &JNI_SetBackground         },
    { "playDoppler",          "()V",                                        (void *) &JNI_PlayDoppler           },
    { "pauseDoppler",         "()V",                                        (void *) &JNI_PauseDoppler          },
    { "addCircle",            "(DDFIIIZ)I",                                 (void *) &JNI_AddCircle             },
    { "removeCircle",         "(I)V",                                       (void *) &JNI_RemoveCircle          },
    { "removeAllCircles",     "()V",                                        (void *) &JNI_RemoveAllCircles      },
    { "setCircleCenter",      "(IDD)V",                                     (void *) &JNI_SetCircleCenter       },
    { "setCircleVisible",     "(IZ)V",                                      (void *) &JNI_SetCircleVisible      },
    { "setCircleRadius",      "(IF)V",                                      (void *) &JNI_SetCircleRadius       },
    { "setCircleFillColor",   "(II)V",                                      (void *) &JNI_SetCircleFillColor    },
    { "setCircleStrokeColor", "(II)V",                                      (void *) &JNI_SetCircleStrokeColor  },
    { "setReferenceCenter",   "(FF)V",                                      (void *) &JNI_SetReferenceCenter    },
    { "enableReferenceCenter", "(Z)V",                                      (void *) &JNI_EnableReferenceCenter },
    { "addRect2D",            "(DDFFFI[BZ)I",                               (void *) &JNI_AddRect2D             },
    { "removeRect2D",         "(I)V",                                       (void *) &JNI_RemoveRect2D          },
    { "setRect2DVisible",     "(IZ)V",                                      (void *) &JNI_SetRect2DVisible      },
    { "setRect2DSize",        "(IFF)V",                                     (void *) &JNI_SetRect2DSize         },
    { "updateRect2D",         "(IDDF)V",                                    (void *) &JNI_UpdateRect2D          },
    { "removeAllRect2Ds",     "()V",                                        (void *) &JNI_RemoveAllRect2Ds      },
    { "addTexture",           "(I[B)V",                                     (void *) &JNI_AddTexture            },
    { "removeTexture",        "(I)V",                                       (void *) &JNI_RemoveTexture         },
    { "removeAllTextures",    "()V",                                        (void *) &JNI_RemoveAllTextures     },
    { "masterClear",          "()V",                                        (void *) &JNI_MasterClear           },
    { "createTile",           "(IIII[I[F[F[D[Ljava/lang/Object;[I)I",       (void *) &JNI_CreateTile            },
    { "destroyTile",          "(I)V",                                       (void *) &JNI_DestroyTile           },
    { "addLayer",             "(Ljava/lang/Object;IIIIIZ)I",                (void *) &JNI_AddLayer              },
    { "removeLayer",          "(I)V",                                       (void *) &JNI_RemoveLayer           },
    { "setLayerVisible",      "(IZ)V",                                      (void *) &JNI_SetLayerVisible       },
    { "invalidateLayer",      "(IIII)V",                                    (void *) &JNI_InvalidTile           },
    { "invalidateLayer",      "(II)V",                                      (void *) &JNI_InvalidTilesByZoom    },
    { "invalidateLayer",      "(I)V",                                       (void *) &JNI_InvalidAllTiles       },
    { "setAvatarScale",       "(F)V",                                       (void *) &JNI_SetAvatarScale        },
    { "snapShot",             "(Ljava/lang/Object;IIII)V",                  (void *) &JNI_SnapShot              }
};

int register_mapview(JavaVM* vm, JNIEnv* env)
{
    jint result = JNI_ERR;

    jclass clazz = env->FindClass("com/locationtoolkit/map3d/internal/jni/NativeMapController");
    if (clazz != NULL)
    {
        fields.clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
        if (fields.clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for com/locationtoolkit/map3d/internal/jni/NativeMapController"));
            return result;
        }
    }
    else
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find com/locationtoolkit/map3d/internal/jni/NativeMapController"));
        return result;
    }

    jclass snapshotwrapper_clazz = env->FindClass("com/locationtoolkit/map3d/internal/jni/SnapShotWrapper");
    if (snapshotwrapper_clazz != NULL)
    {
        fields.snapshotwrapper_clazz = static_cast<jclass>(env->NewGlobalRef(snapshotwrapper_clazz));
        /* The local reference is no longer useful */
        env->DeleteLocalRef(snapshotwrapper_clazz);
        if (fields.snapshotwrapper_clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for com/locationtoolkit/map3d/internal/jni/SnapShotWrapper"));
            return result;
        }
    }
    else
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find com/locationtoolkit/map3d/internal/jni/SnapShotWrapper"));
        return result;
    }

    fields.context = env->GetFieldID(fields.clazz, "mNativeContext", "I");

    if (env->RegisterNatives(fields.clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
    {
        goto fail;
    }

    fields.onCameraUpdateMethodID = env->GetMethodID(fields.clazz, "onCameraUpdate", "(DDFFF)V");
    if (fields.onCameraUpdateMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onCameraUpdateMethodID"));
        goto fail;
    }

    fields.onSingleTapMethodID = env->GetMethodID(fields.clazz, "onSingleTap", "(FF[I)Z");
    if (fields.onSingleTapMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onSingleTapMethodID"));
        goto fail;
    }

    fields.onTapMethodID = env->GetMethodID(fields.clazz, "onTap", "(DD)V");
    if (fields.onTapMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onTapMethodID"));
        goto fail;
    }

    fields.onMapCreatedMethodID = env->GetMethodID(fields.clazz, "onMapCreated", "()V");
    if (fields.onMapCreatedMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onMapCreatedMethodID"));
        goto fail;
    }

    fields.onStaticPOIClickedMethodID = env->GetMethodID(fields.clazz, "onStaticPOIClicked", "(Ljava/lang/String;Ljava/lang/String;DD)V");
    if (fields.onStaticPOIClickedMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onStaticPOIClickedMethodID"));
        goto fail;
    }

    fields.onTraficInsidentPinClickedMethodID = env->GetMethodID(fields.clazz, "onTrafficIncidentPinClicked", "(DD)V");
    if (fields.onTraficInsidentPinClickedMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onTrafficIncidentPinClicked"));
        goto fail;
    }

    fields.onPinClickedMethodID = env->GetMethodID(fields.clazz, "onPinClicked", "(I)V");
    if (fields.onPinClickedMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onPinClickedMethodID"));
        goto fail;
    }

    fields.onOptionalLayerUpdatedMethodID = env->GetMethodID(fields.clazz, "onOptionalLayerUpdated", "([B)V");
    if (fields.onOptionalLayerUpdatedMethodID == NULL)
    {
        abort();
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onOptionalLayerUpdatedMethodID"));
        goto fail;
    }

    fields.onAvatarClickedID = env->GetMethodID(fields.clazz, "onAvatarClicked", "(DD)V");
    if (fields.onAvatarClickedID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onAvatarClickedID"));
        goto fail;
    }

    fields.onMapReadyID = env->GetMethodID(fields.clazz, "onMapReady", "()V");
    if (fields.onMapReadyID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapController.onMapReadyID"));
        goto fail;
    }

    fields.onCameraAnimationDoneID = env->GetMethodID(fields.clazz, "onCameraAnimationDone", "(II)V");
    if (fields.onCameraAnimationDoneID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapController.onCameraAnimationDone"));
        goto fail;
    }

    fields.onCameraLimitedID = env->GetMethodID(fields.clazz, "onCameraLimited", "(I)V");
    if (fields.onCameraLimitedID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapController.onCameraLimitedID"));
        goto fail;
    }

    fields.onNightModeChangedID = env->GetMethodID(fields.clazz, "onNightModeChanged", "(Z)V");
    if (fields.onNightModeChangedID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMapController.onNightModeChanged"));
        goto fail;
    }

    fields.onMarkerClickedMethodID = env->GetMethodID(fields.clazz, "onMarkerClicked", "(I)V");
    if (fields.onMarkerClickedMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBGMView.onMarkerClickedMethodID"));
        goto fail;
    }

    fields.onSnapShotSuccessed = env->GetMethodID(fields.snapshotwrapper_clazz, "onSuccessed", "([BII)V");
    if (fields.onSnapShotSuccessed == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find SnapShotWrapper.onSuccessed"));
        goto fail;
    }

    fields.onSnapShotError = env->GetMethodID(fields.snapshotwrapper_clazz, "onError", "(I)V");
    if (fields.onSnapShotError == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find SnapShotWrapper.onError"));
        goto fail;
    }

    cachedVM = vm;
    result = JNI_OK;

fail:
    return result;
}

int unregister_mapview(JNIEnv* env)
{
    return JNI_OK;
}


AndroidNativeUIListener::AndroidNativeUIListener(jobject androidObj): mAndroidObject(androidObj)
{
}

AndroidNativeUIListener::~AndroidNativeUIListener()
{
    JNIEnv* env = getJNIEnv();
    env->DeleteGlobalRef(mAndroidObject);
}

void
AndroidNativeUIListener::OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt)
{
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        env->CallVoidMethod(mAndroidObject, fields.onCameraUpdateMethodID,
                static_cast<jdouble>(lat), static_cast<jdouble>(lon), static_cast<float>(zoomLevel),
                static_cast<jfloat>(heading), static_cast<jfloat>(tilt));
    }
}

bool AndroidNativeUIListener::OnSingleTap(float x, float y, std::set<Geographic*>& graphics)
{
    JNIEnv* env = getJNIEnv();
    jboolean result = false;

    jsize idCount = graphics.size();
    jintArray ids = env->NewIntArray(idCount);
    jint* buf = new jint[idCount];
    int i = 0;
    for (std::set<Geographic*>::iterator it = graphics.begin(); it != graphics.end(); ++it)
    {
        buf[i++] = reinterpret_cast<jint>(*it);
    }
    env->SetIntArrayRegion(ids, 0, idCount, buf);
    delete[] buf;
    result = env->CallBooleanMethod(mAndroidObject, fields.onSingleTapMethodID, static_cast<jfloat>(x), static_cast<jfloat>(y), ids);
    env->DeleteLocalRef(ids);

    return result ? true : false;
}

void AndroidNativeUIListener::OnTap(double latitude, double longitude)
{
    JNIEnv* env = getJNIEnv();
    env->CallVoidMethod(mAndroidObject, fields.onTapMethodID,
                        static_cast<jdouble>(latitude),
                        static_cast<jdouble>(longitude));
}

bool AndroidNativeUIListener::OnGeoGraphicSelect(Geographic* graphic, bool isSelected)
{
    return false;
}

void AndroidNativeUIListener::UpdateOptionalLayers(const std::vector<LayerAgentPtr>& layers)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        LayerAgentSerializer serializer(layers);
        jsize      size   = (jsize)serializer.GetBufferSize();
        jbyteArray array = env->NewByteArray(size);
        env->SetByteArrayRegion(array, 0, size, (jbyte*) serializer.GetBuffer());
        env->CallVoidMethod(mAndroidObject, fields.onOptionalLayerUpdatedMethodID, array);
        env->DeleteLocalRef(array);
    }
}

void AndroidNativeUIListener::OnPinClicked(MapPin* pin)
{
    JNIEnv* env = getJNIEnv();
    env->CallVoidMethod(mAndroidObject, fields.onPinClickedMethodID, reinterpret_cast<jint>(pin));
}

void AndroidNativeUIListener::OnStaticPOIClicked(const string &id, const string &name, double lat, double lon)
{
    JNIEnv* env = getJNIEnv();
    if(env == NULL)
    {
        return;
    }
    jstring jid = env->NewStringUTF(id.c_str());
    jstring jname = env->NewStringUTF(name.c_str());
    env->CallVoidMethod(mAndroidObject, fields.onStaticPOIClickedMethodID, jid, jname, lat, lon);
    env->DeleteLocalRef(jid);
    env->DeleteLocalRef(jname);
}

void AndroidNativeUIListener::OnTrafficIncidentPinClicked(double lat, double lon)
{
    JNIEnv* env = getJNIEnv();
    if(env == NULL)
    {
        return;
    }
    env->CallVoidMethod(mAndroidObject, fields.onTraficInsidentPinClickedMethodID, lat, lon);
}

void
AndroidNativeUIListener::OnAvatarClicked(double lat, double lon)
{
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        env->CallVoidMethod(mAndroidObject, fields.onAvatarClickedID,
                static_cast<jdouble>(lat), static_cast<jdouble>(lon));
    }
}

void
AndroidNativeUIListener::OnMapCreate()
{
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        env->CallVoidMethod(mAndroidObject, fields.onMapCreatedMethodID);
    }
}

void
AndroidNativeUIListener::OnMapReady()
{
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        env->CallVoidMethod(mAndroidObject, fields.onMapReadyID);
    }
}

void
AndroidNativeUIListener::OnCameraAnimationDone(int animationId, int animationStatus)
{
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        env->CallVoidMethod(mAndroidObject, fields.onCameraAnimationDoneID, animationId, animationStatus);
    }
}

void
AndroidNativeUIListener::OnCameraLimited(int limitationType)
{
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        env->CallVoidMethod(mAndroidObject, fields.onCameraLimitedID, limitationType);
    }
}

void
AndroidNativeUIListener::OnNightModeChanged(bool isNightMode)
{
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        env->CallVoidMethod(mAndroidObject, fields.onNightModeChangedID, isNightMode);
    }
}

void AndroidNativeUIListener::OnMarkerClicked(int id)
{
    JNIEnv* env = getJNIEnv();
    if(env == NULL)
    {
        return;
    }
    env->CallVoidMethod(mAndroidObject, fields.onMarkerClickedMethodID, id);
}
