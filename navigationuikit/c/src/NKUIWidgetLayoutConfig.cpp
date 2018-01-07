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
 @file         WidgetLayoutConfig.cpp
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

#include "NKUIWidgetLayoutConfig.h"
#include <ctype.h>
#include <math.h>
#include "tinyxml.h"
#include "palstdlib.h"

#define IPHONE "IPhone"
#define IPAD   "IPad"
//define configer's element name
#define SCREEN "Screen"
#define STATE  "State"
#define FRAME  "Frame"
#define PORTRAIT "Portrait"
#define LANDSCAPE "Landscape"
#define WIDGETLAYOUT "WidgetLayout"
#define WIDGET "Widget"
#define WIDGETLIST "WidgetList"
//attribute
#define WIDTH  "width"
#define HEIGHT "height"
#define POSX   "pos_x"
#define POSY   "pos_y"
#define OFFSETX "offset_x"
#define MAXWIDTH "max_width"
#define MAXHEIGHT "max_height"
#define MINWIDTH "min_width"
#define MINHEIGHT "min_height"
#define NAME   "name"
#define ID     "id"
#define ZORDER "zorder"
#define ALPHA "alpha"
#define DEFAULT "DEFAULT"
#define LISTNAME "listname"
//specail value
#define FILL 'x'
#define REST 'f'
#define FILLVALUE -1
#define RESTVALUE -2
#define NATIVE '-'         //negatve value
#define OPPOSITE 'o'       //opposite layout. eg postionx normal is origin from left, if set 'o' the origin is from right
#define CENTER 'c'         //center layout
#define ABSOLUTE 'a'       //absolute value


using namespace nkui;
using namespace std;

WidgetLayoutConfig::WidgetLayoutConfig()
{
}

WidgetLayoutConfig::WidgetLayoutConfig(const std::string& configFilePath, const std::string deviceType)
{
    InitNavStateMap();
    InitWidgetIDMap();
    m_configFilePath = configFilePath;
    m_deviceType = deviceType;
}

bool WidgetLayoutConfig::LoadConfigs()
{
    bool ret = true;
    TiXmlDocument *pDoc = new TiXmlDocument(m_configFilePath.c_str());
    if(pDoc->LoadFile())
    {
        TiXmlHandle hDoc(pDoc);
        //find if have deviceType's config
        TiXmlElement* pElem = hDoc.FirstChildElement(m_deviceType.c_str()).Element();
        if(pElem)
        {
            //get screen size reference, this element must exist, maybe have multiple reference size
            TiXmlElement* pScreen = pElem->FirstChildElement(SCREEN);
            for(; pScreen; pScreen = pScreen->NextSiblingElement(SCREEN))
            {
                if(pScreen)
                {
                    if(!ParseScreenElement(pScreen))
                    {
                        printf("Parse config file error, parse screen element error.\n");
                        ret = false;
                    }
                }
                else
                {
                    ret = false;
                    printf("Parse config file error, can't find screen element.\n");
                }
            }
            if(ret)
            {
                //traversed all child element and parse "State" element
                for(pElem = pElem->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
                {
                    std::string name = pElem->Value();
                    if(name == STATE)
                    {
                        ParseStateElement(pElem);
                    }
                }
            }
        }
        else
        {
            printf("Parse config file error, no specified element(%s)\n", m_deviceType.c_str());
            ret = false;
        }
    }
    else
    {
        printf("Parse config file error.");
        ret = false;
    }
    delete pDoc;
    return ret;
}

WidgetLayoutConfig::~WidgetLayoutConfig()
{

}

bool WidgetLayoutConfig::GetWidgetList(NKUIStateID state, std::list<WidgetID>& list)
{
    //check if have this state's config
    StateViewLayoutConfig::iterator result;
    WidgetLayoutConfigs::iterator it = m_layoutConfig.find(state);
    if(it != m_layoutConfig.end())
    {
        if((it->second).widgetList.size() > 0)
        {
            list.insert(list.end(), (it->second).widgetList.begin(), (it->second).widgetList.end());
            return true;
        }
    }
    return false;
}

int WidgetLayoutConfig::GetWidgetZorder(NKUIStateID state, WidgetID id)
{
    int zorder = 0;
    //check if have this state's config
    StateViewLayoutConfig::iterator result;
    WidgetLayoutConfigs::iterator it = m_layoutConfig.find(state);
    if(it != m_layoutConfig.end())
    {
        //get widget config by id
        result = (it->second).layoutConfig.find(id);
    }
    if(it == m_layoutConfig.end() || result == (it->second).layoutConfig.end())
    {
        //try to find from default
        it = m_layoutConfig.find(NAVSTATE_INVALID);
        if(it != m_layoutConfig.end())
        {
            //get widget config by id
            result = (it->second).layoutConfig.find(id);
        }
    }
    if(it != m_layoutConfig.end() && result != (it->second).layoutConfig.end())
    {
        zorder = result->second.zorder;
    }
    return zorder;
}

bool WidgetLayoutConfig::GetWidgetFrame(NKUIStateID state, WidgetID id, const WidgetFrame& parentSize, WidgetFrame& frame)
{
    bool ret = false;
    //get best match size by parentSize to support iphone5/iphone4
    ScreenSize refSize = GetBestMatchScreenSize(parentSize);
    //check if have this state's config
    StateViewLayoutConfig::iterator result;
    WidgetLayoutConfigs::iterator it = m_layoutConfig.find(state);
    if(it != m_layoutConfig.end())
    {
        //get widget config by id
        result = (it->second).layoutConfig.find(id);
    }
    if(it == m_layoutConfig.end() || result == (it->second).layoutConfig.end())
    {
        //try to find from default
        it = m_layoutConfig.find(NAVSTATE_INVALID);
        if(it != m_layoutConfig.end())
        {
            //get widget config by id
            result = (it->second).layoutConfig.find(id);
        }
    }
    if(it != m_layoutConfig.end() && result != (it->second).layoutConfig.end())
    {
        LayoutRect layoutData;
        //portrait or landscape by width and height
        bool isPortrait = parentSize.height > parentSize.width;
        //Landscape
        if(!isPortrait)
        {
            layoutData = result->second.Landscape;
            //correct refSize when landscape
            int temp = refSize.height;
            refSize.height = refSize.width;
            refSize.width = temp;
        }
        else //Portrait
        {
            layoutData = result->second.Portrait;
        }
        double radio = 0;
        //width
        radio = layoutData.width.value / refSize.width;
        layoutData.offsetX = (int)(layoutData.offsetX*(parentSize.width/refSize.width));
        if(layoutData.width.value == FILLVALUE)//'x' case, when set height/width to 'x' in config file
        {
            frame.width = parentSize.width - layoutData.offsetX;
        }
        else if(layoutData.width.value == RESTVALUE)//'f'case, will fill the rest space
        {
            if (layoutData.xPos.type == VLT_Absolute)
            {
                frame.width = parentSize.width - layoutData.xPos.value;
            }
            else
            {
                frame.width = (parentSize.width - parentSize.width * layoutData.xPos.value/refSize.width);
            }
        }
        else
        {
            frame.width = (layoutData.width.type == VLT_Absolute) ? layoutData.width.value : parentSize.width * radio;
        }
        //height
        radio = layoutData.height.value / refSize.height;
        if(layoutData.height.value == FILLVALUE) //'x' case, when set height/width to 'x' in config file
        {
            frame.height = parentSize.height;
        }
        else if(layoutData.height.value == RESTVALUE)//'f'case, will fill the rest space
        {
            if (layoutData.yPos.type == VLT_Absolute)
            {
                frame.height = parentSize.height - layoutData.yPos.value;
            }
            else
            {
                frame.height = (parentSize.height - parentSize.height * layoutData.yPos.value/refSize.height);
            }
        }
        else
        {
            frame.height = (layoutData.height.type == VLT_Absolute) ? layoutData.height.value : parentSize.height * radio;
        }
        //postion x
        radio = layoutData.xPos.value / refSize.width;
        frame.posx = (layoutData.xPos.type == VLT_Absolute) ? layoutData.xPos.value : (radio * parentSize.width + layoutData.offsetX);
        if(layoutData.xPos.origin == LYO_RIGHT)
        {
            frame.posx = parentSize.width - frame.posx - frame.width;
        }
        else if(layoutData.xPos.origin == LYO_CENTER)
        {
            frame.posx = (parentSize.width - frame.width - layoutData.offsetX)/2.0 + layoutData.offsetX;
        }
        //position y
        radio = layoutData.yPos.value / refSize.height;
        frame.posy = (layoutData.yPos.type == VLT_Absolute) ? layoutData.yPos.value : radio * parentSize.height;
        if(layoutData.yPos.origin == LYO_BUTTOM)
        {
            frame.posy = parentSize.height - frame.posy - frame.height;
        }
        else if(layoutData.yPos.origin == LYO_CENTER)
        {
            frame.posy = (parentSize.height - frame.height)/2.0;
        }
        ret = true;
    }
    return ret;
}

float WidgetLayoutConfig::GetWidgetAlpha(NKUIStateID state, WidgetID id)
{
    float alpha = -1.f;
    //check if have this state's config
    StateViewLayoutConfig::iterator result;
    WidgetLayoutConfigs::iterator it = m_layoutConfig.find(state);
    if(it != m_layoutConfig.end())
    {
        //get widget config by id
        result = (it->second).layoutConfig.find(id);
    }
    if(it == m_layoutConfig.end() || result == (it->second).layoutConfig.end())
    {
        //try to find from default
        it = m_layoutConfig.find(NAVSTATE_INVALID);
        if(it != m_layoutConfig.end())
        {
            //get widget config by id
            result = (it->second).layoutConfig.find(id);
        }
    }
    if(it != m_layoutConfig.end() && result != (it->second).layoutConfig.end())
    {
        alpha = result->second.alpha;
    }
    return alpha;
}

ScreenSize WidgetLayoutConfig::GetBestMatchScreenSize(const WidgetFrame& parentSize)
{
    ScreenSize bestScreenSize;
    bestScreenSize.height = (int)parentSize.height;
    bestScreenSize.width = (int)parentSize.width;
    if(m_screenSizes.size() > 0)
    {
        bestScreenSize = *(m_screenSizes.begin());
        double parentRatio = GetWidthHeightRatio(parentSize);
        double different = parentRatio;
        for(std::vector<ScreenSize>::const_iterator iter = m_screenSizes.begin(); iter != m_screenSizes.end(); iter++)
        {
            double ratio = GetWidthHeightRatio(*iter);
            if(fabs(ratio - parentRatio) < different)
            {
                different = fabs(ratio - parentRatio);
                bestScreenSize = *iter;
            }
        }
    }
    return bestScreenSize;
}

double WidgetLayoutConfig::GetWidthHeightRatio(const WidgetFrame& size)
{
    if(size.width > size.height)
    {
        return size.height/(double)size.width;
    }
    else
    {
        return size.width/(double)size.height;
    }
}

double WidgetLayoutConfig::GetWidthHeightRatio(const ScreenSize& size)
{
    if(size.width > size.height)
    {
        return size.height/(double)size.width;
    }
    else
    {
        return size.width/(double)size.height;
    }
}

bool WidgetLayoutConfig::ParseScreenElement(TiXmlElement* pElement)
{
    bool ret = true;
    ScreenSize screen;
    const char* pValue = pElement->Attribute(WIDTH);
    if(pValue)
    {
        sscanf(pValue, "%d", &screen.width);
    }
    else
    {
        ret = false;
    }
    pValue = pElement->Attribute(HEIGHT);
    if(pValue)
    {
        sscanf(pValue, "%d", &screen.height);
    }
    else
    {
        ret = false;
    }
    if(ret)
    {
        m_screenSizes.push_back(screen);
    }
    return ret;
}

bool WidgetLayoutConfig::ParseWidgetLayoutElement(TiXmlElement* pElement, LayoutRectData& data)
{
    bool ret = true;
    //parse portrait layout
    TiXmlElement* pPortrait = pElement->FirstChildElement(PORTRAIT);
    if(pPortrait)
    {
        ret = ParseWidgetFrame(pPortrait, data.Portrait);
    }
    else
    {
        printf("Missing Portrait define %s\n", pElement->Attribute(NAME));
        ret = false;
    }
    if(ret)
    {
        //landscape layout
        TiXmlElement* pLandscape = pElement->FirstChildElement(LANDSCAPE);
        if(pLandscape)
        {
           ret = ParseWidgetFrame(pLandscape, data.Landscape);
        }
        else
        {
            printf("Missing Landscape define %s\n", pElement->Attribute(NAME));
            ret = false;
        }
    }
    return ret;
}

bool WidgetLayoutConfig::ParseWidgetFrame(TiXmlElement* pElement, LayoutRect& data)
{
    bool ret = false;
    TiXmlElement* pFrame = pElement->FirstChildElement(FRAME);
    if(pFrame)
    {
        ret = ParsePosition(pFrame, POSX, data.xPos, false);
        if(ret)
        {
            ret = ParsePosition(pFrame, POSY, data.yPos, true);
        }
        if(ret)
        {
            ret = ParseSize(pFrame, WIDTH, data.width);
        }
        if(ret)
        {
            ret = ParseSize(pFrame, HEIGHT, data.height);
        }
        if(ret)
        {
            const char* pOffsetx = pFrame->Attribute(OFFSETX);
            //the widget default zorder is 0
            data.offsetX = 0;
            if(pOffsetx && IsDigit(pOffsetx))
            {
                sscanf(pOffsetx, "%d", &(data.offsetX));
            }
        }
      }
    return ret;
}

bool WidgetLayoutConfig::ParsePosition(TiXmlElement* pElement, const char* key, LayoutPoint& point,  bool isHeight)
{
    bool ret = false;
    const char* pValue = pElement->Attribute(key);
    if(pValue)
    {
        if (*pValue == ABSOLUTE)
        {
            point.type = VLT_Absolute;
            ++pValue;
        }
        else
        {
            point.type = VLT_Relative;
        }
        if(*pValue == OPPOSITE)
        {
            if(isHeight)
            {
                point.origin = LYO_BUTTOM;
            }
            else
            {
                point.origin = LYO_RIGHT;
            }
            pValue++;
        }
        else if(*pValue == CENTER)
        {
            point.origin = LYO_CENTER;
            point.value = 0;
            ret = true;
        }
        else
        {
            if(isHeight)
            {
                point.origin = LYO_TOP;
            }
            else
            {
                point.origin = LYO_LEFT;
            }
        }
        if(*pValue != CENTER)
        {
            int negative = 1;
            if(*pValue == NATIVE)
            {
                negative = -1;
                pValue++;
            }
            if(IsDigit(pValue))
            {
                sscanf(pValue, "%lf", &(point.value));
                point.value*=negative;
                ret = true;
            }
        }
    }
    return ret;
}

bool WidgetLayoutConfig::ParseSize(TiXmlElement* pElement, const char* key, LayoutSize& size)
{
    bool ret = false;
    const char* pValue = pElement->Attribute(key);
    if(pValue)
    {
        if (*pValue == ABSOLUTE)
        {
            size.type = VLT_Absolute;
            ++pValue;
        }
        else
        {
            size.type = VLT_Relative;
        }
        if(*pValue == FILL)
        {
            size.value = FILLVALUE;
            ret = true;
        }
        else if(*pValue == REST)
        {
            size.value = RESTVALUE;
            ret = true;
        }
        else
        {
            if(IsDigit(pValue))
            {
                sscanf(pValue, "%lf", &size.value);
                ret = true;
            }
        }
    }
    return ret;
}

bool WidgetLayoutConfig::IsDigit(const char* value)
{
    bool ret = true;
    for(;*value;value++)
    {
        if(!isdigit(*value))
        {
            ret = false;
            break;
        }
    }
    return ret;
}

bool WidgetLayoutConfig::ParseStateElement(TiXmlElement* pElement)
{
    bool ret = true;
    //get state ID
    const char* pStateId = pElement->Attribute(ID);
    if(pStateId)
    {
        StateViewLayoutConfig config;
        NKUIStateID uiState = GetNKUIStateID(pStateId);
        //if state is "DEFAULT", this is a special element, it save default widget layout
        if(uiState == NAVSTATE_INVALID && strcmp(DEFAULT, pStateId))
        {
            printf("Warnning unknown state.\n");
            ret = false;
        }
        StateWidgetList stateWidgetList;
        TiXmlElement* widgetList = pElement->FirstChildElement(WIDGETLIST);
        if(ret && widgetList)
        {
            const char* list = widgetList->Attribute(LISTNAME);
            if(list)
            {
                std::vector<WidgetID> widgetIDs;
                char* buf = (char*)nsl_malloc(nsl_strlen(list) + 1);
                if(buf)
                {
                    nsl_strcpy(buf,list);
                    const char *div = " ,";
                    char* inner_ptr = NULL;
                    char* p = nsl_strtok_r(buf, div, &inner_ptr);
                    while(p)
                    {
                        WidgetID id = GetWidgetID(p);
                        if(id != WGT_INVALID)
                        {
                            stateWidgetList.push_back(id);
                        }
                        p = nsl_strtok_r(NULL, div, &inner_ptr);
                    }
                    nsl_free(buf);
                }
            }
        }
        TiXmlElement* widgetLayouts = pElement->FirstChildElement(WIDGETLAYOUT);
        if(ret && widgetLayouts)
        {
            for(TiXmlElement* pWidget = widgetLayouts->FirstChildElement(WIDGET); pWidget; pWidget = pWidget->NextSiblingElement(WIDGET))
            {
                const char* pWidgetID = pWidget->Attribute(ID);
                if(pWidgetID)
                {
                    WidgetID widgetID = GetWidgetID(pWidgetID);
                    LayoutRectData data;
                    ret = ParseWidgetLayoutElement(pWidget, data);
                    if(!ret)
                    {
                        printf("failed load %s's frame\n",pWidgetID);
                    }
                    const char* pZorder = pWidget->Attribute(ZORDER);
                    //the widget default zorder is 0
                    data.zorder = 0;
                    if(pZorder && IsDigit(pZorder))
                    {
                        sscanf(pZorder, "%d", &(data.zorder));
                        ret = true;
                    }
                    const char* pAlpha = pWidget->Attribute(ALPHA);
                    if(pAlpha == NULL || sscanf(pAlpha, "%f", &(data.alpha)) == EOF)
                    {
                        data.alpha = -1.0;
                    }
                    config.insert(StateViewLayoutConfig::value_type(widgetID, data));
                }
            }
        }
        else
        {
            printf(" can't find WidgetLayout element.\n");
            ret = false;
        }
        if(ret)
        {
            StateData state;
            state.layoutConfig = config;
            state.widgetList = stateWidgetList;
            m_layoutConfig.insert(WidgetLayoutConfigs::value_type(uiState, state));
        }
    }
    else
    {
        printf("failed get state id.\n");
    }
    return ret;
}

NKUIStateID WidgetLayoutConfig::GetNKUIStateID(const char* stateName)
{
    std::map<std::string,NKUIStateID>::iterator iter = m_navStateMap.find(stateName);
    if(iter != m_navStateMap.end())
    {
        return iter->second;
    }
    printf("Warnning: unknown State %s\n", stateName);
    return NAVSTATE_INVALID;
}

WidgetID WidgetLayoutConfig::GetWidgetID(const char* widgetName)
{
    std::map<std::string,WidgetID>::iterator iter = m_widgetIDMap.find(widgetName);
    if(iter != m_widgetIDMap.end())
    {
        return iter->second;
    }
    printf("Warnning: unknown State %s\n", widgetName);
    return WGT_INVALID;
}

void WidgetLayoutConfig::InitWidgetIDMap()
{
    m_widgetIDMap.clear();
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_MAP",WGT_MAP));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ANNOUNCER",WGT_ANNOUNCER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_REQUEST_FOOTER",WGT_REQUEST_FOOTER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_STARTINGNAV_FOOTER",WGT_STARTINGNAV_FOOTER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ROUTE_SELECT_FOOTER",WGT_ROUTE_SELECT_FOOTER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ROUTE_SELECT_HEADER",WGT_ROUTE_SELECT_HEADER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ROUTE_SELECT_BUBBLE",WGT_ROUTE_SELECT_BUBBLE));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_NAV_FOOTER",WGT_NAV_FOOTER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_NAV_HEADER",WGT_NAV_HEADER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_OVERFLOW_MENU",WGT_OVERFLOW_MENU));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_CURRENTROAD",WGT_CURRENTROAD));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_STACKED_MANEUVER",WGT_STACKED_MANEUVER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_MINI_MAP_NAV",WGT_MINI_MAP_NAV));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_MINI_MAP_LOCATEME",WGT_MINI_MAP_LOCATEME));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_MINI_MAP_PED",WGT_MINI_MAP_PED));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_TRIP_OVERVIEW_HEADER",WGT_TRIP_OVERVIEW_HEADER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ROUTE_DETALLS_LIST",WGT_ROUTE_DETALLS_LIST));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_NAVIGATION_LIST",WGT_NAVIGATION_LIST));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_SAR",WGT_SAR));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_LANE_GUIDANCE",WGT_LANE_GUIDANCE));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_LANE_GUIDANCE_SAR",WGT_LANE_GUIDANCE_SAR));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_SPEED_LIMITS",WGT_SPEED_LIMITS));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ARRIVAL_FOOTER",WGT_ARRIVAL_FOOTER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ARRIVAL_HEADER",WGT_ARRIVAL_HEADER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_MINI_MAP_RTS",WGT_MINI_MAP_RTS));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_RETRY",WGT_RETRY));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_DETOUR_FOOTER",WGT_DETOUR_FOOTER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_DETOUR_HEADER",WGT_DETOUR_HEADER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_NOGPS",WGT_NOGPS));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_PEDESTRIAN_FOOTER",WGT_PEDESTRIAN_FOOTER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_PEDESTRIAN_HEADER",WGT_PEDESTRIAN_HEADER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_PEDESTRIAN_LIST",WGT_PEDESTRIAN_LIST));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_RECALCULATE",WGT_RECALCULATE));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ENHANCE_NAV_NOTIFICATION",WGT_ENHANCE_NAV_NOTIFICATION));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ENHANCE_HEADER",WGT_ENHANCE_HEADER));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_STATUS_BAR",WGT_STATUS_BAR));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_COMPASS",WGT_COMPASS));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_PIN_BUBBLE",WGT_PIN_BUBBLE));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_MAP_PREFETCH",WGT_MAP_PREFETCH));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ROUTE_SELECT_STATUS_BAR",WGT_ROUTE_SELECT_STATUS_BAR));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_ENDTRIP_BAR",WGT_ENDTRIP_BAR));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_BACKGROUND",WGT_BACKGROUND));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_TRAFFIC_AHEAD",WGT_TRAFFIC_AHEAD));
    m_widgetIDMap.insert(std::map<std::string,WidgetID>::value_type("WGT_OVERVIEW_FOOTER",WGT_OVERVIEW_FOOTER));
}

void WidgetLayoutConfig::InitNavStateMap()
{
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_REQUEST_NAVIGATION",NAVSTATE_REQUEST_NAVIGATION)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_STARING_NAVIGATION",NAVSTATE_STARING_NAVIGATION)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_ROUTE_SELECTION",NAVSTATE_ROUTE_SELECTION)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_NAVIGATION",NAVSTATE_NAVIGATION)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_NAVIGATION_SAR",NAVSTATE_NAVIGATION_SAR)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_TRIP_OVERVIEW",NAVSTATE_TRIP_OVERVIEW)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_ARRIVAL",NAVSTATE_ARRIVAL)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_NAVIGATION_LIST",NAVSTATE_NAVIGATION_LIST)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_ROUTE_SELECTION_LIST",NAVSTATE_ROUTE_SELECTION_LIST)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_DETOUR",NAVSTATE_DETOUR)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_DETOUR_LIST",NAVSTATE_DETOUR_LIST)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_PEDESTRIAN",NAVSTATE_PEDESTRIAN)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_PEDESTRIAN_UNLOCK",NAVSTATE_PEDESTRIAN_UNLOCK)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_PEDESTRIAN_LIST",NAVSTATE_PEDESTRIAN_LIST)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_PEDESTRIAN_OVERVIEW",NAVSTATE_PEDESTRIAN_OVERVIEW)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_ENHANCED_STARTUP",NAVSTATE_ENHANCED_STARTUP)));
    m_navStateMap.insert((std::map<std::string,NKUIStateID>::value_type("NAVSTATE_NAVIGATION_ENDTRIP",NAVSTATE_NAVIGATION_ENDTRIP)));
}

/*! @} */
