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

@file     NBMPolylineMaker.h
@date     11/23/2013
@defgroup MOBIUS_NBM NavBuilder Binary Model API
@author   Seber Tian

*/
/*
(C) Copyright 2013 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include "NBMTypes.h"

using namespace std;

namespace NbmApi
{
    struct BmpPattern
    {
        unsigned int color;         //Color of on-color of a dash in b, g, r, a order.
        char* textureName;          //Texture Name
        char* textureBuf;           //Texture buffer
        unsigned int textureBufLen; //Texture buffer Length
        unsigned char distance;     //Transparent block length of road in %a of width
        unsigned char width;        //Texture width in pixel
        unsigned char height;       //Texture height in pixel
    };

    struct PolylineColor
    {
        Uint32_t color;          
        Uint32_t endIdx;                  //color transform point index
    };
    enum LineStyle
    {
        Solid,
        Dash
    };
    
    struct Point
    {
        float lat;  /* Value of latitude  */
        float lon;  /* Value of longitude */
        Point(float latitude,float longitude): lat(latitude),lon(longitude){}
        Point(const Point& pt){lat=pt.lat; lon=pt.lon;}
        Point():lat(0.0f),lon(0.0f){}
        Point& operator=(const Point& rhs)
        {
            if(this==&rhs)
                return *this;
            lat = rhs.lat;
            lon = rhs.lon;
            return *this;
        }
        ~Point(){}
        bool operator!=(const Point& pt)
        {
            return (this->lat!=pt.lat || this->lon!=pt.lon);
        }
        bool operator==(const Point& pt)
        {
            return !(this->operator !=(pt));
        }
    };
    typedef vector<Point> Polyline;
    struct PolylineElement
    {
        string polylineId;              /* an id for every polyline*/
        unsigned int polylineAttrId;    /* an id for every polyline attribute */
        vector<Point> vPoints;          /*all points of one polyline*/
    };
    struct PolylineComponent
    {
        string polylineId;              /* an id for every polyline*/
        vector<Point> vPoints;          /*all points of one polyline*/
    };
    enum CapType
    {
        Arrow,
        Circle,
        Line_Join
    };
    struct PolylineAttr
    {
        unsigned int polylineAttrId;    //an id for every different polyline attribute
        CapType startType;              //enum CapType
        CapType endType;                //enum CapType
        unsigned int interiorColor;     //Interior fill color, in b, g, r, a order.
        unsigned int outlineColor;      //Outline color, in b, g, r, a order.
        unsigned char outlineWidth;    //Outline width, as %a of the road width.
        unsigned short shapeHeight;     //Shape height in %a of width of road 
        unsigned short shapeWidth;      //Shape width in %a of width of road 
        unsigned short radius;          //Circle in %a of width of road
    };
    struct GeoPoint
    {
        Uint32_t color;
        Point pt;
        GeoPoint(Uint32_t c,Point point):color(c),pt(point){}
        GeoPoint():color(0),pt(0.0f,0.0f){}
        bool operator!=(const GeoPoint& pt)
        {
            return (this->pt!=pt.pt || this->color!=pt.color);
        }
        bool operator==(const GeoPoint& pt)
        {
            return !(this->operator !=(pt));
        }
    };
    typedef vector<GeoPoint> ColorPolyline;

    struct ColorPolylineElement
    {
        string polylineId;           /* an id for every color polyline*/
        vector<GeoPoint> vGeoPoints; /*all points of one color polyline*/
    };
}

struct PolylineWithColorIdx
{
    vector<NbmApi::Point> vPoints;
    Uint32_t colorIdx;
};

struct PolySubAttr
{
    string polylineId;
    unsigned int polylineAttrId;
    unsigned char flag; //0: start polyline; 1: end polyline; 2: whole polyline; 3: middle polyline
    bool operator < (const PolySubAttr& rhs)const
    {
        if(polylineId<rhs.polylineId)
            return true;
        else if(polylineId==rhs.polylineId && polylineAttrId<rhs.polylineAttrId)
            return true;
        else if(polylineId==rhs.polylineId && polylineAttrId==rhs.polylineAttrId && flag<rhs.flag)
            return true;
        else
            return false;
    }
};


class MaterialMgr
{
public:
    MaterialMgr(){}
    ~MaterialMgr(){}

    static unsigned short SearchPolylineId(long long key,map<long long, unsigned short>& polylineMat);
    static unsigned short SearchColorPolylineId(unsigned int key, map<unsigned int, unsigned short>& colorPolylineMat);

    map<long long, unsigned short> m_multiColorMat;
    map<long long, unsigned short> m_polylineMat;
    map<unsigned int,unsigned short> m_bmpPatternMat;
};

namespace NbmApi
{
    class NBMPolylineMgr
    {
    public:
        NBMPolylineMgr();
        ~NBMPolylineMgr();

        /* Input parameters:
        ** line style for one polyline
        */
        void SetLineStyle(LineStyle lineStyle){m_colorPolylineStyle = lineStyle;}

        /* Input parameters:
        ** Multiple polyline colors for one polyline
        */
        void SetPolylineColorAttr(vector<PolylineColor>& vPolylineColor){m_vPolylineColor = vPolylineColor;}

        /* Input parameters:
        **  polyline pattern for one polyline
        */
        void SetPolylinePattern(vector<BmpPattern>& vPolylinePattern){m_vPolylinePattern = vPolylinePattern;}

        /* Input parameters:
        ** default color when un-highlight
        */
        void SetDefaultColor(Uint32_t color){m_defaultColor = color;}

        /* Input parameters:
        ** width: road width (mercator value)
        */
        void SetRoadWidth(float width){m_roadWidth = width;}

        /* Input parameters:
        ** vPolylineAttr: polyline attributes
        */
        void SetPolylineAttr(map<unsigned int,PolylineAttr>& mPolylineAttr){m_polylinesAttr = mPolylineAttr;}

        /* Return value:
        ** default color when un-highlight
        */
        Uint32_t GetDefaultColor(){return m_defaultColor;}

        /* Return value:
        ** road width 
        */
        float GetRoadWidth(){return m_roadWidth;}

        /*Input parameters:
        **tileX is tile X index,
        **tileY is tile Y index,
        **Return value:
        **tileBuf is the buffer of one tile in one zLevel
        */
        NbmErrorCode GetTile(unsigned int tileX,unsigned int tileY,int zLevel,TileBuf& tileBuf); 

        /* Function:
        ** Set one polyline's data
        ** Input parameters:
        ** one polyline's data
        */
        void SetPolyline(PolylineComponent& polyline){m_polyline = polyline;}

        /* Function:
        ** Set all the polylines' data
        ** Input parameters:
        ** vector of all polylines' data
        */
        void SetPolylineList(vector<PolylineElement>& vPolyline){m_polylines = vPolyline;}

        /* Function:
        ** Multiple color polyline material
        ** Output parameter:
        ** highlightMaterial: Need to initialize outside, it's multiple color material for polylines
        ** normalMaterial:    Need to initialize outside, it's un-highlight material for polylines
        ** Return value:
        ** Error code for build material fail, NO_ERROR for return successfully
        */
        NbmErrorCode BuildSolidColorLineMaterial(TileBuf& highlightMaterial, TileBuf& defaultMaterial);

        /* Function:
        ** Build polyline pattern material
        ** Output parameter:
        ** highlightPatternMaterial: Need to initialize outside, it's line pattern material for highlight polyline
        ** defaultPatternMaterial: Need to initialize outside, it's line pattern material for unselected (default) polyline
        ** Return value:
        ** Error code for build material fail, NO_ERROR for return successfully
        */
        NbmErrorCode BuildBmpPatternMaterial(TileBuf& bmpPatternMaterial);

        /* Function:
        ** build end mark material
        ** Output parameter:
        ** endMarkMaterial: Need to initialize outside, it's the end mark material for end mark polylines
        ** Return value:
        ** Error code for build material fail, NO_ERROR for return successfully
        */
        NbmErrorCode BuildPolylineMaterial(TileBuf& polylineMaterial);

        /* Function:
        ** Generate all tiles in one zLevel 
        ** Input parameter: zLevel
        ** Return value: the count of tiles 
        */
        int BuildPolylinesTiles(int zLevel); 

        /* Function:
        ** reset all memory cache, and material memory
        */
        void Reset();

        /*These 4 functions below are not for interface of CCC Team or NBGM Team, just for TestNBMTile*/
        map< long long,vector<PolylineWithColorIdx> >& GetColorPolylineTileCache(){return m_oneColorPolylineTileCache;}
        map< long long,map< PolySubAttr,vector<Polyline> > >& GetPolylineTileCache(){return m_polylineTileCache;}       
        map<unsigned int,PolylineAttr>& GetPolylinesAttr(){return m_polylinesAttr;}

		void setNBMVersion(unsigned short iMajor=23, unsigned short iMinor=5) {m_NBMVersion = (unsigned short)(iMajor<<8|iMinor);}
		unsigned short getNBMVersion () {return m_NBMVersion;}

    private:
        int LoadColorPolylineData(int zLevel);
        int LoadPolylinesData(int zLevel);
        NbmErrorCode CreateColorPolylineNBMTiles(long long tileName, TileBuf& tileBuf);
        NbmErrorCode CreatePolylineNBMTiles(long long tileName, TileBuf& tileBuf);
        void AddColor2Polylines(vector<PolylineWithColorIdx>& vPL,vector<ColorPolyline>& vCPL);
        void PushColorPolylineDataToTile(long long tileName, PolylineWithColorIdx& polyline);
        void PushPolylineDataToTile(long long tileName, Polyline& polyline, PolySubAttr& polylineSubAttr);
    private:
        LineStyle m_colorPolylineStyle;
        vector<PolylineColor> m_vPolylineColor;
        vector<BmpPattern> m_vPolylinePattern;
        float m_roadWidth;
        Uint32_t m_defaultColor;
        map< long long, vector<PolylineWithColorIdx> > m_oneColorPolylineTileCache;
        Point m_colorStartPoint;
        Point m_colorEndPoint;
        map< long long,map< PolySubAttr,vector<Polyline> > > m_polylineTileCache;
        MaterialMgr m_matMgr;
        map<unsigned int,PolylineAttr> m_polylinesAttr;
        PolylineComponent m_polyline;
        vector<PolylineElement> m_polylines;
		unsigned short m_NBMVersion;
    };
}

