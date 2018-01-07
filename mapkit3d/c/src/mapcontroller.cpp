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
 @file     mapcontroller.cpp
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

#include "mapcontroller.h"
#include "paldebuglog.h"
#include "palstdlib.h"
#include "palclock.h"
#include "MapServicesConfiguration.h"
#include "nbnetwork.h"
#include "nbnetworkoptions.h"
#include "palfile.h"
#include "MetadataConfiguration.h"
#include <algorithm>
#include "mappin.h"
#include "palevent.h"
#include "paluitaskqueue.h"
#include "mapcircle.h"
#include "maprect2d.h"
#include "palfile.h"
#include "maptile.h"
#include "CustomNBMTile.h"
#include "contextbasedsingleton.h"
#include "TileFactory.h"
#include "maplayer.h"
#include "CustomLayerManagerImpl.h"
#include "camerahelper.h"

#define  MAX_PATH_LENGTH 512

using namespace nbmap;

static const char PIN_MATERIAL_CONFIG_FILENAME[] = "pin_materials.xml";
static const char CUSTOM_PIN_LAYER_ID[] = "custom_pin_layer";
static const char METADATA_FILE_NAME[] = "metadata";

uint8 get_language_code(const char* language)
{
    if (nsl_stricmp(language, "en-us") == 0)
    {
        return 20;  //English
    }
    else if (nsl_stricmp(language, "zh-cn") == 0)
    {
        return 12;  //Chinese (Modern)
    }
    else if (nsl_strnicmp(language, "es-", 3) == 0)
    {
        return 64;  //Spanish
    }
    else if (nsl_strnicmp(language, "en-", 3) == 0)
    {
        return 20;  //English
    }

    return 0;
}

/*! Helper functor to compare graphic Id */
class GeographicFindById
{
public:
    GeographicFindById(set<std::string>* ids)
            : m_ids(ids){}
    virtual ~GeographicFindById(){}
    bool operator() (const nbmap::Geographic* graphic) const
    {
        return !(graphic && m_ids->find(graphic->GetId()) != m_ids->end());
    }
private:
    set<std::string>* m_ids;
};

class MetadataConfigurationData
{
public:
    MetadataConfigurationData(MapController *map, int metaDataType, bool enable):mMap(map),
        mMetaDataType(metaDataType),
        mEnable(enable) {}
    ~MetadataConfigurationData() {}

 public:
    MapController *mMap;
    int mMetaDataType;
    bool mEnable;
};

class AddPinData
{
public:
    AddPinData(MapController &map, MapPin* pin):mMap(map),
        mPin(pin) {}
    ~AddPinData() {}

 public:
    MapController &mMap;
    vector<nbmap::PinParameters<nbmap::BubbleInterface> > mPinData;
    MapPin* mPin;
};

class AddLayerData
{
public:
    AddLayerData(MapController &map, MapLayer* layer):mMap(map),
        mLayer(layer) {}
    ~AddLayerData() {}

 public:
    MapController &mMap;
    TileProviderPtr mProvider;
    int mMinZoom;
    int mMaxZoom;
    int mRefZoom;
    bool mVisible;
    int mMainOrder;
    int mSubOrder;
    MapLayer* mLayer;
};

class RemoveLayerData
{
public:
    RemoveLayerData(MapController &map, MapLayer* layer):mMap(map),
        mLayer(layer) {}
    ~RemoveLayerData() {}

 public:
    MapController &mMap;
    MapLayer* mLayer;
};

class EnableLayerCollisionData
{
public:
    EnableLayerCollisionData(MapController &map, MapLayer* layer, bool enable)
        :mMap(map),
        mLayer(layer),
        mEnable(enable)         {}
    ~EnableLayerCollisionData() {}

public:
    MapController &mMap;
    MapLayer* mLayer;
    bool mEnable;
};

class CreateTileData
{
public:
    CreateTileData(MapController &map, MapTile* tile, int x, int y, int z):mMap(map),
        mX(x),
        mY(y),
        mZoom(z),
        mTile(tile){}
    ~CreateTileData() {}

 public:
    MapController &mMap;
    int mX;
    int mY;
    int mZoom;
    vector<nbmap::PinParameters<nbmap::BubbleInterface> > mPinParamerts;
    MapTile* mTile;
};

class RemovePinData
{
public:
    RemovePinData(MapController &map, MapPin* pin):m_mapController(map),
        m_pin(pin) {}
    ~RemovePinData() {}

 public:
    MapController &m_mapController;
    MapPin* m_pin;
};

class DestroyTileData
{
public:
    DestroyTileData(MapController &map, MapTile* tile):m_mapController(map),
        m_tile(tile) {}
    ~DestroyTileData() {}

 public:
    MapController &m_mapController;
    MapTile* m_tile;
};

class LoadTileData
{
public:
    LoadTileData(MapController &map, MapTile* tile):m_mapController(map),
        m_tile(tile) {}
    ~LoadTileData() {}

 public:
    MapController &m_mapController;
    MapTile* m_tile;
};

class RemoveAllPinsData
{
public:
    RemoveAllPinsData(MapController &map, const set<MapPin*> &pins):m_mapController(map),
        m_pins(pins) {}
    ~RemoveAllPinsData() {}

 public:
    MapController &m_mapController;
    set<MapPin*> m_pins;
};

class PinClickedData
{
public:
    PinClickedData(MapController &map, MapPin *pin):m_mapController(map),
        m_pin(pin) {}
    ~PinClickedData() {}

 public:
    MapController &m_mapController;
    MapPin *m_pin;
};
class CreateMapParameters
{
public:
    CreateMapParameters(MapController& controller,
                        const MapViewConfiguration& config, void* mapViewContext)
            : mController(controller),
              mConfig(config),
              mViewContext(mapViewContext)
    {}
    virtual ~CreateMapParameters(){}

    MapController&       mController;
    MapViewConfiguration mConfig;
    void*                mViewContext;
};

class ShowTrafficParameter
{
public:
    ShowTrafficParameter(MapController& controller,
                         bool enabled)
            : m_controller(controller),
              m_enabled(enabled)
    {}
    virtual ~ShowTrafficParameter(){}

    MapController& m_controller;
    bool           m_enabled;
};

void UiTaskQueueCreatedCallback( PAL_Instance* pal, void* userData )
{
}

class MapInitializeCallback : public AsyncCallback<void*>
{
public:
    MapInitializeCallback(){}
    virtual ~MapInitializeCallback(){};

public:
    /* Overwrites from AsyncCallback. This gets called once the map is fully initialized (or the initialization failed) */
    virtual void Success(void*){}
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

// Helper class is used to erase shared_ptr from a set quickly
template<typename T>
class FindByPointer
{
public:
    FindByPointer(T* obj):mObj(obj) {}
    ~FindByPointer() {}

    bool operator()(const shared_ptr<T> element)const
    {
        return element.get() == mObj;
    }

private:
    T *mObj;
};
namespace nbmap
{

class DefaultMapViewListener : public MapViewListener
{
public:
    /*! Default constructor. */
    DefaultMapViewListener(MapController* map):mMap(map) {};

    /*! Default destructor. */
    virtual ~DefaultMapViewListener() {};

    virtual bool GestureDuringFollowMeMode( MapViewGestureType gesture )
    {
        return true;
    }

    virtual void LayersUpdated()
    {
        mMap->OnLayerUpdate();
    }

    virtual bool OnSingleTap( float screenX, float screenY, const set<std::string>& ids)
    {
        return mMap->OnSingleTap(screenX, screenY, ids);
    }

    virtual void OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt)
    {
        mMap->OnCameraUpdate(lat, lon, zoomLevel, heading, tilt);
    }

    virtual void OnPinClicked(const std::string& id)
    {
        mMap->OnPinClicked(id);
    }

    virtual void OnUnselectAllPins()
    {
        mMap->OnUnselectAllPins();
    }

    virtual void OnAvatarClicked(double lat, double lon)
    {
        mMap->OnAvatarClicked(lat, lon);
    }

    virtual void OnMapCreate()
    {
        mMap->OnMapCreate();
    }

    virtual void OnMapReady()
    {
        mMap->OnMapReady();
    }

    virtual void OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus)
    {
        mMap->OnCameraAnimationDone(animationId, animationStatus);
    }

    virtual void OnNightModeChanged(bool isNightMode)
    {
        mMap->OnNightModeChanged(isNightMode);
    }

    virtual void OnStaticPOIClicked(const std::string& id, const std::string& name, double lat, double lon)
    {
        mMap->OnStaticPOIClicked(id, name, lat, lon);
    }

    virtual void OnTrafficIncidentPinClicked(double lat, double lon)
    {
        mMap->OnTrafficIncidentPinClicked(lat, lon);
    }

    virtual void OnTap(double latitude, double longitude)
    {
        mMap->OnTap(latitude, longitude);
    }

    virtual void OnCameraLimited(MapViewCameraLimitedType type)
    {
        mMap->OnCameraLimited(type);
    }
    virtual void OnMarkerClicked(int id)
    {
        mMap->OnMarkerClicked(id);
    }
    virtual void OnLongPressed(double lat, double lon)
    {
        mMap->OnLongPressed(lat, lon);
    }
    virtual void OnGesture(MapViewGestureType type, MapViewGestureState state, int time)
    {
        mMap->OnGesture(type, state, time);
    }
private:
    MapController* mMap;
};
}

class DefaultGenerateMapCallback : public GenerateMapCallback
{
public:
    /*! Default constructor. */
    DefaultGenerateMapCallback(shared_ptr<SnapShotCallback> callback):m_callback(callback) {};

    /*! Default destructor. */
    virtual ~DefaultGenerateMapCallback() {}

    virtual void OnSnapShotSuccessed(nbcommon::DataStreamPtr dataStream, uint32 width, uint32 height)
    {
        m_callback->OnSuccessed(dataStream, width, height);
    }

    virtual void OnSnapShotError(NB_Error error)
    {
        m_callback->OnError(error);
    }

private:
    shared_ptr<SnapShotCallback> m_callback;
};

class MasterClearParameters
{
public:
    MasterClearParameters(NB_Context* context, PAL_Event* event)
        :mContext(context), mEvent(event)
    {}
    virtual ~MasterClearParameters(){}

    NB_Context*  mContext;
    PAL_Event*   mEvent;
};

class UpdatePinPositionData
{
public:
    UpdatePinPositionData(MapController &map, MapPin* pin, double lat, double lon)
        :m_mapController(map),
         m_pin(pin),
         m_lat(lat),
         m_lon(lon){}
    ~UpdatePinPositionData() {}

 public:
    MapController &m_mapController;
    MapPin* m_pin;
    double m_lat;
    double m_lon;
};

MapController::MapController(NB_Context* nbContext, PAL_Instance* pal):
        mNBContext(nbContext),
        mPal(pal),
        mMapView(NULL),
        mMapListener(NULL),
        mProjection(NULL),
        mNextModelId(1),
        mEnableSSAO(false)
{
    mDefaultMapViewListener = new DefaultMapViewListener(this);
}

MapController::~MapController()
{
    delete mDefaultMapViewListener;
    if (mMapView)
    {
        Destroy();
    }
}

void MapController::Create(const MapViewConfiguration& config, void* mapViewContext)
{
    // Create UI task queue first.
    PAL_Error error = PAL_UiTaskQueueAdd(mPal, UiTaskQueueCreatedCallback, NULL);

    // Post task to CCC thread to create mapview.
    uint32 taskId = 0;
    error = error ? error : PAL_EventTaskQueueAdd(mPal,
                                                  MapController::CCC_Thread_CreateMap,
                                                  new CreateMapParameters(*this, config, mapViewContext),
                                                  &taskId);
    if (error!= PAL_Ok)
    {
        abort();
    }
}

void MapController::Destroy()
{
    //@todo: remove custom objects.
    RemoveAllCustomTiles();
    RemoveAllPins();
    RemoveAllCustomLayers();
    RemoveAllCircles();
    RemoveAllRect2ds();
    mOptionalLayersAgent.reset();
    mPinLayer.reset();
    mPinManager.reset();
    mMapView->DestroyRenderer();
    mMapView->Destroy();
    mMapView = NULL;
}

void MapController::BeginAtomicUpdate()
{
    if(mMapView)
    {
        mMapView->BeginAtomicUpdate();
    }
}

void MapController::EndAtomicUpdate()
{
    if(mMapView)
    {
        mMapView->EndAtomicUpdate();
    }
}

void MapController::SetCamera(double latitude, double longitude, float zoomLevel, float heading, float tilt, bool animated, unsigned int duration, CameraAnimationAccelerationType accelerationType, int cameraAnimationId)
{
    if(mMapView)
    {
        MapViewCameraAnimationAccelertaionType type = MVCAAT_LINEAR;
        switch(accelerationType)
        {
        case CAAT_LINEAR:
            type = MVCAAT_LINEAR;
            break;
        case CAAT_DECELERATION:
            type = MVCAAT_DECELERATION;
            break;
        case CAAT_ACCELERATION:
            type = MVCAAT_ACCELERATION;
            break;
        default:
            break;
        }
        MapViewCameraParameter para;
        para.latitude = latitude;
        para.longitude = longitude;
        para.zoomLevel = zoomLevel;
        para.heading = heading;
        para.tilt = tilt;
        para.id = cameraAnimationId;
        mMapView->SetCameraSettings(para, animated, duration, type);
    }
}

void MapController::SetPosition(double latitude, double longitude)
{
    if(mMapView)
    {
        MapViewCameraParameter para;
        para.latitude = latitude;
        para.longitude = longitude;
        para.heading = INVALID_CAMERA_HEADING;
        para.tilt = INVALID_CAMERA_TILT;
        para.zoomLevel = INVALID_CAMERA_ZOOMLEVEL;
        mMapView->SetCamera(para);
    }
}

void MapController::CCCThread_AddPin(vector<nbmap::PinParameters<nbmap::BubbleInterface> > &pinData)
{
    if(mMapView)
    {
        mMapView->AddPins(pinData);
    }
}

void MapController::AddPinFunc(PAL_Instance* pal, void* userData)
{
     AddPinData *param = static_cast<AddPinData*>(userData);
     if(!pal || !param)
     {
        return;
     }

     MapController &mapCotroller = param->mMap;
     MapPin* pin = param->mPin;

     if(!pin)
     {
        delete param;
        return;
     }

     mapCotroller.CCCThread_AddPin(param->mPinData);
     if (!param->mPinData.empty() && param->mPinData.back().m_pinId)
     {
        pin->m_id = shared_ptr<string>(new string(*(param->mPinData.back().m_pinId)));
        mapCotroller.mPinMap.insert(make_pair(*(pin->m_id), pin));
     }
     delete param;
}

void MapController::RemovePinFunc(PAL_Instance* pal, void* userData)
{
    RemovePinData *data = static_cast<RemovePinData*>(userData);
    if(!pal || !data)
    {
       return;
    }

    MapController &mapCotroller = data->m_mapController;
    MapPin* pin = data->m_pin;
    delete data;

    if(!pin)
    {
       return;
    }

    if(mapCotroller.mMapView && pin->m_id)
    {
        vector<shared_ptr<string> > ids;
        ids.push_back(shared_ptr<string>(new string(*(pin->m_id))));
        mapCotroller.mMapView->RemovePins(ids);
        mapCotroller.mPinMap.erase(*(pin->m_id));
    }

    delete pin;
}

void MapController::UpdatePinPositionFunc(PAL_Instance* pal, void* userData)
{
    UpdatePinPositionData *data = static_cast<UpdatePinPositionData*>(userData);
    if(!pal || !data)
    {
       return;
    }

    MapController &mapCotroller = data->m_mapController;
    MapPin* pin = data->m_pin;
    double lat = data->m_lat;
    double lon = data->m_lon;
    delete data;

    if(!pin)
    {
       return;
    }

    if(mapCotroller.mMapView && pin->m_id)
    {
        mapCotroller.mMapView->UpdatePinPosition(shared_ptr<string>(new string(*(pin->m_id))), lat, lon);
    }
}

MapPin*
MapController::AddPin(nbmap::PinParameters<nbmap::BubbleInterface> &pinParameter)
{
    if(mMapView && mPinLayer)
    {
        MapPin* pin = new MapPin(*mPal, *this);
        if(pin == NULL)
        {
            return NULL;
        }

        AddPinData* data = new AddPinData(*this, pin);
        if(data == NULL)
        {
            delete pin;
            return NULL;
        }

        mPinSet.insert(pin);
        data->mPinData.push_back(pinParameter);
        uint32 taskId = 0;
        PAL_EventTaskQueueAdd(mPal,
                              MapController::AddPinFunc,
                              data,
                              &taskId);
        return pin;
    }
    return NULL;
}

void MapController::RemovePin(MapPin *pin)
{
    if(mMapView && pin)
    {
        set<MapPin*>::iterator it = mPinSet.find(pin);
        if(it == mPinSet.end())
        {
            return;
        }
        uint32 taskId = 0;
        PAL_EventTaskQueueAdd(mPal,
                              MapController::RemovePinFunc,
                              new RemovePinData(*this, pin),
                              &taskId);
        mPinSet.erase(it);
    }
}

void MapController::UpdatePinPosition(MapPin *pin, double lat, double lon)
{
    if(mMapView && pin)
    {
        set<MapPin*>::iterator it = mPinSet.find(pin);
        if(it == mPinSet.end())
        {
            return;
        }
        uint32 taskId = 0;
        PAL_EventTaskQueueAdd(mPal,
                              MapController::UpdatePinPositionFunc,
                              new UpdatePinPositionData(*this, pin, lat, lon),
                              &taskId);
    }
}
void MapController::SelectPin(MapPin *pin, bool selected)
{
    if(mMapView && pin)
    {
        set<MapPin*>::iterator it = mPinSet.find(pin);
        if(it == mPinSet.end())
        {
            return;
        }
        pin->SetSelected(selected);
    }
}

bool MapController::GetPinSelected(MapPin* pin)
{
    bool selected = false;

    if(mMapView && pin)
    {
        set<MapPin*>::iterator it = mPinSet.find(pin);
        if(it == mPinSet.end())
        {
            selected = false;
        }
        else
        {
            selected = pin->GetSelected();
        }
    }
    return selected;
}

void MapController::CCCThread_RemoveAllPins()
{
    if(mMapView)
    {
        // Traffic incident pin cannot be removed manually.
        mMapView->RemoveAllPins();
    }
}

void MapController::RemoveAllPinsFunc(PAL_Instance* pal, void* userData)
{
    RemoveAllPinsData *data = static_cast<RemoveAllPinsData*>(userData);
    if(!pal || !data)
    {
       return;
    }

    MapController &mapCotroller = data->m_mapController;

    mapCotroller.CCCThread_RemoveAllPins();
    for(set<MapPin*>::iterator it = data->m_pins.begin(); it != data->m_pins.end(); ++it)
    {
        delete *it;
    }
    mapCotroller.mPinMap.clear();
    delete data;
}

void MapController::RemoveAllPins()
{
    if(mMapView && mPinLayer)
    {
        uint32 taskId = 0;
        PAL_EventTaskQueueAdd(mPal,
                              MapController::RemoveAllPinsFunc,
                              new RemoveAllPinsData(*this, mPinSet),
                              &taskId);
        mPinSet.clear();
    }
}

void MapController::ShowTraffic(bool enable)
{
#ifdef DTRAFFIC
    qWarning("TRAFFIC: 4. MapController::ShowTraffic(%s)", enable?"true":"false");
#endif

    uint32 taskId = 0;
    PAL_EventTaskQueueAdd(mPal,
                          MapController::CCC_Thread_ShowTraffic,
                          new ShowTrafficParameter(*this, enable),
                          &taskId);
}

void MapController::EnableDebugView(bool enable)
{
    if(mMapView)
    {
        mMapView->SetViewSettings(MVS_SHOW_DEBUG_VIEW, enable);
    }
}

void MapController::CCCThread_RefreshMetadata(int metaDataType, bool enable)
{
    if(!mMapView)
    {
        return;
    }
    shared_ptr<nbmap::MetadataConfiguration> metadataConfigPtr = GetMetaDataConfiguration();
    if(!metadataConfigPtr)
    {
        return;
    }
    if(metaDataType & MDT_LABLELAYERS)
    {
        metadataConfigPtr->m_wantLableLayers = enable;
    }
    if(metaDataType & MDT_OPTIONALLAYERS)
    {
        metadataConfigPtr->m_wantOptionalLayers = enable;
    }
    if(metaDataType & MDT_WEATHERLAYERS)
    {
        metadataConfigPtr->m_wantWeatherLayer = enable;
    }
    if(metaDataType & MDT_DAM)
    {
        metadataConfigPtr->m_wantDAM = enable;
    }
    if(metaDataType & MDT_LOC)
    {
        metadataConfigPtr->m_wantLoc = enable;
    }
    if(metaDataType & MDT_2DBUILDINGS)
    {
        metadataConfigPtr->m_want2DBuildings = enable;
    }
    if(metaDataType & MDT_LOD)
    {
        metadataConfigPtr->m_wantLod = enable;
    }
    if(metaDataType & MDT_SATELLITELAYERS)
    {
        metadataConfigPtr->m_wantSatelliteLayers = enable;
    }
    mMapView->RefreshMetadataAndLayers(metadataConfigPtr);
}

void MapController::ConfigureMetadataFunc(PAL_Instance* pal, void* userData)
{
     MetadataConfigurationData *param = static_cast<MetadataConfigurationData*>(userData);
     if(!pal || !param)
     {
        return;
     }

     MapController *mapCotroller = param->mMap;
     int metaDataType            = param->mMetaDataType;
     bool enable                 = param->mEnable;
     delete param;

     if(mapCotroller)
     {
         mapCotroller->CCCThread_RefreshMetadata(metaDataType, enable);
     }
}

void MapController::ConfigureMetadata(int metaDataType, bool enable)
{
    if(mMapView)
    {
        uint32 taskId = 0;
        PAL_EventTaskQueueAdd(mPal,
                              MapController::ConfigureMetadataFunc,
                              new MetadataConfigurationData(this, metaDataType, enable),
                              &taskId);
    }
}

void MapController::SetAvatarLocation(const NB_GpsLocation &location)
{
    if(mMapView)
    {
        mMapView->SetAvatarLocation(location);
    }
}

void MapController::SetGpsMode(NB_GpsMode mode)
{
    if(mMapView)
    {
        mMapView->SetGpsMode(mode);
    }
}

NB_GpsMode MapController::GetGpsMode()
{
    if(mMapView)
    {
        return mMapView->GetGpsMode();
    }
    return NGM_INVALID;
}

void MapController::SetAvatarMode(MapViewAvatarMode mode)
{
    if(mMapView)
    {
        mMapView->SetAvatarMode(mode);
    }
}

void MapController::SetAnimationLayerOpacity(unsigned int opacity)
{
    if(mMapView)
    {
        mMapView->SetAnimationLayerOpacity(opacity);
    }
}

void
MapController::ShowCompass(bool enable)
{
    if(mMapView)
    {
        mMapView->ShowCompass(enable);
    }
}

void
MapController::SetCompassPosition(float screenX, float screenY)
{
    if(mMapView)
    {
        mMapView->SetCompassPosition(screenX, screenY);
    }
}

void
MapController::SetCompassIcons(const std::string& dayModeIconPath, const std::string& nightModeIconPath)
{
    if(mMapView)
    {
        mMapView->SetCompassIcons(dayModeIconPath, nightModeIconPath);
    }
}

void MapController::SetNightMode(unsigned int mode)
{
    if(mMapView)
    {
        mMapView->SetNightMode(static_cast <MapViewNightMode>(mode));
    }
}

bool MapController::IsNightMode()
{
    if(mMapView)
    {
        return mMapView->IsNightMode();
    }
    return false;
}

/* See description in header file. */
GeoPolyline* MapController::AddPolyline(const GeoPolylineOption& option)
{
    // Should be called in CCC thread
    return mOptionalLayersAgent ? mOptionalLayersAgent->AddPolyline(option) : NULL;
}

void
MapController::SetPolylineZorder(GeoPolyline* polyline, int zorder)
{
    if(mOptionalLayersAgent)
    {
        mOptionalLayersAgent->SetPolylineZorder(polyline, zorder);
    }
}

void
MapController::SetPolylineVisible(GeoPolyline* polyline, bool visible)
{
    if(mOptionalLayersAgent)
    {
        mOptionalLayersAgent->SetPolylineVisible(polyline, visible);
    }
}

void
MapController::SetPolylineSelected(GeoPolyline* polyline, bool selected)
{
    if(mOptionalLayersAgent)
    {
        mOptionalLayersAgent->SetPolylineSelected(polyline, selected);
    }
}

/* See description in header file. */
void MapController::RemoveGeographic(vector<Geographic*>& graphic)
{
    // Should be called in CCC thread
    if (mOptionalLayersAgent)
    {
        mOptionalLayersAgent->RemoveGeographic(graphic);
    }
}

void MapController::GetSelectedElements(std::vector<nbmap::Geographic*>& graphics) const
{
    if (!mMapView || !mOptionalLayersAgent)
    {
        return;
    }

    std::vector<nbmap::Geographic*> gs;
    // NBGM can only returns ID of objects, hence we need to convert Id to corresponding
    // Geographic Objects.
    mOptionalLayersAgent->GetAllGeographics(gs);
    for (std::vector<nbmap::Geographic*>::iterator it = gs.begin(); it != gs.end(); ++it)
    {
        nbmap::Geographic* g = *it;
        if (g->IsSelected())
        {
            graphics.push_back(g);
        }
    }
}

/* See description in header file. */
void MapController::PickedUpElements(int screenX, int screenY,
                                     std::vector<Geographic*>& graphics) const
{
    // Should be called in renderThread.
    if (!mMapView || !mOptionalLayersAgent)
    {
        return;
    }


    set<std::string> Ids;
    mMapView->GetElementsAt((float)screenX, (float)screenY, Ids);
    if (Ids.empty())
    {
        return;
    }

    // NBGM can only returns ID of objects, hence we need to convert Id to corresponding
    // Geographic Objects.
    mOptionalLayersAgent->GetAllGeographics(graphics);
    graphics.erase(remove_if(graphics.begin(), graphics.end(),GeographicFindById(&Ids)),
                   graphics.end());
}

/* See description in header file. */
void MapController::EnableCustomLayerCollisionDetection(MapLayer *layer, bool enable)
{
    if(mMapView && mCustomLayerManager && layer)
    {
        set<MapLayer*>::iterator it = mLayerSet.find(layer);
        if(it == mLayerSet.end())
        {
            return;
        }

        MapLayer* layer = *it;
        if(layer != NULL)
        {
            uint32 taskId = 0;
            PAL_EventTaskQueueAdd(mPal,
                MapController::EnableLayerCollisionFunc,
                new EnableLayerCollisionData(*this, layer, enable),
                &taskId);
        }
    }
}

void MapController::OnCameraUpdate(double lat, double lon, float zoomLevel, float heading, float tilt)
{
    if (mMapListener)
    {
        mMapListener->OnCameraUpdate(lat, lon, zoomLevel, heading, tilt);
    }
}

bool MapController::OnSingleTap(float screenX, float screenY, const set<std::string>& ids)
{
    if (!mMapView || !mOptionalLayersAgent || ids.empty())
    {
        return false;
    }

    // Select one item and cancel the select state of others.
    std::vector<Geographic*> graphics;
    mOptionalLayersAgent->GetAllGeographics(graphics);

    // Try handle event with listener
    if (mMapListener)
    {
        std::set<Geographic*> gs;
        for (std::vector<Geographic*>::iterator i = graphics.begin(); i != graphics.end(); ++i)
        {
            Geographic* g = *i;
            Geographic* parentGeographic = NULL;
            if (ids.find(g->GetId()) != ids.end())
            {
                parentGeographic = g->GetParentGeographic();
                if(parentGeographic)
                {
                    //Don't worry many same parent geographic in the 'gs', the 'gs' is std::set!!!
                    gs.insert(parentGeographic);
                }
                else
                {
                    gs.insert(g);
                }
            }
        }

        mMapListener->OnSingleTap(screenX, screenY, gs);
    }

    for (std::set<MapPin*>::iterator i = mPinSet.begin(); i != mPinSet.end(); ++i)
    {
        MapPin* pin = (*i);
        pin->m_selected = false;
    }

    return false;
}

void MapController::UnselectAllPinsCallBack(PAL_Instance* pal, void* userData)
{
    if (!pal)
    {
        return;
    }

    MapController* mapCotroller = static_cast<MapController*>(userData);

    for (std::set<MapPin*>::iterator i = mapCotroller->mPinSet.begin(); i != mapCotroller->mPinSet.end(); ++i)
    {
        MapPin* pin = (*i);
        pin->m_selected = false;
    }
}

void MapController::OnUnselectAllPins()
{
    if (mMapListener)
    {
        // Need switching to UI thread.
        PAL_UiTaskQueueAdd(mPal, UnselectAllPinsCallBack, this);
    }
}

void MapController::PinSelectedCallBack(PAL_Instance* pal, void* userData)
{
    if(!pal || !userData)
    {
       return;
    }

    PinClickedData* data = static_cast<PinClickedData*>(userData);
    MapController& mapCotroller = data->m_mapController;
    MapPin* pin = data->m_pin;
    delete data;

    if(!pin)
    {
       return;
    }

    if(mapCotroller.mPinLayer)
    {
        // Set all pins' states to unselected.
        for (std::set<MapPin*>::iterator i = mapCotroller.mPinSet.begin(); i != mapCotroller.mPinSet.end(); ++i)
        {
            MapPin* pin = (*i);
            pin->m_selected = false;
        }

        // Need checking the pin if removed.
        if(mapCotroller.mPinSet.find(pin) != mapCotroller.mPinSet.end())
        {
            // Set pin's state to selected.
            pin->m_selected = true;
            mapCotroller.mMapListener->OnPinClicked(pin);
        }
    }
}

void MapController::OnPinClicked(const std::string& id)
{
    if (mMapListener)
    {
        map<string, MapPin*>::iterator it = mPinMap.find(id);
        if(it != mPinMap.end())
        {
            MapPin* pin = it->second;
            if(pin)
            {
                // Need switching to UI thread.
                PAL_UiTaskQueueAdd(mPal, PinSelectedCallBack, new PinClickedData(*this, pin));
            }
        }
    }
}

void MapController::SetPinBubbleResolver(shared_ptr<PinBubbleResolver> bubbleResovler)
{
    if(mMapView && bubbleResovler)
    {
        mMapView->SetCustomPinBubbleResolver(bubbleResovler);
    }
}

void MapController::SetMapListener(MapListener* listener)
{
    mMapListener = listener;
}

void
MapController::Prefetch(shared_ptr<vector<pair<double, double> > > polylineList,
        double prefetchExtensionLengthMeters, double prefetchExtensionWidthMeters, float zoomLevel)
{
    if(mMapView)
    {
        mMapView->Prefetch(polylineList, prefetchExtensionLengthMeters,
                prefetchExtensionWidthMeters, zoomLevel);
    }
}

void
MapController::Prefetch(double lat, double lon, float zoomLevel, float heading, float tilt)
{
    if(mMapView)
    {
        mMapView->Prefetch(lat, lon, zoomLevel, heading, tilt);
    }
}

const Projection* MapController::GetDefaultProjection() const
{
    return mProjection;
}

bool MapController::GetCamera(double &latitude, double &longitude, float &zoomLevel, float &heading, float &tilt)
{
    bool ret = false;
    if(mMapView)
    {
        MapViewCameraParameter para;
        if(mMapView->GetCamera(para) == NE_OK)
        {
            latitude = para.latitude;
            longitude = para.longitude;
            zoomLevel = para.zoomLevel;
            heading = para.heading;
            tilt = para.tilt;
            ret = true;
        }
    }
    return ret;
}

void
MapController::GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const
{
    if(mMapView)
    {
        mMapView->GetCompassBoundingBox(leftBottomX, leftBottomY, rightTopX, rightTopY);
    }
}

/* See description in header file. */
void MapController::OnLayerUpdate()
{
    // this get called in CCC thread, and layer operation should always in CCC thread.
    if (mMapListener)
    {
        mMapListener->OnOptionalLayerUpdated(GetOptionalLayers());
    }
}

/* See description in header file. */
std::vector<LayerAgentPtr> MapController::GetOptionalLayers()
{
    return mOptionalLayersAgent ? mOptionalLayersAgent->GetOptionalLayerAgents() :
            std::vector<LayerAgentPtr>();
}

void
MapController::StopAnimations(int type)
{
    if(mMapView)
    {
        mMapView->StopAnimations(type);
    }
}

void
MapController::SetBackground(bool backGround)
{
    if(mMapView)
    {
        mMapView->SetBackground(backGround);
    }
}

void
MapController::SetBackgroundSynchronized(bool background)
{
    if(mMapView)
    {
        mMapView->SetBackgroundSynchronized(background);
    }
}

/* See description in header file. */
void MapController::CCC_Thread_CreateMap(PAL_Instance* pal, void* userData)
{
    CreateMapParameters* param = static_cast<CreateMapParameters*>(userData);
    if (param)
    {
        param->mController.CreateMapViewInstance(param->mConfig, param->mViewContext);
        delete param;
    }
}

/* See description in header file. */
void MapController::CreateMapViewInstance(const MapViewConfiguration& config,
                                          void* mapViewContext)
{
    if(config.cleanMetadataFlag)
    {
        CleanMetaData(config.workFolder);
    }
    shared_ptr<MapServicesConfiguration> mapService = MapServicesConfiguration::GetInstance();
    mapService->SetPreferredLanguageCode(get_language_code(config.languageCode.c_str()));

    PAL_Instance *pal = mPal;
    // Initiallize http download manager

    shared_ptr<AsyncCallback<void*> > initializeCallback( new MapInitializeCallback());
    shared_ptr<AsyncCallback<void*> > destroyCallback(new MapDestroyCallback());
    shared_ptr<nbmap::MetadataConfiguration> metadataConfigPtr = GetMetaDataConfiguration();
    if(metadataConfigPtr)
    {
        if(!config.productClass.empty())
        {
            //Actually, if the product class is empty, we will use the default value.
            //The default value already be set when MetadataConfiguration construction.
            metadataConfigPtr->m_productClass = shared_ptr<string>(new string(config.productClass));
        }

        metadataConfigPtr->m_wantOptionalLayers  = true;
        metadataConfigPtr->m_want2DBuildings     = true;
        metadataConfigPtr->m_wantDAM             = true;
        metadataConfigPtr->m_wantLod             = true;
        metadataConfigPtr->m_wantLoc             = true;
        metadataConfigPtr->m_wantWeatherLayer    = true;
        metadataConfigPtr->m_wantPoiLayers       = true;
        metadataConfigPtr->m_wantLableLayers     = true;
        metadataConfigPtr->m_wantSatelliteLayers = true;
        metadataConfigPtr->m_pLanguage = shared_ptr<string>(new string(config.languageCode));
    }
    mCustomLayerManager = shared_ptr<CustomLayerManagerImpl>(new CustomLayerManagerImpl(mNBContext));
    MapConfiguration mapConfig(mPinManager, mCustomLayerManager, mOptionalLayersAgent);
    mapConfig.initializeCallback = initializeCallback;
    mapConfig.destroyCallback = destroyCallback;
    mapConfig.defaultLatitude = config.defaultLatitude;
    mapConfig.defaultLongitude = config.defaultLongitude;
    mapConfig.defaultTiltAngle = config.defaultTiltAngle;
    mapConfig.maximumCachingTileCount = config.maximumCachingTileCount;
    mapConfig.maximumTileRequestCountPerLayer = config.maximumTileRequestCountPerLayer;
    mapConfig.workFolder = config.workFolder.c_str();
    mapConfig.resourceFolder = config.resourceFolder.c_str();
    mapConfig.metadataConfig = metadataConfigPtr;
    mapConfig.zorderLevel = config.zorderLevel;
    mapConfig.mapViewContext = mapViewContext;
    mapConfig.mapViewListener = mDefaultMapViewListener;
    mapConfig.fontMagnifierLevel = config.fontMaginfierLevel;
    mapConfig.scaleFactor = config.scaleFactor;

    mMapView = mapService->GetMap(mNBContext, mapConfig);


    if (mPinManager)
    {
        char pathChar[MAX_PATH_LENGTH] = {0};
        char fullPath[MAX_PATH_LENGTH] = {0};

        // Append filename of pin material config information to the resource folder.
        nsl_memset(pathChar, 0, MAX_PATH_LENGTH);
        nsl_strlcpy(pathChar,config.resourceFolder.c_str(), MAX_PATH_LENGTH);

        PAL_Error palError = PAL_FileAppendPath(pal, pathChar, MAX_PATH_LENGTH, PIN_MATERIAL_CONFIG_FILENAME);
        if (palError != PAL_Ok)
        {
            // @todo: We probably need to add some callbacks to the MapView user to report any errors
            return;
        }

        PAL_FileFullPath(fullPath, MAX_PATH_LENGTH, pathChar);

        shared_ptr<string> pinMaterialConfigPath(new string(fullPath));
        if (!pinMaterialConfigPath)
        {
            return;
        }
        mPinManager->SetMaterialConfigPath(pinMaterialConfigPath);

        shared_ptr<string> pinLayerID(new string(CUSTOM_PIN_LAYER_ID));
        mPinLayer = mPinManager->AddPinLayer(pinLayerID);
    }

    mProjection = new Projection(mMapView);

    mEnableSSAO = config.enableSSAO;
    mEnableGlow = config.enableGlow;
}

/* See description in header file. */
void MapController::CCC_Thread_ShowTraffic(PAL_Instance* pal, void* userData)
{

    ShowTrafficParameter* param = static_cast<ShowTrafficParameter*>(userData);
    if (param)
    {
#ifdef DTRAFFIC
        qWarning("TRAFFIC: 5. MapController::CCC_Thread_ShowTraffic(%s)", param->m_enabled?"true":"false");
#endif
        param->m_controller.CCC_ShowTraffic(param->m_enabled);
        delete param;
    }

}

/* See description in header file. */
void MapController::CCC_ShowTraffic(bool enabled)
{
#ifdef DTRAFFIC
    qWarning("TRAFFIC: 5. MapController::CCC_ShowTraffic(%s)", enabled?"true":"false");
#endif

    if(mMapView)
    {
        if (mOptionalLayersAgent)
        {
            if (enabled)
            {
                mOptionalLayersAgent->EnableTraffic();
            }
            else
            {
                mOptionalLayersAgent->DisableTraffic();
            }
        }
    }
}

/* See description in header file. */
void MapController::PlayDoppler()
{
    if(mMapView)
    {
        mMapView->PlayDoppler();
    }
}

/* See description in header file. */
void MapController::PauseDoppler()
{
    if(mMapView)
    {
        mMapView->PauseDoppler();
    }
}

void MapController::OnAvatarClicked(double lat, double lon)
{
    if (mMapListener)
    {
        mMapListener->OnAvatarClicked(lat, lon);
    }
}

MapCircle* MapController::AddCircle(double lat, double lon, float radius, int fillColor, int strokeColor, int zOrder, bool visible)
{
    MapCircle *circle = NULL;
    if(mMapView)
    {
        circle = new MapCircle(*this, mNextModelId++, lat, lon, radius, fillColor, strokeColor, zOrder, visible);
        if(circle != NULL)
        {
            mMapView->AddCircle(circle->GetId(), lat, lon, radius, fillColor, strokeColor, zOrder, visible);
            mCircleSet.insert(shared_ptr<MapCircle>(circle));
        }
    }
    return circle;
}

void MapController::RemoveCircle(MapCircle* circle)
{
    if(mMapView && circle)
    {
        circle->Remove();
        set<shared_ptr<MapCircle> >::iterator it = find_if(mCircleSet.begin(), mCircleSet.end(), FindByPointer<MapCircle>(circle));
        if(it != mCircleSet.end())
        {
            mCircleSet.erase(it);
        }
    }
}

void MapController::RemoveAllCircles()
{
    mCircleSet.clear();
    if(mMapView)
    {
        return mMapView->RemoveAllCircles();
    }
}

void MapController::SetReferenceCenter(float x, float y)
{
    if(mMapView)
    {
        mMapView->SetReferenceCenter(x, y);
    }

}

void MapController::EnableReferenceCenter(nb_boolean enable)
{
    if(mMapView)
    {
        mMapView->EnableReferenceCenter(enable);
    }
}

MapRect2d* MapController::AddRect2d(double lat, double lon, float heading, float width, float height, int texId, nbcommon::DataStreamPtr texData, bool visible)
{
    MapRect2d *rect = NULL;
    if(mMapView)
    {
        rect = new MapRect2d(*this, mNextModelId++, lat, lon, heading, width, height, visible);
        if(rect != NULL)
        {
            mMapView->AddRect2d(rect->GetId(), lat, lon, heading, width, height, texId, texData, visible);
            mRect2dSet.insert(shared_ptr<MapRect2d>(rect));
        }
    }
    return rect;
}

void MapController::RemoveRect2d(MapRect2d* rect)
{
    if(mMapView && rect)
    {
        set<shared_ptr<MapRect2d> >::iterator it = find_if(mRect2dSet.begin(), mRect2dSet.end(), FindByPointer<MapRect2d>(rect));
        if(it != mRect2dSet.end())
        {
            rect->Remove();
            mRect2dSet.erase(it);
        }
    }
}

void MapController::RemoveAllRect2ds()
{
    mRect2dSet.clear();
    if(mMapView)
    {
        mMapView->RemoveAllRect2ds();
        return;
    }
}

void MapController::AddTexture(int textureId, nbcommon::DataStreamPtr textureData)
{
    if(mMapView && textureId != 0 && textureData)
    {
        mMapView->AddTexture(textureId, textureData);
    }
}

void MapController::RemoveTexture(int textureId)
{
    if(mMapView)
    {
        mMapView->RemoveTexture(textureId);
    }
}

void MapController::RemoveAllTextures()
{
    if(mMapView)
    {
        mMapView->RemoveAllTextures();
    }
}

void MapController::OnMapCreate()
{
    if(mMapListener)
    {
        mMapListener->OnMapCreate();
    }
}

void MapController::OnMapReady()
{
    HBAOParameters hbaoParams;
    hbaoParams.enable = mEnableSSAO;
    SetHBAOParameters(hbaoParams);

    GlowParameters glowParams;
    glowParams.enable = mEnableGlow;
    SetGlowParameters(glowParams);

    if(mMapListener)
    {
        mMapListener->OnMapReady();
    }
}

void MapController::OnCameraAnimationDone(int animationId, MapViewAnimationStatusType animationStatus)
{
    if(mMapListener)
    {
        mMapListener->OnCameraAnimationDone(animationId, animationStatus);
    }
}

void MapController::OnNightModeChanged(bool isNightMode)
{
    if(mMapListener)
    {
        mMapListener->OnNightModeChanged(isNightMode);
    }
}

void MapController::OnTap(double latitude, double longitude)
{
    if(mMapListener)
    {
        mMapListener->OnTap(latitude, longitude);
    }
}

void MapController::OnCameraLimited(MapViewCameraLimitedType type)
{
    if(mMapListener)
    {
        mMapListener->OnCameraLimited(type);
    }
}

void MapController::MasterClear()
{
    if(mMapView == NULL)
    {
        return;
    }

    PAL_Event* ev = NULL;
    if (PAL_EventCreate(mPal, &ev) == PAL_Ok)
    {
        // Post task to CCC thread
        uint32 taskId = 0;
        PAL_Error error = PAL_EventTaskQueueAdd(mPal,
            MapController::CCC_MasterClear,
            new MasterClearParameters(mNBContext, ev),
            &taskId);
        if (error!= PAL_Ok)
        {
            // Wait for the event to finish master clear.
            PAL_EventWaitForEvent(ev);
        }
        PAL_EventDestroy(ev);
    }
}

void MapController::CCC_MasterClear(PAL_Instance* pal, void* userData)
{
    MasterClearParameters* param = static_cast<MasterClearParameters*>(userData);
    if (param)
    {
        NB_ContextSendMasterClear(param->mContext);
        PAL_Event* ev = param->mEvent;
        delete param;
        PAL_EventSet(ev);
    }
}

void MapController::OnStaticPOIClicked(const std::string& id, const std::string& name, double lat, double lon)
{
    if (mMapListener)
    {
        mMapListener->OnStaticPOIClicked(id, name, lat, lon);
    }
}

void MapController::OnTrafficIncidentPinClicked(double lat, double lon)
{
    if (mMapListener)
    {
        mMapListener->OnTrafficIncidentPinClicked(lat, lon);
    }
}

void MapController::DestroyCustomTile(MapTile* tile)
{
    if(mMapView && tile)
    {
        set<MapTile*>::iterator it = mTileSet.find(tile);
        if(it == mTileSet.end())
        {
            return;
        }
        uint32 taskId = 0;
        PAL_EventTaskQueueAdd(mPal,
                              MapController::DestroyTileFunc,
                              new DestroyTileData(*this, tile),
                              &taskId);
        mTileSet.erase(it);
    }
}

MapTile* MapController::CreateCustomTile(int x, int y, int zoom, vector<PinParameters<BubbleInterface> >& parameters)
{
    if(!mMapView || parameters.empty())
    {
        return NULL;
    }

    MapTile* tile = new MapTile(*mPal, *this);
    if(tile == NULL)
    {
        return NULL;
    }

    CreateTileData *data = new CreateTileData(*this, tile, x, y, zoom);
    if(data == NULL)
    {
        delete tile;
        return NULL;
    }

    data->mPinParamerts.swap(parameters);
    mTileSet.insert(tile);
    uint32 taskId = 0;
    PAL_EventTaskQueueAdd(mPal,
                          MapController::CreateTileFunc,
                          data,
                          &taskId);
    return tile;
}

void MapController::CreateTileFunc(PAL_Instance* pal, void* userData)
{
    CreateTileData *param = static_cast<CreateTileData*>(userData);
    if(!pal || !param)
    {
       return;
    }

    MapController &mapCotroller = param->mMap;
    MapTile* tile = param->mTile;

    if(!tile)
    {
       delete param;
       return;
    }

    TileFactoryPtr factoryPtr = ContextBasedSingleton<TileFactory>::getInstance(mapCotroller.mNBContext);
    if(factoryPtr)
    {
        CustomNBMTile* tileData = NULL;
        NB_Error err = factoryPtr->GenerateCustomLayerTileData(TileKey(param->mX, param->mY, param->mZoom), param->mPinParamerts, tileData);
        if(err == NE_OK && tileData != NULL)
        {
            tile->SetTileData(CustomNBMTilePtr(tileData));
        }
    }
     delete param;
}

void MapController::DestroyTileFunc(PAL_Instance* pal, void* userData)
{
    DestroyTileData *data = static_cast<DestroyTileData*>(userData);
    if(!pal || !data)
    {
       return;
    }

    MapTile* tile = data->m_tile;
    delete data;

    if(!tile)
    {
       return;
    }
    delete tile;
}

MapLayer* MapController::AddCustomLayer(TileProviderPtr provider, int mainOrder, int subOrder, int refZoom, int minZoom, int maxZoom, bool visible)
{
    if(!mMapView || !mCustomLayerManager || !provider)
    {
        return NULL;
    }

    MapLayer* layer =  new MapLayer(*mPal, *this, visible);
    if(layer == NULL)
    {
        return NULL;
    }

    AddLayerData *data = new AddLayerData(*this, layer);
    if(data == NULL)
    {
        delete layer;
        return NULL;
    }

    data->mProvider = provider;
    data->mMainOrder = mainOrder;
    data->mSubOrder = subOrder;
    data->mRefZoom = refZoom;
    data->mMinZoom = minZoom;
    data->mMaxZoom = maxZoom;
    data->mVisible = visible;

    mLayerSet.insert(layer);
    uint32 taskId = 0;
    PAL_EventTaskQueueAdd(mPal,
                          MapController::AddLayerFunc,
                          data,
                          &taskId);

    return layer;
}

void MapController::AddLayerFunc(PAL_Instance* pal, void* userData)
{
    AddLayerData *param = static_cast<AddLayerData*>(userData);
    if(!pal || !param)
    {
       return;
    }

    MapController &mapCotroller = param->mMap;
    MapLayer* layer = param->mLayer;

    if(!layer)
    {
       delete param;
       return;
    }

    if(mapCotroller.mCustomLayerManager)
    {
        uint32 layerId = mapCotroller.mCustomLayerManager->AddCustomLayer(param->mProvider, param->mMainOrder, param->mSubOrder, param->mRefZoom, param->mMinZoom, param->mMaxZoom, param->mVisible);
        if(layerId != 0)
        {
            layer->SetLayerId(layerId);
        }
    }
    delete param;
}

void MapController::RemoveLayerFunc(PAL_Instance* pal, void* userData)
{
    RemoveLayerData *param = static_cast<RemoveLayerData*>(userData);
    if(!pal || !param)
    {
       return;
    }

    MapController &mapCotroller = param->mMap;
    MapLayer* layer = param->mLayer;
    delete param;

    if(!layer)
    {
       return;
    }

    uint32 layerId = layer->GetLayerId();
    if(mapCotroller.mCustomLayerManager && layerId != 0)
    {
        mapCotroller.mCustomLayerManager->RemoveCustomLayer(layerId);
    }

    delete layer;
}

void MapController::EnableLayerCollisionFunc(PAL_Instance* pal, void* userData)
{
    EnableLayerCollisionData *param = static_cast<EnableLayerCollisionData*>(userData);
    if(!pal || !param)
    {
        return;
    }

    MapController &mapCotroller = param->mMap;
    MapLayer* layer = param->mLayer;

    if(!layer)
    {
        delete param;
        return;
    }

    uint32 layerId = layer->GetLayerId();
    char strBuf[20];
    sprintf(strBuf, "%d", layerId);
    mapCotroller.mMapView->EnableCustomLayerCollisionDetection(strBuf, param->mEnable);
    delete param;
}

void MapController::RemoveCustomLayer(MapLayer* layer)
{
    if(mMapView && mCustomLayerManager && layer)
    {
        set<MapLayer*>::iterator it = mLayerSet.find(layer);
        if(it == mLayerSet.end())
        {
            return;
        }

        MapLayer* layer = *it;
        if(layer != NULL)
        {
            uint32 taskId = 0;
            PAL_EventTaskQueueAdd(mPal,
                                  MapController::RemoveLayerFunc,
                                  new RemoveLayerData(*this, layer),
                                  &taskId);
        }
        mLayerSet.erase(it);
    }
}

void MapController::RemoveAllCustomLayers()
{
    //@todo
}

void MapController::RemoveAllCustomTiles()
{
    //@todo
}

void MapController::OnMarkerClicked(int id)
{
    if (mMapListener)
    {
        mMapListener->OnMarkerClicked(id);
    }
}

void MapController::SetFontScale(float scale)
{
    if (mMapView)
    {
        mMapView->SetFontScale(scale);
    }
}

void MapController::SetAvatarScale(float scale)
{
    if (mMapView)
    {
        mMapView->SetAvatarScale(scale);
    }
}

void MapController::OnLongPressed(double lat, double lon)
{
    if (mMapListener)
    {
        mMapListener->OnLongPressed(lat, lon);
    }
}

void MapController::OnGesture(MapViewGestureType type, MapViewGestureState state, int time)
{
    if (mMapListener)
    {
        mMapListener->OnGesture(type, state, time);
    }
}

void MapController::SnapShot(shared_ptr<SnapShotCallback> callback, int x, int y, int width, int height)
{
    shared_ptr<DefaultGenerateMapCallback> defaultCallback = shared_ptr<DefaultGenerateMapCallback>(new DefaultGenerateMapCallback(callback));
    if(mMapView)
    {
        mMapView->GenerateMapImage(defaultCallback, x, y, width, height);
    }
}

void MapController::CleanMetaData(const std::string& workPath)
{
    if(workPath.empty())
    {
        return;
    }

    char metaDatafilePath[MAX_PATH_LENGTH] = {0};
    nsl_strlcpy(metaDatafilePath, workPath.c_str(), MAX_PATH_LENGTH);

    // Generate the full path of metedata.
    PAL_Error result = PAL_FileAppendPath(mPal, metaDatafilePath, MAX_PATH_LENGTH, METADATA_FILE_NAME);
    if (result == PAL_Ok)
    {
        // Check if this path exists.
        if(PAL_FileExists(mPal, metaDatafilePath) == PAL_Ok)
        {
            // Remove this file without chekcing returned error.
            PAL_FileRemove(mPal, metaDatafilePath);
        }
    }
}

shared_ptr<MetadataConfiguration> MapController::GetMetaDataConfiguration()
{
    if(mMetadataConfigPtr == NULL)
    {
        mMetadataConfigPtr = shared_ptr<MetadataConfiguration>(new MetadataConfiguration);
    }
    return mMetadataConfigPtr;
}

void MapController::SetHBAOParameters(const HBAOParameters& parameters)
{
    if(mMapView)
    {
        mMapView->SetHBAOParameters(parameters);
    }
}

void MapController::SetGlowParameters(const GlowParameters& parameters)
{
    if(mMapView)
    {
        mMapView->SetGlowParameters(parameters);
    }
}

void MapController::SetDisplayScreen(int screenIndex)
{
    if(mMapView)
    {
        mMapView->SetDisplayScreen(screenIndex);
    }
}

void MapController::SetDPI(float dpi)
{
    if(mMapView)
    {
        mMapView->SetDPI(dpi);
    }
}

