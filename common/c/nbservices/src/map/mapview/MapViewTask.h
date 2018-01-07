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
   @file        MapViewTask.h
   @defgroup    nbmap

   Description: Different tasks used by MapView to put into EventTaskQueue.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

#ifndef _MAPVIEWTASK_H_
#define _MAPVIEWTASK_H_

#include "palevent.h"
#include "paltaskqueue.h"
#include "nbtaskqueue.h"
#include "Tile.h"
#include "Layer.h"
#include "Pin.h"
#include "MapView.h"
#include "MapNativeView.h"
#include "nbgmtypes.h"
#include "MapViewAnimationLayer.h"
#include <sstream>
#include "MapViewTileCallback.h"
#include "ExternalMaterial.h"
#include "PinParameters.h"
/*
    Disable warning C4512: "assignment operator could not be generated"
    This is due to the 'const' members of some of the classes. Assignment operators are not needed for these classes.
*/
#pragma warning(disable:4512)

namespace nbcommon
{
class DataStream;
}

namespace nbmap
{

class MapViewTask;
class MapLegendInfo;
class TileWrapper;

typedef void (MapView::*TaskFunction)(const MapViewTask* pTask);

/*! Basic used to add to task queue by MapView, refer to class Task for more description */
class MapViewTask : public Task
{
public:
    MapViewTask(MapView* handler, TaskFunction function);
    virtual ~MapViewTask();

    // Refer to class Task for description.
    virtual void Execute(void);

protected:
    shared_ptr<bool> m_isValid;         /*!< Flag to indicate if MapView instance is valid. */

private:
    MapView*         m_handler;
    TaskFunction     m_function;
};

/*! Task used during initialization */
class MapViewTaskInitialize : public MapViewTask
{
public:
    MapViewTaskInitialize(MapView* handler, TaskFunction function, NB_Error result)
    : MapViewTask(handler, function), m_result(result){};
    virtual ~MapViewTaskInitialize(){};

    /*! Result from MapView initialization */
    NB_Error m_result;
};

/*! Task used by MapView to unload tiles. */
class MapViewTaskUnloadTiles : public MapViewTask
{
public:
    MapViewTaskUnloadTiles(MapView* handler, TaskFunction function);
    virtual ~MapViewTaskUnloadTiles(){};

    vector<TilePtr> m_tilesToUnload; /*! Data used for RenderThread_UnloadTilesToNBGM */

};

/*! Task used by MapView to load a tiles. */
class MapViewTaskLoadTiles : public MapViewTask
{
public:
    MapViewTaskLoadTiles(MapView* handler, TaskFunction function);
    virtual ~MapViewTaskLoadTiles(){};

    vector<shared_ptr<TileWrapper> > m_tilesToLoad; /*! Data used for RenderThread_LoadTilesToNBGM */
};

class MapViewTaskUpdateCommonMaterial : public MapViewTask
{
public:
    MapViewTaskUpdateCommonMaterial(MapView* handler, TaskFunction function);
    virtual ~MapViewTaskUpdateCommonMaterial(){};

    std::vector<TilePtr> m_tileToLoad; /*! Data used for LoadingThread_UpdateCommonMaterial */
    bool isDay;
    bool isSatellite;
};

/*! Task used by MapView to update frame. */
class MapViewTaskUpdateFrame : public MapViewTask
{
public:
    MapViewTaskUpdateFrame(MapView* handler, TaskFunction function);
    virtual ~MapViewTaskUpdateFrame(){};

    // I don't think we need any members here.
};

/*! Task used by MapView to update debug UI. */
class MapViewTaskUpdateDebugUI : public MapViewTask
{
public:
    MapViewTaskUpdateDebugUI(MapView* handler, TaskFunction function) : MapViewTask(handler, function){};
    virtual ~MapViewTaskUpdateDebugUI(){};

    /*! Output for debug UI */
    stringstream m_debugStream;

    /*! Output for prifiling debug file. Only valid when we're in follow-me mode */
    stringstream m_fileStream;

    /*! Only needed the first time we start a new gps file */
    stringstream m_fileHeader;
};

/*! Task used by MapView to show the pin bubble by the pin ID */
class MapViewTaskShowBubbleById : public MapViewTask
{
public:
    MapViewTaskShowBubbleById(MapView* handler,
                              TaskFunction function,
                              shared_ptr<string> pinId,
                              float x,
                              float y,
                              bool orientation);

    virtual ~MapViewTaskShowBubbleById(){};

    shared_ptr<string> m_pinId;     /*!< The pin ID to show the bubble */
    float m_x;                      /*!< The position of X, the whole length of
                                         X is 1, so it is a percent. */
    float m_y;                      /*!< The position of Y */
    bool m_orientation;             /*!< ??
                                     @todo: I checked the code in nbui of iphone.
                                     It is not used now.
                                    */
};

/*! Task used by MapView to hide the pin bubble by the pin ID */
class MapViewTaskHideBubbleById : public MapViewTask
{
public:
    MapViewTaskHideBubbleById(MapView* handler,
                              TaskFunction function,
                              shared_ptr<string> pinId);

    virtual ~MapViewTaskHideBubbleById(){};

    shared_ptr<string> m_pinId;        /*!< A pin ID to hide the bubble */
};

/*! Task used by MapView to show the pin bubble by the pin */
class MapViewTaskShowBubbleByPin : public MapViewTask
{
public:
    MapViewTaskShowBubbleByPin(MapView* handler,
                               TaskFunction function,
                               PinPtr pin,
                               float x,
                               float y,
                               bool orientation);

    virtual ~MapViewTaskShowBubbleByPin(){};

    PinPtr m_pin;           /*!< Pin to show the bubble */
    float m_x;              /*!< The position of X, the whole length of
                                 X is 1, so it is a percent. */
    float m_y;              /*!< The position of Y */
    bool m_orientation;     /*!< ??
                                 @todo: I checked the code in nbui of iphone.
                                        It is not used now.
                            */
};

/*! Task used by MapView to hide the pin bubble by the pin */
class MapViewTaskHideBubbleByPin : public MapViewTask
{
public:
    MapViewTaskHideBubbleByPin(MapView* handler,
                               TaskFunction function,
                               PinPtr pin);

    virtual ~MapViewTaskHideBubbleByPin(){};

    PinPtr m_pin;   /*!< Pin to hide the bubble */
};

/*! Task used by MapView to remove the pin bubble */
class MapViewTaskRemovePinBubble : public MapViewTask
{
public:
    MapViewTaskRemovePinBubble(MapView* handler,
                               TaskFunction function,
                               shared_ptr<vector<PinPtr> > pins);

    virtual ~MapViewTaskRemovePinBubble(){};

    shared_ptr<vector<PinPtr> > m_pins;        /*!< The pins to remove the bubble */
};


/*! Task used by MapView to show the bubble*/
class MapViewTaskShowBubble : public MapViewTask
{
public:
    MapViewTaskShowBubble(MapView* handler,
                               TaskFunction function,
                               shared_ptr<BubbleInterface> bubble,
                               float x,
                               float y,
                               bool orientation);

    virtual ~MapViewTaskShowBubble(){};

    shared_ptr<BubbleInterface> m_bubble;           /*!< Bubble to show */
    float m_x;              /*!< The position of X, the whole length of
                                 X is 1, so it is a percent. */
    float m_y;              /*!< The position of Y */
    bool m_orientation;     /*!< ??
                                 @todo: I checked the code in nbui of iphone.
                                        It is not used now.
                            */
};


/*! Task used by MapView to show the static poi bubble */
class MapViewTaskShowStaticPoiBubble : public MapViewTask
{
public:
    MapViewTaskShowStaticPoiBubble(MapView* handler,
                               TaskFunction function,
                               float x,
                               float y,
                               bool orientation);

    virtual ~MapViewTaskShowStaticPoiBubble(){};

    float m_x;              /*!< The position of X, the whole length of
                                 X is 1, so it is a percent. */
    float m_y;              /*!< The position of Y */
    bool m_orientation;     /*!< ??
                                 @todo: I checked the code in nbui of iphone.
                                        It is not used now.
                            */
};

/*! Task used by MapView to hide the pin bubble by the pin */
class MapViewTaskHideStaticPoiBubble : public MapViewTask
{
public:
    MapViewTaskHideStaticPoiBubble(MapView* handler,
                               TaskFunction function);

    virtual ~MapViewTaskHideStaticPoiBubble(){};
};

/*! Task used by MapView to hide the bubble*/
class MapViewTaskHideBubble : public MapViewTask
{
public:
    MapViewTaskHideBubble(MapView* handler,
                               TaskFunction function,
                               shared_ptr<BubbleInterface> bubble);

    virtual ~MapViewTaskHideBubble(){};

    shared_ptr<BubbleInterface> m_bubble;   /*!< Bubble to hide */
};

/*! Task used by MapView to remove the bubble */
class MapViewTaskRemoveBubble : public MapViewTask
{
public:
    MapViewTaskRemoveBubble(MapView* handler,
                               TaskFunction function,
                               shared_ptr<vector<shared_ptr<BubbleInterface> > > bubbles);

    virtual ~MapViewTaskRemoveBubble(){};

    shared_ptr<vector<shared_ptr<BubbleInterface> > > m_bubbles;        /*!< The bubbles to remove */
};

class MapViewTaskShowMapLegend : public MapViewTask
{
public:
    MapViewTaskShowMapLegend(MapView* handler,
                             TaskFunction function,
                             shared_ptr<MapLegendInfo> mapLegend);

    virtual ~MapViewTaskShowMapLegend(){};

    shared_ptr<MapLegendInfo> m_pMapLegend;
};

class MapViewTaskDropPin : public MapViewTask
{
public:
    MapViewTaskDropPin(MapView* handler, TaskFunction function, double latitude, double longitude);
    virtual ~MapViewTaskDropPin(){};

    double m_latitude;
    double m_longitude;
};

class MapViewTaskAddStaticPoiBubble : public MapViewTask
{
public:
    MapViewTaskAddStaticPoiBubble(MapView* handler, TaskFunction function, const string& id, const string& name, double m_latitude, double m_longtitude);
    virtual ~MapViewTaskAddStaticPoiBubble(){};

    string m_id;
    string m_name;
    double m_latitude;
    double m_longtitude;
};


/*! Task is used to add/remove animation layers */
class MapViewTaskAnimationLayer : public MapViewTask
{
public:
    MapViewTaskAnimationLayer(MapView* handler, TaskFunction function);
    virtual ~MapViewTaskAnimationLayer(){};

    /*! Layer to add/remove/update. Only one layer is valid at a time. */
    shared_ptr<Layer> m_layerAdded;
    shared_ptr<Layer> m_layerRemoved;
    shared_ptr<Layer> m_layerUpdated;

    /* Only valid when we update the layer with a new frame list. Contains timestamps, sorted. */
    vector<uint32> m_sortedFrameList;
};

/*! Task to handle add/remove/update animation layers on the UI Thread */
class MapViewTaskUIAnimationLayer : public MapViewTask
{
public:
    MapViewTaskUIAnimationLayer(MapView* handler, TaskFunction function, bool createLabel, bool removeLabel, uint32 timestamp, bool animationCanPlay);
    virtual ~MapViewTaskUIAnimationLayer(){};

    /*! Create the timestamp label in the UI when set */
    bool m_createTimestampLabel;

    /*! Remove the timestamp label in the UI when set */
    bool m_removeTimestampLabel;

    /*! Update label with new timestamp, if not zero */
    uint32 m_timestamp;

    /*! Indicate whether it has enough tiles to play animation */
    bool m_animationCanPlay;
};

/*! Task used to show (update) location bubble */
class UITaskUpdateLocationBubble : public MapViewTask
{
public:
    UITaskUpdateLocationBubble(MapView* handler,
                                  TaskFunction function,
                                  float x,         /*!< The position of X, the whole length of
                                                        X is 1, so it is a percent. */
                                  float y,         /*!< The position of Y */
                                  bool orientation,/*!< */
                                  double latitude, /*!< */
                                  double longitude /*!< */
                             );
    virtual ~UITaskUpdateLocationBubble(){};


    float  m_x;
    float  m_y;
    bool   m_orientation;
    double m_latitude;
    double m_longitude;
};

/*! Task used to switch to CCC thread and download specified tiles. */
class MapViewTaskGetSpecifiedTiles : public MapViewTask
{
public:
    MapViewTaskGetSpecifiedTiles(MapView* handler,
                                 TaskFunction function,
                                 const vector<TileKeyPtr>& tileKeys,
                                 const vector<LayerPtr>& layerList,
                                 const vector<LayerPtr>& ignoreList);

    virtual ~MapViewTaskGetSpecifiedTiles() {}

    vector<TileKeyPtr> m_tileKeys;      /*!< List of tile Keys to be downloaded. */
    vector<LayerPtr>   m_layerList;     /*!< List of layers to download tiles.  */
    vector<LayerPtr>   m_ignoreList;    /*!< List of layers that should be ignored  */
};

/*! Task to check and load tiles. */
class MapViewTaskCheckAndLoadTiles : public MapViewTask
{
public:
    MapViewTaskCheckAndLoadTiles(MapView* handler, TaskFunction function,
                                 const TilePtr& tile)
            : MapViewTask(handler, function),
              m_tileToCheck(tile) {}
    virtual ~MapViewTaskCheckAndLoadTiles() {}
    TilePtr m_tileToCheck;
};

/*! Task used to create a screenshot base on current map view*/
class RenderTaskGenerateMapImage : public MapViewTask
{
public:
    RenderTaskGenerateMapImage(void* handler,
                           TaskFunction function,
                           shared_ptr<GenerateMapCallback>  callback,
                           int32 x,
                           int32 y,
                           uint32 width,
                           uint32 height)
      : MapViewTask((MapView*)handler, function),
        m_callback(callback),
        m_x(x), m_y(y), m_width(width), m_height(height){}
    virtual ~RenderTaskGenerateMapImage() {}

    shared_ptr<GenerateMapCallback>  m_callback;
    int32 m_x;
    int32 m_y;
    uint32 m_width;
    uint32 m_height;
};

class UItaskGenerateMapImage : public MapViewTask
{
public:
    UItaskGenerateMapImage(MapView* handler,
                       TaskFunction function,
                       shared_ptr<GenerateMapCallback>  callback,
                       shared_ptr<nbcommon::DataStream> dataStream,
                       uint32 width,
                       uint32 height)
    : MapViewTask(handler, function),
      m_callback(callback),
      m_dataStream(dataStream),
      m_width(width), m_height(height){}

    virtual ~UItaskGenerateMapImage() {}

    shared_ptr<GenerateMapCallback>  m_callback;
    shared_ptr<nbcommon::DataStream> m_dataStream;
    uint32 m_width;
    uint32 m_height;
};

/*! Task used to create an asynchronous screenshot base on current map view*/
class MapViewTaskAsyncGenerateMapImage : public MapViewTask
{
public:
    MapViewTaskAsyncGenerateMapImage(void* handler,
                           TaskFunction function,
                           const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > asyncGenerateMapImageCallback,
                           shared_ptr<bool> screenshotInProgress)
      : MapViewTask((MapView*)handler, function),
        m_pAsyncGenerateMapImageCallback(asyncGenerateMapImageCallback),
        m_pScreenshotInProgress(screenshotInProgress){}
    virtual ~MapViewTaskAsyncGenerateMapImage() {}
    const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > m_pAsyncGenerateMapImageCallback;
    shared_ptr<bool> m_pScreenshotInProgress;
};


/*! Task used to update traffic tip which will be shown on screen */
class UITaskUpdateTrafficTip : public MapViewTask
{
public:
    UITaskUpdateTrafficTip(MapView* handler,
                           TaskFunction function,
                           TrafficTipType type)
      : MapViewTask(handler, function), m_tipType(type) {}
    virtual ~UITaskUpdateTrafficTip() {}

    TrafficTipType m_tipType;
};

/*! Task used to update animation tip which will be shown on screen */
class UITaskUpdateAnimationTip : public MapViewTask
{
public:
    UITaskUpdateAnimationTip(MapView* handler,
                           TaskFunction function,
                           AnimationTipType type)
      : MapViewTask(handler, function), m_tipType(type) {}
    virtual ~UITaskUpdateAnimationTip() {}

    AnimationTipType m_tipType;
};

// Render tasks ...................................................................................................
/*
    @todo: We should rename all other tasks for the render thread to start with "RenderTask"
 */

class RenderTaskSetGpsMode : public MapViewTask
{
public:
    RenderTaskSetGpsMode(MapView* handler, TaskFunction function, NB_GpsMode gpsMode)
        : MapViewTask(handler, function), m_gpsMode(gpsMode){}

    virtual ~RenderTaskSetGpsMode(){};

    const NB_GpsMode m_gpsMode;
};

class RenderTaskSetAvatarState : public MapViewTask
{
public:

    RenderTaskSetAvatarState(MapView* handler, TaskFunction function, bool headingValid)
        : MapViewTask(handler, function), m_headingValid(headingValid){}

    virtual ~RenderTaskSetAvatarState(){};

    const bool m_headingValid;
};

class RenderTaskSetAvatarMode : public MapViewTask
{
public:

    RenderTaskSetAvatarMode(MapView* handler, TaskFunction function, MapViewAvatarMode mode)
        : MapViewTask(handler, function), m_avatarMode(mode){}

    virtual ~RenderTaskSetAvatarMode(){};

    const MapViewAvatarMode m_avatarMode;
};

class RenderTaskSetAvatarLocation : public MapViewTask
{
public:

    RenderTaskSetAvatarLocation(MapView* handler, TaskFunction function, const NB_GpsLocation& gpsLocation)
        : MapViewTask(handler, function), m_gpsLocation(gpsLocation){}
    virtual ~RenderTaskSetAvatarLocation(){};

    const NB_GpsLocation m_gpsLocation;
};

class RenderTaskSetCustomAvatar : public MapViewTask
{
public:

    RenderTaskSetCustomAvatar(MapView* handler,
                                 TaskFunction function,
                                 shared_ptr<nbcommon::DataStream> directionalAvatar,
                                 shared_ptr<nbcommon::DataStream> directionlessAvatar,
                                 int8 directionalCalloutOffsetX,
                                 int8 directionalCalloutOffsetY,
                                 int8 directionlessCalloutOffsetX,
                                 int8 directionlessCalloutOffsetY
                                 )
        : MapViewTask(handler, function),
          m_directionalAvatar(directionalAvatar),
          m_directionlessAvatar(directionlessAvatar),
          m_directionalCalloutOffsetX(directionalCalloutOffsetX),
          m_directionalCalloutOffsetY(directionalCalloutOffsetY),
          m_directionlessCalloutOffsetX(directionlessCalloutOffsetX),
          m_directionlessCalloutOffsetY(directionlessCalloutOffsetY){}

    RenderTaskSetCustomAvatar(MapView* handler,
                                 TaskFunction function,
                                 shared_ptr<std::string> directionalAvatarPath,
                                 shared_ptr<std::string> directionlessAvatarPath,
                                 int8 directionalCalloutOffsetX,
                                 int8 directionalCalloutOffsetY,
                                 int8 directionlessCalloutOffsetX,
                                 int8 directionlessCalloutOffsetY
                                 )
    : MapViewTask(handler, function),
      m_directionalAvatarPath(directionalAvatarPath),
      m_directionlessAvatarPath(directionlessAvatarPath),
      m_directionalCalloutOffsetX(directionalCalloutOffsetX),
      m_directionalCalloutOffsetY(directionalCalloutOffsetY),
      m_directionlessCalloutOffsetX(directionlessCalloutOffsetX),
      m_directionlessCalloutOffsetY(directionlessCalloutOffsetY){}
    virtual ~RenderTaskSetCustomAvatar(){};

    shared_ptr<nbcommon::DataStream> m_directionalAvatar;
    shared_ptr<nbcommon::DataStream> m_directionlessAvatar;
    shared_ptr<std::string> m_directionalAvatarPath;
    shared_ptr<std::string> m_directionlessAvatarPath;
    int8 m_directionalCalloutOffsetX;
    int8 m_directionalCalloutOffsetY;
    int8 m_directionlessCalloutOffsetX;
    int8 m_directionlessCalloutOffsetY;
};

class RenderTaskSetCustomAvatarHaloStyle : public MapViewTask
{
public:

    RenderTaskSetCustomAvatarHaloStyle(MapView* handler, TaskFunction function, shared_ptr<GeographyMaterial> haloStyle)
        : MapViewTask(handler, function), m_haloStyle(haloStyle){}
    virtual ~RenderTaskSetCustomAvatarHaloStyle(){};

    shared_ptr<GeographyMaterial> m_haloStyle;
};

class RenderTaskSetBackground : public MapViewTask
{
public:

    RenderTaskSetBackground(MapView* handler, TaskFunction function, bool background, PAL_Event* event)
        : MapViewTask(handler, function), m_background(background), m_event(event){}

    virtual ~RenderTaskSetBackground(){}

    const bool m_background;
    PAL_Event* m_event;
};

class RenderTaskSetViewSettings : public MapViewTask
{
public:

    RenderTaskSetViewSettings(MapView* handler,
                              TaskFunction function,
                              MapViewSettings viewSetting,
                              bool flag,
                              float portraitFOV,
                              float landscapeFOV)
        : MapViewTask(handler, function),
          m_viewSetting(viewSetting),
          m_flag(flag),
          m_portraitFOV(portraitFOV),
          m_landscapeFOV(landscapeFOV){};

    virtual ~RenderTaskSetViewSettings(){};

    const MapViewSettings m_viewSetting;
    const bool m_flag;
    const float m_portraitFOV;
    const float m_landscapeFOV;
};

class RenderTaskSetCameraSettings : public MapViewTask
{
public:
    RenderTaskSetCameraSettings(MapView* handler,
                                TaskFunction function,
                                double latitude,
                                double longitude,
                                double heading,
                                double zoomLevel,
                                double tilt,
                                bool animated,
                                unsigned int duration,
                                MapViewCameraAnimationAccelertaionType type,
                                int id,
                                uint32 timestamp)
        : MapViewTask(handler, function),
          m_latitude(latitude),
          m_longitude(longitude),
          m_heading(heading),
          m_zoomLevel(zoomLevel),
          m_tilt(tilt),
          m_animated(animated),
          m_duration(duration),
          m_type(type),
          m_id(id),
          m_timestamp(timestamp){};

    virtual ~RenderTaskSetCameraSettings(){};

    const double m_latitude;
    const double m_longitude;
    const double m_heading;
    const double m_zoomLevel;
    const double m_tilt;
    const bool m_animated;
    const unsigned int m_duration;
    const MapViewCameraAnimationAccelertaionType m_type;
    const int m_id;
    const uint32 m_timestamp;
};

class RenderTaskSetCurrentPosition : public MapViewTask
{
public:
    
    RenderTaskSetCurrentPosition(MapView* handler, TaskFunction function, double latitude, double longitude)
      : MapViewTask(handler, function), m_latitude(latitude), m_longitude(longitude){}
    
    virtual ~RenderTaskSetCurrentPosition(){};
    const double m_latitude;
    const double m_longitude;
};
class RenderTaskSetViewPort : public MapViewTask
{
public:
    
    RenderTaskSetViewPort(MapView* handler, TaskFunction function, int width, int height)
      : MapViewTask(handler, function), m_width(width), m_height(height){}
    
    virtual ~RenderTaskSetViewPort(){};
    const int m_width;
    const int m_height;
};

class RenderTaskSetScreenOrientation : public MapViewTask
{
public:
    
    RenderTaskSetScreenOrientation(MapView* handler, TaskFunction function, bool portrait)
      : MapViewTask(handler, function), m_portrait(portrait){}
    
    virtual ~RenderTaskSetScreenOrientation(){};
    const bool m_portrait;
};

/*! Task used to update traffic tip. */
class MapViewTaskUpdateTrafficTip : public MapViewTask
{
public:
    MapViewTaskUpdateTrafficTip(MapView* handler,
                              TaskFunction function,
                              TrafficTipType type)
            : MapViewTask(handler, function),
              m_tipType(type) {}
    virtual ~MapViewTaskUpdateTrafficTip() {}

    TrafficTipType m_tipType;
};
    
/*! Task used to update the opacity for animation layers */
class RenderTaskUpdateOpacity : public MapViewTask
{
public:
    RenderTaskUpdateOpacity(MapView* handler, TaskFunction function, uint32 opacity) : MapViewTask(handler, function), m_opacity(opacity){};
    virtual ~RenderTaskUpdateOpacity(){}
    
    uint32 m_opacity;
};

/*! Task used to update parameters for animation layers */
class RenderTaskUpdateAnimationParameters : public MapViewTask
{
public:
    RenderTaskUpdateAnimationParameters(MapView* handler, TaskFunction function, const AnimationLayerParameters& parameters)
        : MapViewTask(handler, function), m_parameters(parameters){};
    virtual ~RenderTaskUpdateAnimationParameters(){}
    
    const AnimationLayerParameters m_parameters;
};

/*! Task used to destroy the renderer */
class RenderTaskDestroyRenderer : public MapViewTask
{
public:
    RenderTaskDestroyRenderer(MapView* handler,
                              TaskFunction function,
                              PAL_Event* event)
    : MapViewTask(handler, function),
      m_event(event)
    {}

    virtual ~RenderTaskDestroyRenderer()
    {}

    /*!< Event to set after destroying renderer */
    PAL_Event* m_event;
};

class RenderTaskPrepareAsyncGenerateMapImage : public MapViewTask
{
public:
    RenderTaskPrepareAsyncGenerateMapImage(MapView* handler,
                              TaskFunction function,
                              shared_ptr<AsyncGenerateMapImageTileCallback> generateImageTileCallback
                              ) : MapViewTask(handler, function), m_generateImageTileCallback(generateImageTileCallback){}

    virtual ~RenderTaskPrepareAsyncGenerateMapImage()
    {}

    shared_ptr<AsyncGenerateMapImageTileCallback> m_generateImageTileCallback;
};

class MapViewTaskPrepareAsyncGenerateMapImage : public MapViewTask
{
public:
    MapViewTaskPrepareAsyncGenerateMapImage(void* handler,
                           TaskFunction function,
                           shared_ptr<AsyncGenerateMapImageTileCallback> generateImageTileCallback
                           ): MapViewTask((MapView*)handler, function), m_generateImageTileCallback(generateImageTileCallback){}

    virtual ~MapViewTaskPrepareAsyncGenerateMapImage() {}

    shared_ptr<AsyncGenerateMapImageTileCallback> m_generateImageTileCallback;
};


class MapViewTaskNotifyAsyncGenerateMapImageCallback : public MapViewTask
{
public:
    MapViewTaskNotifyAsyncGenerateMapImageCallback(void* handler,
                           TaskFunction function,
                           const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > pAsyncGenerateMapImageCallback,
                           shared_ptr<nbcommon::DataStream> dataStream,
                           NB_Error                         error
                           ): MapViewTask((MapView*)handler, function), m_pAsyncGenerateMapImageCallback(pAsyncGenerateMapImageCallback),
                                                                        m_dataStream(dataStream),
                                                                        m_error(error){}

    virtual ~MapViewTaskNotifyAsyncGenerateMapImageCallback() {}

    const shared_ptr<AsyncCallback<shared_ptr<nbcommon::DataStream> > > m_pAsyncGenerateMapImageCallback;
    shared_ptr<nbcommon::DataStream> m_dataStream;
    NB_Error                         m_error;
};

class MapViewTaskRender : public MapViewTask
{
public:
	MapViewTaskRender(MapView* handler,
                      TaskFunction function,
                      RenderListener* listener
                      ): MapViewTask(handler, function), m_pListener(listener){}

    virtual ~MapViewTaskRender() {}

    RenderListener* m_pListener;

};

class MapViewTaskCalculateNightMode : public MapViewTask
{
public:
    MapViewTaskCalculateNightMode (MapView* handler,
                                   TaskFunction function)
        : MapViewTask((MapView*)handler, function){}
    virtual ~MapViewTaskCalculateNightMode() {}
};

class RenderTaskModifyExternalMaterial : public MapViewTask
{
public:
    typedef enum _ExternalMaterialModifyType
    {
        EMMT_Load = 0,
        EMMT_Unload,
        EMMT_Activate,
    } ExternalMaterialModifyType;

    RenderTaskModifyExternalMaterial(MapView*                   handler,
                                     TaskFunction               function,
                                     ExternalMaterialModifyType type,
                                     const ExternalMaterialPtr& material)
            : MapViewTask(handler, function),
              m_type(type),
              m_material(material){}

    virtual ~RenderTaskModifyExternalMaterial(){}

    ExternalMaterialModifyType m_type;
    ExternalMaterialPtr        m_material;
};

class MapViewTaskSetCustomPinBubbleResolver : public MapViewTask
{
public:
    MapViewTaskSetCustomPinBubbleResolver(void* handler,
                           TaskFunction function,
                           shared_ptr<PinBubbleResolver> resolver
                           ): MapViewTask((MapView*)handler, function),
                              m_resolver(resolver){}

    virtual ~MapViewTaskSetCustomPinBubbleResolver() {}

    shared_ptr<PinBubbleResolver> m_resolver;
};

class MapViewTaskPrefetchForNAV : public MapViewTask
{
public:
    MapViewTaskPrefetchForNAV(void* handler,
                           TaskFunction function,
                           shared_ptr<vector<pair<double, double> > > polylineList,
                           double prefetchExtensionLengthMeters,
                           double prefetchExtensionWidthMeters, float zoomLevel)
                           : MapViewTask((MapView*)handler, function),
                           m_polylineList(polylineList),
                           m_prefetchExtensionLengthMeters(prefetchExtensionLengthMeters),
                           m_prefetchExtensionWidthMeters(prefetchExtensionWidthMeters),
                           m_zoomLevel(zoomLevel){}

    virtual ~MapViewTaskPrefetchForNAV() {}

    shared_ptr<vector<pair<double, double> > > m_polylineList;
    double m_prefetchExtensionLengthMeters;
    double m_prefetchExtensionWidthMeters;
    float m_zoomLevel;
};

class MapViewTaskPrefetch : public MapViewTask
{
public:
    MapViewTaskPrefetch(void* handler,TaskFunction function,
                        double lat, double lon, float zoomLevel, float heading, float tilt)
                        : MapViewTask((MapView*)handler, function),
                          m_lat(lat), m_lon(lon), m_zoomLevel(zoomLevel), m_heading(heading), m_tilt(tilt){}

    virtual ~MapViewTaskPrefetch() {}

    double m_lat;
    double m_lon;
    float  m_zoomLevel;
    float  m_heading;
    float  m_tilt;
};

class CombinedAtomicTask:public MapViewTask
{
public:
    CombinedAtomicTask(MapView* handler, TaskFunction function);
    virtual ~CombinedAtomicTask();

public:
    //has lock in AddTask function to prevent threading issues.
    void AddTask(MapViewTask* task);
    void ExcuteAllTasks() const;
    void ClearAllTasks();

private:
    std::vector<MapViewTask*> m_AtomicTasks; 
};

class NBGMViewController;

class RenderTaskGetElementsAt : public MapViewTask
{
public:
    RenderTaskGetElementsAt(MapView*            handler,
                            NBGMViewController* controller,
                            float               x,
                            float               y,
                            set<string>&        elements,
                            PAL_Event*   event);
    virtual ~RenderTaskGetElementsAt() {}
    virtual void Execute();

    NBGMViewController* m_controller;
    float               m_x;
    float               m_y;
    set<string>&        m_elements;
    PAL_Event*          m_event;
};

class RenderTaskEnableCustomLayerCollisionDetection : public MapViewTask
{
public:
    RenderTaskEnableCustomLayerCollisionDetection(MapView*            handler,
                                                  NBGMViewController* controller,
                                                  const std::string&  layerId,
                                                  bool                enable);
    virtual ~RenderTaskEnableCustomLayerCollisionDetection() {}
    virtual void Execute();

    NBGMViewController* m_controller;
    std::string         m_layerId;
    bool                m_enable;
};

class UITaskInvokeSingleTapListener : public MapViewTask
{
public:
    UITaskInvokeSingleTapListener(MapView*     handler,
                                  TaskFunction function,
                                  float        x,
                                  float        y,
                                  set<string>& elements)
            : MapViewTask(handler, function),
              m_x(x),m_y(y),m_elements(elements) {}
    virtual ~UITaskInvokeSingleTapListener(){}

    float       m_x;
    float       m_y;
    set<string> m_elements;
};

class UITaskInvokeCameraUpdateListener : public MapViewTask
{
public:
    UITaskInvokeCameraUpdateListener(MapView*     handler,
                                    TaskFunction function,
                                    double lat, double lon, float zoomLevel, float heading, float tilt
                                    )
            : MapViewTask(handler, function),
              m_lat(lat), m_lon(lon), m_zoomLevel(zoomLevel), m_heading(heading), m_tilt(tilt){}
    virtual ~UITaskInvokeCameraUpdateListener(){}

    double m_lat;
    double m_lon;
    float  m_zoomLevel;
    float  m_heading;
    float  m_tilt;
};

class CCCTaskInvokePinClickedListener : public MapViewTask
{
public:
    CCCTaskInvokePinClickedListener(MapView*     handler,
                                    TaskFunction function,
                                    const string& pinId
                                    )
            : MapViewTask(handler, function),
              m_pinId(pinId){}
    virtual ~CCCTaskInvokePinClickedListener() {}

    string m_pinId;
};

class CCCTaskInvokeUnselectAllPinsListener : public MapViewTask
{
public:
    CCCTaskInvokeUnselectAllPinsListener(MapView* handler, TaskFunction function)
    : MapViewTask(handler, function){}

    virtual ~CCCTaskInvokeUnselectAllPinsListener() {}
};

class RenderTaskHandleTapGesture : public MapViewTask
{
public:
    RenderTaskHandleTapGesture(MapView*     handler,
                               TaskFunction function,
                               float        x,
                               float        y)
            : MapViewTask(handler, function),
              m_x(x),m_y(y){}
    virtual ~RenderTaskHandleTapGesture(){}

    float m_x;
    float m_y;
};

class MapViewTaskSelectPin : public MapViewTask
{
public:
    MapViewTaskSelectPin(void* handler,
                           TaskFunction function,
                           shared_ptr<string> pinId,
                           bool selected
                           )
            : MapViewTask((MapView*)handler, function),
              m_pinId(pinId),
              m_selected(selected) {}

    virtual ~MapViewTaskSelectPin() {}

    shared_ptr<string> m_pinId;
    bool m_selected;
};

class RenderTaskReloadTiles : public MapViewTask
{
public:
    RenderTaskReloadTiles(MapView*               handler,
                          TaskFunction           function,
                          const vector<TilePtr>& tiles)
            : MapViewTask(handler, function),
              m_tilesToReload(tiles) {}
    virtual ~RenderTaskReloadTiles(){}

    vector<TilePtr> m_tilesToReload;
};

class RenderTaskUpdateDopplerState : public MapViewTask
{
public:
    RenderTaskUpdateDopplerState(void* handler, TaskFunction function, bool pause, uint32 time)
        :MapViewTask((MapView*)handler, function)
        ,m_pause(pause)
        ,m_time(time)
    {
    }

    virtual ~RenderTaskUpdateDopplerState() {}

public:
    bool    m_pause;
    bool    m_resume;
    uint32  m_time;
};

class UITaskAvatarClickedListener : public MapViewTask
{
public:
    UITaskAvatarClickedListener(MapView* handler,
                                TaskFunction function,
                                double lat,
                                double lon)
            : MapViewTask(handler, function),
              m_lat(lat),
              m_lon(lon){}
    virtual ~UITaskAvatarClickedListener() {}

    double m_lat;
    double m_lon;
};

class MapViewTaskAddCircle: public MapViewTask
{
public:
    MapViewTaskAddCircle(MapView* handler,
                         TaskFunction function,
                         int circleId,
                         const NBGM_CircleParameters &para)
            : MapViewTask(handler, function),
              m_circleId(circleId),
              m_circlePara(para) {}
    virtual ~MapViewTaskAddCircle() {}

    int m_circleId;
    NBGM_CircleParameters m_circlePara;
};

class MapViewTaskRemoveCircle: public MapViewTask
{
public:
    MapViewTaskRemoveCircle(MapView* handler,
                         TaskFunction function,
                         int circleId)
            : MapViewTask(handler, function),
              m_circleId(circleId){}
    virtual ~MapViewTaskRemoveCircle() {}

    int m_circleId;
};

class MapViewTaskSetCircleCenter: public MapViewTask
{
public:
    MapViewTaskSetCircleCenter(MapView* handler,
                         TaskFunction function,
                         int circleId,
                         double lat,
                         double lon)
            : MapViewTask(handler, function),
              m_circleId(circleId),
              m_lat(lat),
              m_lon(lon) {}
    virtual ~MapViewTaskSetCircleCenter() {}

    int m_circleId;
    double m_lat;
    double m_lon;
};

class MapViewTaskSetCircleVisible: public MapViewTask
{
public:
    MapViewTaskSetCircleVisible(MapView* handler,
                         TaskFunction function,
                         int circleId,
                         bool visible)
            : MapViewTask(handler, function),
              m_circleId(circleId),
              m_visible(visible) {}
    virtual ~MapViewTaskSetCircleVisible() {}

    int m_circleId;
    bool m_visible;
};

class MapViewTaskSetCircleStyle: public MapViewTask
{
public:
    MapViewTaskSetCircleStyle(MapView* handler,
                         TaskFunction function,
                         int circleId,
                         float radius,
                         int fillClr,
                         int outlineClr)
            : MapViewTask(handler, function),
              m_circleId(circleId),
              m_radius(radius),
              m_fillColor(fillClr),
              m_outlineColor(outlineClr){}
    virtual ~MapViewTaskSetCircleStyle() {}

    int m_circleId;
    float m_radius;
    int m_fillColor;
    int m_outlineColor;
};

class RenderTaskSetReferenceCenter : public MapViewTask
{
public:
    RenderTaskSetReferenceCenter(MapView* handler,
                                TaskFunction function,
                                float x,
                                float y)
            : MapViewTask(handler, function),
              m_x(x),
              m_y(y){}
    virtual ~RenderTaskSetReferenceCenter() {}

    float m_x;
    float m_y;
};


class RenderTaskEnableReferenceCenter : public MapViewTask
{
public:
    RenderTaskEnableReferenceCenter(MapView* handler,
                                    TaskFunction function,
                                    nb_boolean enable)
            : MapViewTask(handler, function),
              m_enable(enable){}
    virtual ~RenderTaskEnableReferenceCenter() {}

    nb_boolean m_enable;
};

class MapViewTaskAddRect2d: public MapViewTask
{
public:
    MapViewTaskAddRect2d(MapView* handler,
                         TaskFunction function,
                         int rectId,
                         double lat,
                         double lon,
                         float heading,
                         float width,
                         float height,
                         int textureId,
                         shared_ptr<nbcommon::DataStream> textureBuffer,
                         bool visible)
            : MapViewTask(handler, function),
              m_rectId(rectId),
              m_lat(lat),
              m_lon(lon),
              m_heading(heading),
              m_width(width),
              m_height(height),
              m_textureId(textureId),
              m_textureData(textureBuffer),
              m_visible(visible){}
    virtual ~MapViewTaskAddRect2d() {}

    int m_rectId;
    double m_lat;
    double m_lon;
    float m_heading;
    float m_width;
    float m_height;
    int m_textureId;
    shared_ptr<nbcommon::DataStream> m_textureData;
    bool m_visible;
};

class MapViewTaskRemoveRect2d: public MapViewTask
{
public:
    MapViewTaskRemoveRect2d(MapView* handler,
                         TaskFunction function,
                         int rectId)
            : MapViewTask(handler, function),
              m_rectId(rectId){}
    virtual ~MapViewTaskRemoveRect2d() {}

    int m_rectId;
};

class MapViewTaskSetRect2dVisible: public MapViewTask
{
public:
    MapViewTaskSetRect2dVisible(MapView* handler,
                         TaskFunction function,
                         int rectId,
                         bool visible)
            : MapViewTask(handler, function),
              m_rectId(rectId),
              m_visible(visible){}
    virtual ~MapViewTaskSetRect2dVisible() {}

    int m_rectId;
    bool m_visible;
};


class MapViewTaskUpdateRect2d: public MapViewTask
{
public:
    MapViewTaskUpdateRect2d(MapView* handler,
                         TaskFunction function,
                         int rectId,
                         double lat,
                         double lon,
                         float heading)
            : MapViewTask(handler, function),
              m_rectId(rectId),
              m_lat(lat),
              m_lon(lon),
              m_heading(heading){}
    virtual ~MapViewTaskUpdateRect2d() {}

    int m_rectId;
    double m_lat;
    double m_lon;
    float m_heading;
};


class MapViewTaskSetRect2dSize: public MapViewTask
{
public:
    MapViewTaskSetRect2dSize(MapView* handler,
                         TaskFunction function,
                         int rectId,
                         float width,
                         float height)
            : MapViewTask(handler, function),
              m_rectId(rectId),
              m_width(width),
              m_height(height){}
    virtual ~MapViewTaskSetRect2dSize() {}

    int m_rectId;
    float m_width;
    float m_height;
};


class MapViewTaskAddTexture: public MapViewTask
{
public:
    MapViewTaskAddTexture(MapView* handler,
                         TaskFunction function,
                         int textureId,
                         shared_ptr<nbcommon::DataStream> data)
            : MapViewTask(handler, function),
              m_textureId(textureId),
              m_textureData(data){}
    virtual ~MapViewTaskAddTexture() {}

    int m_textureId;
    shared_ptr<nbcommon::DataStream> m_textureData;
};

class MapViewTaskRemoveTexture: public MapViewTask
{
public:
    MapViewTaskRemoveTexture(MapView* handler,
                         TaskFunction function,
                         int textureId)
            : MapViewTask(handler, function),
              m_textureId(textureId){}
    virtual ~MapViewTaskRemoveTexture() {}

    int m_textureId;
};

class MapViewTaskCameraAnimationDone: public MapViewTask
{
public:
    MapViewTaskCameraAnimationDone(MapView* handler,
                         TaskFunction function,
                         int id,
                         MapViewAnimationStatusType status)
            : MapViewTask(handler, function),
              m_animationId(id),
              m_animationStatus(status){}
    virtual ~MapViewTaskCameraAnimationDone() {}

    int m_animationId;
    MapViewAnimationStatusType m_animationStatus;
};

class MapViewTaskSetTheme: public MapViewTask
{
public:
    MapViewTaskSetTheme(MapView* handler,
                         TaskFunction function,
                         MapViewTheme theme)
            : MapViewTask(handler, function),
              m_theme(theme){}
    virtual ~MapViewTaskSetTheme() {}

    MapViewTheme m_theme;
};

class MapViewTaskChangeNightMode: public MapViewTask
{
public:
    MapViewTaskChangeNightMode(MapView* handler,
        TaskFunction function,
        bool isNightMode)
        : MapViewTask(handler, function),
        m_isNightMode(isNightMode){}
    virtual ~MapViewTaskChangeNightMode() {}

    bool m_isNightMode;
};

class UITaskStaticPOIClickedListener : public MapViewTask
{
public:
    UITaskStaticPOIClickedListener(MapView* handler,
                                  TaskFunction function,
                                  const string &id,
                                  const string &name,
                                  double lat,
                                  double lon)
            : MapViewTask(handler, function),
              m_id(id),
              m_name(name),
              m_lat(lat),
              m_lon(lon){}
    virtual ~UITaskStaticPOIClickedListener() {}

    string m_id;
    string m_name;
    double m_lat;
    double m_lon;
};

class UITaskTrafficIncidentPinClickedListener : public MapViewTask
{
public:
    UITaskTrafficIncidentPinClickedListener(MapView* handler,
                                            TaskFunction function,
                                            double lat,
                                            double lon)
            : MapViewTask(handler, function),
              m_lat(lat),
              m_lon(lon){}
    virtual ~UITaskTrafficIncidentPinClickedListener() {}


    double m_lat;
    double m_lon;
};

class UITaskOnTap : public MapViewTask
{
public:
    UITaskOnTap(MapView* handler,
        TaskFunction function,
        double latitude,
        double longitude)
        : MapViewTask(handler, function),
        m_latitude(latitude), m_longitude(longitude) {}
    virtual ~UITaskOnTap() {}

    double m_latitude;
    double m_longitude;
};

class UITaskCameraLimted : public MapViewTask
{
public:
    UITaskCameraLimted(MapView* handler,
                       TaskFunction function,
                       MapViewCameraLimitedType type)
    :MapViewTask(handler, function),
    m_type(type){}
    virtual ~UITaskCameraLimted() {}

    MapViewCameraLimitedType m_type;
};

class CCCTaskInvokeMarkerClickedListener: public MapViewTask
{
public:
    CCCTaskInvokeMarkerClickedListener(MapView* handler,
        TaskFunction function,
        const string& id)
        : MapViewTask(handler, function),
        m_markerId(id){}
    virtual ~CCCTaskInvokeMarkerClickedListener() {}

    string m_markerId;
};

class UITaskInvokeMarkerClickedListener : public MapViewTask
{
public:
    UITaskInvokeMarkerClickedListener(MapView* handler,
                                TaskFunction function,
                                int id)
            : MapViewTask(handler, function),
              m_markerId(id){}
    virtual ~UITaskInvokeMarkerClickedListener() {}

    int m_markerId;
};
    
class RenderTaskSetFontScale : public MapViewTask
{
public:
        
    RenderTaskSetFontScale(MapView* handler, TaskFunction function, float scale)
    : MapViewTask(handler, function), m_scale(scale){}
        
    virtual ~RenderTaskSetFontScale(){};
        
    float m_scale;
};

class RenderTaskSetAvatarScale : public MapViewTask
{
public:

    RenderTaskSetAvatarScale(MapView* handler, TaskFunction function, float scale)
        : MapViewTask(handler, function), m_scale(scale){}

    virtual ~RenderTaskSetAvatarScale(){};

    float m_scale;
};

class RenderTaskSelectAvatarAndPinTask : public MapViewTask
{
public:
    RenderTaskSelectAvatarAndPinTask(MapView* handler,
                                    TaskFunction function,
                                    float screenX,
                                    float screenY)
                    :MapViewTask(handler, function),
                     m_screenX(screenX),
                     m_screenY(screenY) {}
    virtual ~RenderTaskSelectAvatarAndPinTask() {}

    float m_screenX;
    float m_screenY;
};

class RenderTaskUpdateDopplerByGesture : public MapViewTask
{
public:
    RenderTaskUpdateDopplerByGesture(MapView* handler,
                                    TaskFunction function,
                                    bool needUpdate)
                    :MapViewTask(handler, function),
                    m_needUpdate(needUpdate) {}
    virtual ~RenderTaskUpdateDopplerByGesture() {}

    bool m_needUpdate;
};

class RenderTaskSetGestureProcessing : public MapViewTask
{
public:
    RenderTaskSetGestureProcessing(MapView* handler,
                                 TaskFunction function,
                                  bool processing)
                    :MapViewTask(handler, function),
                    m_isProcessing(processing) {}
    virtual ~RenderTaskSetGestureProcessing() {}

    bool m_isProcessing;
};

class RenderTaskAddPins : public MapViewTask
{
public:
    RenderTaskAddPins(MapView* handler, TaskFunction function, std::vector<NBGM_PinParameters>& pinParameters)
        : MapViewTask(handler, function), m_pinParameters(pinParameters){}
    virtual ~RenderTaskAddPins(){};

    std::vector<NBGM_PinParameters> m_pinParameters;
};

class RenderTaskRemovePins : public MapViewTask
{
public:
    RenderTaskRemovePins(MapView* handler, TaskFunction function, const vector<shared_ptr<string> >& pinIDs)
        : MapViewTask(handler, function), m_pinIDs(pinIDs){}
    virtual ~RenderTaskRemovePins(){};

    const vector<shared_ptr<string> > m_pinIDs;
};

class RenderTaskRemoveAllPins : public MapViewTask
{
public:
    RenderTaskRemoveAllPins(MapView* handler, TaskFunction function)
        : MapViewTask(handler, function){}
    virtual ~RenderTaskRemoveAllPins(){};

};

class RenderTaskUpdatePinPosition : public MapViewTask
{
public:
    RenderTaskUpdatePinPosition(MapView* handler, TaskFunction function, shared_ptr<string> pinID, double lat, double lon)
        : MapViewTask(handler, function),
          m_pinID(pinID),
          m_lat(lat),
          m_lon(lon){}
    virtual ~RenderTaskUpdatePinPosition(){};

    shared_ptr<string> m_pinID;
    double m_lat;
    double m_lon;
};

class RenderTaskSetHBAOParameters : public MapViewTask
{
public:
    RenderTaskSetHBAOParameters(MapView* handler, TaskFunction function, const HBAOParameters& parameters)
        : MapViewTask(handler, function),
        m_Parameters(parameters){}

    virtual ~RenderTaskSetHBAOParameters(){};
    HBAOParameters m_Parameters;
};

class RenderTaskSetGlowParameters : public MapViewTask
{
public:
    RenderTaskSetGlowParameters(MapView* handler, TaskFunction function, const GlowParameters& parameters)
        : MapViewTask(handler, function),
        m_Parameters(parameters){}

    virtual ~RenderTaskSetGlowParameters(){};
    GlowParameters m_Parameters;
};

class RenderTaskSetDPI : public MapViewTask
{
public:
    RenderTaskSetDPI(MapView* handler, TaskFunction function, float dpi)
        : MapViewTask(handler, function),
        m_dpi(dpi){}

    virtual ~RenderTaskSetDPI(){};
    float m_dpi;
};

/*! Task used by MapView to unload materials. */
class RenderTaskUnloadMaterials : public MapViewTask
{
public:
    RenderTaskUnloadMaterials(MapView* handler, TaskFunction function)
        : MapViewTask(handler, function) {}
    virtual ~RenderTaskUnloadMaterials(){};

    vector<TilePtr> m_tilesToUnload; /*! Data used for RenderThread_UnloadTilesToNBGM */
};

}

#endif /* _MAPVIEWTASK_H_ */

/*! @} */
