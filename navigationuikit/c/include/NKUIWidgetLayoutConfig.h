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
 @file         NKUIWidgetLayoutConfig
 @defgroup     nkui
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */

#ifndef _NKUI_WIDGETLAYOUTCONFIG_H_
#define _NKUI_WIDGETLAYOUTCONFIG_H_

#include "NKUIStateID.h"
#include "WidgetID.h"
#include <string>
#include <map>
#include <vector>
#include <list>

class TiXmlElement;

namespace nkui
{
/**
 *  postion origin type
 */
typedef enum
{
    LYO_INVALID = 0,
    LYO_LEFT,
    LYO_RIGHT,
    LYO_BUTTOM,
    LYO_TOP,
    LYO_CENTER
} LayoutOrigin;

/**
 *  value type
 */
typedef enum
{
    VLT_Relative = 0,
    VLT_Absolute
} ValueType;

/**
 *  widget layout positon
 */
typedef struct
{
    LayoutOrigin origin;
    double       value;
    ValueType    type;
} LayoutPoint;

/**
 *  widget layout size
 */
typedef struct
{
    double       value;
    ValueType    type;
} LayoutSize;

typedef struct
{
    /*! frame size for a widget. the unit is x,width percent with parent view's width
     y,height is view's height*/
    LayoutPoint xPos;
    LayoutPoint yPos;
    LayoutSize width;
    LayoutSize height;
    int offsetX;
} LayoutRect;

typedef struct
{
    LayoutRect  Landscape;
    LayoutRect  Portrait;
    int         zorder;
    float       alpha;
} LayoutRectData;

/**
 *  reference creen size
 */
typedef struct
{
    int width;
    int height;
} ScreenSize;

/**
 *  widget frame
 */
typedef struct
{
    double posx;
    double posy;
    double width;
    double height;
} WidgetFrame;

typedef std::list<WidgetID> StateWidgetList;
typedef std::map<WidgetID, LayoutRectData> StateViewLayoutConfig;

typedef struct
{
    StateWidgetList widgetList;
    StateViewLayoutConfig layoutConfig;
} StateData;

typedef std::map<NKUIStateID, StateData> WidgetLayoutConfigs;

class WidgetLayoutConfig
{
public:
    WidgetLayoutConfig(const std::string& configFilePath, const std::string deviceType);
    virtual ~WidgetLayoutConfig();
    /**
     *  Get widget frame at current state
     *
     *  @param state      current nkui state
     *  @param id         widget id
     *  @param parentSize parentview size include widgets
     *  @param frame      widget frame will at there
     *  @param zorder     widget's zorder
     *
     *  @return if success get frame will return true
     */
    bool GetWidgetFrame(NKUIStateID state, WidgetID id, const WidgetFrame& parentSize, WidgetFrame& frame);
    /**
     *  get widget's zorder
     *
     *  @param state current nkui state
     *  @param id    widget id
     *
     *  @return zorder
     */
    int GetWidgetZorder(NKUIStateID state, WidgetID id);

    /**
     *  get state widget list
     *
     *  @param state current nkui state
     *  @param list
     *
     *  @return bool if configure file have widget list return true
     */
    bool GetWidgetList(NKUIStateID state, std::list<WidgetID>& list);


    /**
     *  get widget's alpha
     *
     *  @param state current nkui state
     *  @param id    widget id
     *  @return alpha
     */
    float GetWidgetAlpha(NKUIStateID state, WidgetID id);

    /**
     *  load config from xml file
     *
     *  @return if success return true
     */
    bool LoadConfigs();
private:
    WidgetLayoutConfig();
    /**
     *  init widgetID and navState map, the key is string, value is enum value. then we can easy covert from enum string to enum value
     */
    void InitWidgetIDMap();
    void InitNavStateMap();
    /**
     *  parse screen element, it include support screen size, include width and height
     *
     *  @param pElement screen element in xml file
     *
     *  @return if have any error will return false
     */
    bool ParseScreenElement(TiXmlElement* pElement);
    /**
     *  parse nkui state element, it include widget layout
     *
     *  @param pElement state element
     *
     *  @return if have any error will return false
     */
    bool ParseStateElement(TiXmlElement* pElement);
    /**
     *  widget layout element
     *  @return if have any error will return false
     */
    bool ParseWidgetLayoutElement(TiXmlElement* pElement, LayoutRectData& data);
    /**
     *  parse widget frame,
     *
     *  @param pElement frame element in xml
     *  @param data     result
     *
     *  @return if have any error will return false
     */
    bool ParseWidgetFrame(TiXmlElement* pElement, LayoutRect& data);
    /**
     *  parse position attribute
     *
     *  @param pElement frame element, it include  postion
     *  @param key      it can be "pos_x" or "pos_y"
     *  @param point    result
     *  @param isHeight if this postion ref by height
     *
     *  @return if have any error will return false
     */
    bool ParsePosition(TiXmlElement* pElement, const char* key, LayoutPoint& point, bool isHeight);
    /**
     *  parse size attribute
     *
     *  @param pElement frame elemnt in xml
     *  @param key      the attribute key
     *  @param size     result
     *
     *  @return if have any error will return false
     */
    bool ParseSize(TiXmlElement* pElement, const char* key, LayoutSize& size);
    /**
     *  check a string if a digit string
     *
     *  @param value the string to check
     *
     *  @return true or false
     */
    bool IsDigit(const char* value);
    /**
     *  get best match reference screen size
     *
     *  @param parentSize widget parent size
     *
     *  @return the best match screen size
     */
    ScreenSize GetBestMatchScreenSize(const WidgetFrame& parentSize);
    double GetWidthHeightRatio(const WidgetFrame& size);
    double GetWidthHeightRatio(const ScreenSize& size);
    NKUIStateID GetNKUIStateID(const char* stateName);
    WidgetID    GetWidgetID(const char* widgetName);
private:
    std::map<std::string,WidgetID> m_widgetIDMap;
    std::map<std::string,NKUIStateID> m_navStateMap;
    WidgetLayoutConfigs m_layoutConfig;
    std::vector<ScreenSize> m_screenSizes;
    std::string m_configFilePath;
    std::string m_deviceType;
};
}

#endif /* defined(_NKUI_WIDGETLAYOUTCONFIG_H_) */

/*! @} */
