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
    @file       GeographyLayerImpl.h

    Class GeographyLayerImpl inherits from GeographyLayer interface.

*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _GEOGRAPHYLAYERIMPL_H
#define _GEOGRAPHYLAYERIMPL_H

/*!
    @addtogroup nbmap
    @{
*/

#include "base.h"
#include "smartpointer.h"
#include "GeographyLayer.h"
#include "UnifiedLayer.h"
#include "TileInternal.h"
#include <string>
#include <vector>
#include <map>
#include "NBMFileHeader.h"

extern "C"
{
#include "nbcontextprotected.h"
}

namespace nbmap
{

class GeographyManager;

/*! */
class GeographyLayerImpl : public UnifiedLayer,
                           public GeographyLayer
{
public:
    // Public functions .........................................................................

    /*! GeographyLayerImpl constructor */
    GeographyLayerImpl(shared_ptr<string> layerID,                /*!< An ID to identify the layer */
                       uint32  layerIdDigital,                    /*!< Digital layer ID */
                       GeographyManager* geographyManager,        /*!< A pointer to a geography manager */
                       NB_Context* context                        /*!< NB_Context instance */
                       );

    /*! GeographyLayerImpl destructor */
    virtual ~GeographyLayerImpl();

    /* See description in Layer.h */

    virtual shared_ptr<string> GetTileDataType();

    virtual void GetTiles(vector<TileKeyPtr> tileKeys,
                         AsyncTileRequestWithRequestPtr callback,
                         uint32 priority);
    virtual NB_Error GetCachedTiles(const vector <TileKeyPtr>& tileKeys,
                                    AsyncTileRequestWithRequestPtr callback);
    virtual void RemoveAllTiles();

    /* See description in GeographyLayer.h */
    virtual NB_Error AddGeographyShape(GeographyShapePtr shape, int fitZoomLevel);
    virtual NB_Error RemoveGeographyShape(shared_ptr<std::string> id);

    /*! Check whether this GeographyLayer is in valid state.

        This function should be called right after creating GeographyLayerImpl. It checks whether
        associated TileLayerInfo is valid. If it is invalid, user should not use this
        GeographyLayerImpl.

        @return true if valid.
    */
    bool IsValid();

    virtual std::string className() const { return "GeographyLayerImpl"; }

private:

    /*! Convert lat and long to Tile's point(x, y). */
    void ConvertLatLongToTile(double latitude, double longitude, int& xTile, int& yTile);

    /*! Process add polyline shape */
    void ProcessAddPolylineShape(GeographyShapePtr shape, int fitZoomLevel);

    /*! Process add polygon shape */
    void ProcessAddPolygonShape(GeographyShapePtr shape, int fitZoomLevel);

    /*! Which tile can hold this polyline. Now we handle polyline
        in the only one tile, if involve multi-tile, we ignore it.

        @param PolylinePtr specify polyline.
        @param tileKeyPtr The tile info via polyline calculation
        @return if false, the error occur, this polyline not in the one tile.
    */
    bool ConvertPolylineToTile(PolylinePtr polylinePtr, TileKeyPtr tileKeyPtr);

    /*! Calculate the polyline boundary */
    void CalcLatLonBoundary(PolylinePtr polylinePtr, NB_LatitudeLongitude& leftTop, NB_LatitudeLongitude& rightBottom);

    /*! Check the polyline boundary in only one tile*/
    bool IsLatLogBoundaryInOneTile(NB_LatitudeLongitude& leftTop, NB_LatitudeLongitude& rightBottom,  TileKeyPtr tileKeyPtr);

    /*! After calculate, the shape can be held in one tile will add into container('m_geographyShapes')  */
    void AddShape(TileKeyPtr tileKeyPtr, GeographyShapePtr shape);

    void GetShapesInTile(TileKeyPtr tileKey, map<TileKey, vector<GeographyShapePtr> >& shapeMaps);

    /*! Notify to refresh shapes in the specified tiles

        @param tileKeys Specified the tiles to refresh
        @return None
    */
    void RefreshShapesOfTiles(const std::vector<TileKeyPtr>& tileKeys, GeographyType geographyType);
    TileKeyPtr ConvertTileKeyToLessZoomLevel(TileKeyPtr sourceTileKey, int targetZoomLevel);
    shared_ptr<string> GetContentID(const TileKey& tileKey, const vector<GeographyShapePtr>& shapes);
    void GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                 AsyncTileRequestWithRequestPtr callback);

    /*! Create geography tile data and return it via data stream.

        Specify one tile and indicate that how much shapes involve in it.
        After invoke this interface, we will get the related tile data.
    */
    NB_Error GenerateGeographyTileData(const TileKey& tileKey,                   /*!< Tile key of the tile */
                                       const vector<GeographyShapePtr>& shapes,  /*!< Shapes existing in the tile */
                                       nbcommon::DataStreamPtr& tileData         /*!< Return data of the geography tile */
                                       );

    /*! Generate NBM tile from polygons */
    NB_Error GeneratePolygonTile(NBMFileHeader& nbmTile,						/*!< In/Out: nbm tile to modify */
								 const vector<GeographyShapePtr>& shapes		/*!< Shapes to add to the nbm tile */
								 );

    /*! Generate NBM tile from polylines */
    NB_Error GeneratePolylineTile(NBMFileHeader& nbmTile,						/*!< In/Out: nbm tile to modify */
							 	  const vector<GeographyShapePtr>& shapes		/*!< Shapes to add to the nbm tile */
								  );

    // Private Members ...............................................................................................

    shared_ptr<string> m_layerID;              /*!< An ID to identify this layer */
    GeographyManager* m_geographyManager;

    /*! we will store the shape id and corresponding tile key during adding shapes.

        This map can optimize the shape removing.
     */
    map<string, TileKeyPtr> m_tileKeyIndexByShapeID;

    /*! Collection of added shapes.

        All shapes are organized by its TileKey.
     */
    map<TileKey, map<string, GeographyShapePtr> > m_geographyShapes;

};

};  // namespace nbmap

/*! @} */

#endif  // _GEOGRAPHYLAYERIMPL_H
