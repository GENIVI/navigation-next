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

#include "ltknkuitypeimpl.h"
#include "mapwidget.h"
#include "polyline.h"
#include "polylineparameters.h"
#include "segmentattribute.h"
#include "patternsegment.h"

using namespace locationtoolkit;

const char* LTKNKUIPolylineImpl::POLYLINE_HIGHLIGHT_COLOR = "#0069bc";
const char* LTKNKUIPolylineImpl::POLYLINE_UNHIGHLIGHT_COLOR = "#807d7d";
const char* LTKNKUIPolylineImpl::POLYLINE_BORDER_COLOR = "#333333";
const char* LTKNKUIPolylineImpl::MANEUVER_COLOR = "#111111";
const float LTKNKUIPolylineImpl::TRAFFICE_LINE_WIDTH_RATE   = 0.238;
const float LTKNKUIPolylineImpl::ROUTE_UNHIGTHLIGHT_WIDTH_RATE   = 0.72;

LTKNKUIPolylineImpl::LTKNKUIPolylineImpl(locationtoolkit::MapWidget* map)
    : mPolyline(NULL)
    , mTrafficPolylineGreen(NULL)
    , mTrafficPolylineOthers(NULL)
    , mSelected(false)
    , mVisible(false)
    , mZOrder(false)
    , mNavMode(M_Remain)
    , mIsDayMode(false)
    , mLineWidth(0.f)
    , mMapView(map)
{
}

LTKNKUIPolylineImpl::LTKNKUIPolylineImpl(locationtoolkit::Polyline* polyline)
    : mPolyline(polyline)
    , mTrafficPolylineGreen(NULL)
    , mTrafficPolylineOthers(NULL)
    , mSelected(false)
    , mVisible(false)
    , mZOrder(false)
    , mNavMode(M_Remain)
    , mIsDayMode(false)
    , mLineWidth(0.f)
    , mMapView(NULL)
{
}


LTKNKUIPolylineImpl::~LTKNKUIPolylineImpl()
{
}

void LTKNKUIPolylineImpl::SetVisible(bool visible)
{
    mVisible = visible;
    if(mPolyline)
    {
        mPolyline->SetVisible(visible);
    }
}

void LTKNKUIPolylineImpl::SetSelected(bool selected)
{
    mSelected = selected;

    if(mPolyline)
    {
        mPolyline->SetSelected(selected);
    }
    if(mTrafficPolylineGreen)
    {
        if(selected && mNavMode != M_TurnByTurn)
        {
            mTrafficPolylineGreen->SetVisible(true);
            mTrafficPolylineGreen->SetSelected(selected);
        }
        else
        {
            mTrafficPolylineGreen->SetVisible(false);
        }
    }
    if(mTrafficPolylineOthers)
    {
        mTrafficPolylineOthers->SetSelected(selected);
    }
    if (selected)
    {
        SetZOrder(HigherOrder);
    }
    else
    {
        SetZOrder(HigherOrder - 1);
    }
}

void LTKNKUIPolylineImpl::SetZOrder(int zOrder)
{
    mZOrder = zOrder;
    if(mPolyline)
    {
        mPolyline->SetZOrder(zOrder);
    }
    if(mTrafficPolylineGreen)
    {
        mTrafficPolylineGreen->SetZOrder(zOrder+1);
    }
    if(mTrafficPolylineOthers)
    {
        mTrafficPolylineOthers->SetZOrder(zOrder+2);
    }
}

void LTKNKUIPolylineImpl::SetNavMode(NavigationMode navMode)
{
    mNavMode = navMode;
    if(mPolyline)
    {
        switch (mNavMode)
        {
        case M_TurnByTurn:
        {
            mPolyline->SetSelected(true);
            mTrafficPolylineGreen->SetVisible(false);
            break;
        }
        default:
        {
            mPolyline->SetSelected(mSelected);
            break;
        }
        }
    }
}

bool LTKNKUIPolylineImpl::IsVisible() const
{
    return mVisible;
}

bool LTKNKUIPolylineImpl::IsSeleced() const
{
    return mSelected;
}

bool LTKNKUIPolylineImpl::Init(const LTKNKUIPolylineParameter& parameter)
{
    if( mMapView )
    {
        QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
        QString strRoot = dir.absolutePath();
        QString imgPath;
        //polyline
        QList<Coordinates> points;
        for( int i = 0; i < parameter.points.size(); i++ )
        {
            points.append( parameter.points[i] );
        }
        QList<SegmentAttribute*> segAttrs;
        ColorSegment* segattr = new ColorSegment();
        segattr->SetEndPointIndex(parameter.colorSegments[parameter.colorSegments.size()-1].endIndex);
        segattr->SetColor(QColor(POLYLINE_HIGHLIGHT_COLOR));
        segAttrs.append(segattr);

        PolylineParameters para;
        para.SetPoints(points);
        para.SetSegmentAttributes(segAttrs);
        para.SetUnhighlightColor(QColor(POLYLINE_UNHIGHLIGHT_COLOR));
        para.SetWidth(POLYLINE_WIDTH);
        para.setOutlineColor(QColor(POLYLINE_BORDER_COLOR));
        para.setOutlineWidth(POLYLINE_BORDER_WIDTH);
        para.SetZOrder(PT_SOLID_LINE_ZORDER);
        mPolyline =  &(mMapView->AddPolyline( para ));
        //traffic
        QList<SegmentAttribute*> attrGreen;
        QList<SegmentAttribute*> attrOthers;
        segAttrs.clear();
        for(int i=0; i< parameter.colorSegments.size(); i++)
        {
            switch(parameter.colorSegments[i].trafficColor)
            {
            case locationtoolkit::TC_Green:
            {
                ColorSegment* colorSeg = new ColorSegment();
                colorSeg->SetEndPointIndex(parameter.colorSegments[i].endIndex);
                colorSeg->SetColor(QColor("#00FF00"));
                attrGreen.append(colorSeg);
                imgPath = strRoot + "/resource/images/trans_dash.png";
                PatternSegment* segattr = new PatternSegment(parameter.colorSegments[i].endIndex, imgPath, 0);
                attrOthers.append(segattr);
                break;
            }
            case locationtoolkit::TC_Red:
            {
                imgPath = strRoot + "/resource/images/red_dash.png";
                PatternSegment* segattr = new PatternSegment(parameter.colorSegments[i].endIndex, imgPath, 0);
                attrOthers.append(segattr);
                imgPath = strRoot + "/resource/images/trans_dash.png";
                segattr = new PatternSegment(parameter.colorSegments[i].endIndex, imgPath, 0);
                attrGreen.append(segattr);
                break;
            }
            case locationtoolkit::TC_Yellow:
            {
                imgPath = strRoot + "/resource/images/amber_dash.png";
                PatternSegment* segattr = new PatternSegment(parameter.colorSegments[i].endIndex, imgPath, 0);
                attrOthers.append(segattr);
                imgPath = strRoot + "/resource/images/trans_dash.png";
                segattr = new PatternSegment(parameter.colorSegments[i].endIndex, imgPath, 0);
                attrGreen.append(segattr);
                break;
            }
            default:
            {
                imgPath = strRoot + "/resource/images/trans_dash.png";
                PatternSegment* segattr = new PatternSegment(parameter.colorSegments[i].endIndex, imgPath, 0);
                attrOthers.append(segattr);
                segattr = new PatternSegment(parameter.colorSegments[i].endIndex, imgPath, 0);
                attrGreen.append(segattr);
                break;
            }
            }
        }
        PolylineParameters traffic;
        traffic.SetPoints(points);
        traffic.SetSegmentAttributes(attrGreen);
        traffic.SetUnhighlightColor(QColor(POLYLINE_HIGHLIGHT_COLOR));
        traffic.SetWidth(parameter.width*TRAFFICE_LINE_WIDTH_RATE);
        traffic.setOutlineColor(QColor(POLYLINE_BORDER_COLOR));
        traffic.setOutlineWidth(POLYLINE_BORDER_WIDTH);
        traffic.SetZOrder(PT_DASH_LINE_ZORDER);
        mTrafficPolylineGreen =  &(mMapView->AddPolyline( traffic ));
        traffic.SetSegmentAttributes(attrOthers);
        mTrafficPolylineOthers = &(mMapView->AddPolyline( traffic ));
    }
    return true;
}

int LTKNKUIPolylineImpl::GetZOrder() const
{
    return mZOrder;
}

NavigationMode LTKNKUIPolylineImpl::GetNavMode() const
{
    return mNavMode;
}

void LTKNKUIPolylineImpl::SetDayNightMode(bool isDayMode)
{
    if (mIsDayMode == isDayMode)
    {
        return;
    }
    mIsDayMode = isDayMode;
}

void LTKNKUIPolylineImpl::UpdateTrafficPolyline(const LTKNKUIPolylineParameter *parameter)
{
    if( mMapView )
    {
        int zorder = PT_DASH_LINE_ZORDER;
        if(mTrafficPolylineOthers)
        {
            zorder =  mTrafficPolylineOthers->GetZOrder();
            mMapView->RemovePolyline(mTrafficPolylineOthers);
            mTrafficPolylineOthers = NULL;
        }

        QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
        QString strRoot = dir.absolutePath();
        QString imgPath;
        //polyline
        QList<Coordinates> points;
        for( int i = 0; i < parameter->points.size(); i++ )
        {
            points.append( parameter->points[i] );
        }
        //traffic
        QList<SegmentAttribute*> attrGreen;
        QList<SegmentAttribute*> attrOthers;
        for(int i=0; i< parameter->colorSegments.size(); i++)
        {
            switch(parameter->colorSegments[i].trafficColor)
            {
            case locationtoolkit::TC_Green:
            {
                imgPath = strRoot + "/resource/images/trans_dash.png";
                PatternSegment* segattr = new PatternSegment(parameter->colorSegments[i].endIndex, imgPath, 0);
                attrOthers.append(segattr);
                break;
            }
            case locationtoolkit::TC_Red:
            {
                imgPath = strRoot + "/resource/images/red_dash.png";
                PatternSegment* segattr = new PatternSegment(parameter->colorSegments[i].endIndex, imgPath, 0);
                attrOthers.append(segattr);
                break;
            }
            case locationtoolkit::TC_Yellow:
            {
                imgPath = strRoot + "/resource/images/amber_dash.png";
                PatternSegment* segattr = new PatternSegment(parameter->colorSegments[i].endIndex, imgPath, 0);
                attrOthers.append(segattr);
                break;
            }
            default:
            {
                imgPath = strRoot + "/resource/images/trans_dash.png";
                PatternSegment* segattr = new PatternSegment(parameter->colorSegments[i].endIndex, imgPath, 0);
                attrOthers.append(segattr);
                segattr = new PatternSegment(parameter->colorSegments[i].endIndex, imgPath, 0);
                attrGreen.append(segattr);
                break;
            }
            }
        }
        PolylineParameters traffic;
        traffic.SetPoints(points);
        traffic.SetUnhighlightColor(QColor(POLYLINE_HIGHLIGHT_COLOR));
        traffic.SetWidth(parameter->width*TRAFFICE_LINE_WIDTH_RATE);
        traffic.setOutlineColor(QColor(POLYLINE_BORDER_COLOR));
        traffic.setOutlineWidth(POLYLINE_BORDER_WIDTH);
        traffic.SetZOrder(zorder);
        traffic.SetSegmentAttributes(attrOthers);
        mTrafficPolylineOthers = &(mMapView->AddPolyline( traffic ));
    }
}

const QString &LTKNKUIPolylineImpl::GetID()
{
    if(mPolyline)
    {
        return mPolyline->GetID();
    }
    else
    {
        return "";
    }
}

void LTKNKUIPolylineImpl::Remove()
{
    if(mPolyline)
    {
        mMapView->RemovePolyline(mPolyline);
        mPolyline = NULL;
    }
    if(mTrafficPolylineGreen)
    {
        mMapView->RemovePolyline(mTrafficPolylineGreen);
        mTrafficPolylineGreen = NULL;
    }
    if(mTrafficPolylineOthers)
    {
        mMapView->RemovePolyline(mTrafficPolylineOthers);
        mTrafficPolylineOthers = NULL;
    }
}

bool LTKNKUIPolylineImpl::ContainPolyline(locationtoolkit::Polyline* polyline)
{
    if(mPolyline == polyline)
    {
        return true;
    }
    if(mTrafficPolylineGreen == polyline)
    {
        return true;
    }
    if(mTrafficPolylineOthers == polyline)
    {
        return true;
    }
    return false;
}
