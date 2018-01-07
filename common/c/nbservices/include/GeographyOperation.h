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
    @file       GeographyOperation.h

    Interface to add/remove/update Geographic shapes to the map.
*/

/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.


    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _GEOGRAPHYOPERATION_H_
#define _GEOGRAPHYOPERATION_H_

#include "smartpointer.h"
#include "Layer.h"
#include <list>

namespace nbmap
{
// Types ........................................................................................
/*! Style to draw the edges of polygones */
enum GeographyEdgeStyle
{
    GEO_EDGE_NONE,
    GEO_EDGE_SOLID,
    GEO_EDGE_DOT,		// Currently not supported!
    GEO_EDGE_DASH		// Currently not supported!
};

enum GeographyType
{
    GEO_GRAPHIC_INVALID,
    GEO_GRAPHIC_POLYLINE,
    GEO_GRAPHIC_POLYGON
};

// Format is 0xRRGGBBAA
typedef uint32 GeographyColor;

/*! Material used for shapes */
struct GeographyMaterial
{
    GeographyMaterial(uint8 edgeSize,
                      GeographyEdgeStyle edgeStyle,
                      GeographyColor edgeColor,
                      GeographyColor fillColor)
    {
        m_edgeSize  = edgeSize;
        m_edgeStyle = edgeStyle;
        m_edgeColor = edgeColor;
        m_fillColor = fillColor;
    }

    uint8 m_edgeSize;                   // Edge in meters?
    GeographyEdgeStyle m_edgeStyle;
    GeographyColor m_edgeColor;
    GeographyColor m_fillColor;
};

struct GeographyCoordinate
{
    GeographyCoordinate(double latitude,
                        double longitude,
                        double altitude)
    {
        m_latitude = latitude;
        m_longitude = longitude;
        m_altitude = altitude;
    }

    double m_latitude;
    double m_longitude;
    double m_altitude;      // Currently not used
};

typedef shared_ptr<GeographyMaterial> MaterialPtr;

/*! A base class for all geographic shapes. */
class GeographyShape
{
public:
    GeographyShape(GeographyType type,
                   shared_ptr<std::string> id,
                   shared_ptr<std::string> name,
                   MaterialPtr materialPtr)
    {
        m_type = type;
        m_idPtr = id;
        m_namePtr = name;
        m_materialPtr = materialPtr;
    }
    virtual ~GeographyShape(){};

    GeographyType GetShapeType() const {return m_type;};
    shared_ptr<std::string> GetId() const {return m_idPtr;};
    shared_ptr<std::string> GetName() const {return m_namePtr;};
    MaterialPtr GetMaterial() const {return m_materialPtr;};

private:
    GeographyType m_type;
    shared_ptr<std::string> m_idPtr;       /*!< Shape id */
    shared_ptr<std::string> m_namePtr;     /*!< Shape name or description */
    MaterialPtr m_materialPtr;
};

typedef shared_ptr<std::list<GeographyCoordinate> > PolylinePtr;

/*! A polyline in geographic space that is represented by
    a set of ordered Points.
 */
class GeographyPolyline : public GeographyShape
{
public:
    GeographyPolyline(shared_ptr<std::string> id,
                      shared_ptr<std::string> name,
                      MaterialPtr material,
                      PolylinePtr polyline)
    :GeographyShape(GEO_GRAPHIC_POLYLINE, id, name, material),
     m_polylinePtr(polyline){}
    virtual ~GeographyPolyline(){};
    PolylinePtr GetPolyline() const {return m_polylinePtr;};

private:
    PolylinePtr m_polylinePtr;
};

typedef shared_ptr<std::list<shared_ptr<std::list<GeographyCoordinate> > > > PolylineListPtr;

/*! A polygon is an arbitrary object in geographic space that is
    defined by a sequence of coordinates.

    At a basic level, a simple polygon is defined as just an outer boundary.
    A more complex polygon can be created using an outer boundary with inner
    boundaries. All boundaries are closed-looped polylines.
 */
class GeographyPolygon : public GeographyShape
{
public:
    GeographyPolygon(shared_ptr<std::string> id,
                     shared_ptr<std::string> name,
                     MaterialPtr material,
                     PolylinePtr outerBoundary,
                     PolylineListPtr innerBoundaries) /*!< innerBoundaries are currently not supported */
    :GeographyShape(GEO_GRAPHIC_POLYGON, id, name, material),
     m_outerBoundaryPtr(outerBoundary),
     m_innerBoundariesPtr(innerBoundaries){}
    virtual ~GeographyPolygon(){};

    PolylinePtr GetOuterBoundary() const {return m_outerBoundaryPtr;};
    PolylineListPtr GetInnerBoundaries() const {return m_innerBoundariesPtr;};

private:
    PolylinePtr m_outerBoundaryPtr;
    PolylineListPtr m_innerBoundariesPtr;   /*!< Maybe have multi-inner-boundaries */
};

typedef shared_ptr<GeographyShape> GeographyShapePtr;

/*! Optional layer used to add geographic shapes.
    This layer should only be added if geographic shapes are added.
 */

class GeographyOperation
{
public:
    // Public functions .........................................................................
    GeographyOperation(){};

    virtual ~GeographyOperation(){};

    /*! Add a geographic shape to the layer
        @return NE_OK on success
     */
    virtual NB_Error AddGeographyShape(GeographyShapePtr shape) = 0;

    /*! Remove a geographic shape from the layer
        @return NE_NOENT if the entry doesn't exist, NE_OK if the item was removed.
     */
    virtual NB_Error RemoveGeographyShape(shared_ptr<std::string> id) = 0;


    /*! Update a geographic shape from the layer
        @return NE_NOENT if the entry doesn't exist, NE_OK if the item was updated.
    */
    virtual NB_Error UpdateGeographyShape(GeographyShapePtr shape) = 0;
};

typedef shared_ptr<GeographyOperation> GeographyOperationPtr;


};  // namespace nbmap

/*! @} */
#endif /* _GEOGRAPHYOPERATION_H_ */
