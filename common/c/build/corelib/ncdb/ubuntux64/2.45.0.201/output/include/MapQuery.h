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

    @file     mapquery.h
    @author   Gleb Panchishen
    @date     10/20/2009
    @defgroup MOBIUS_MAP  Mobius Map Draw API
    @brief    MapQuery class

    API for retreiving various information from map.
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef MAP_QUERY_H
#define MAP_QUERY_H

#include "NcdbTypes.h"
#include "AutoPtr.h"
#include "UtfString.h"
#include "Segment.h"
#include <string>

class Map;
class MapNode;
class MapLink;

namespace Ncdb {
class Session;
class NamesBinFileHandler;

struct ConnectedSegmentsInfo;

class NCDB_API MapQuery
{
public:

    enum ManeuverPermission {
        Allowed,
        Forbidden,
        DtmDriven
    };

    typedef AutoArray< ManeuverPermission > ManeuverPermissionList;

    MapQuery(Session&  session);

    ~MapQuery();

/*! @brief Searches street within radius

    Searches street within specified radius around specified center

    @return
            NCDB_OK - street and pronun key found
            NCDB_NOT_FOUND - no streets or pronun keys for this streets found
*/
    ReturnCode FindStreet(const WorldPoint& center, /*!< center of search */
        float radiusInMeters, /*!< radius of search */
        UtfString& foundStreetName, /*!< name of found street */
        UtfString& pronunKey /*!< pronun key of found street name */
        );

    ReturnCode FindStreetPhoneme(const WorldPoint& center, /*!< center of search */
        float radiusInMeters, /*!< radius of search */
        UtfString& foundStreetName, /*!< name of found street */
        UtfString& pronunText, /*!< phoneme text of found street name */
        UtfString& pronunDial, /*!< phoneme language dialect */
        int& pronunType /*!< phoneme type */
        );


/*! @brief Searches street by name

    Searches street within specified radius around specified center
    with specified name.

    @return
            NCDB_OK - street and pronun key found
            NCDB_NOT_FOUND - no streets or pronun keys for this streets found
*/
    ReturnCode FindStreet(const UtfString& streetName, /*!< name of needed street */
        const WorldPoint& center, /*!< center of search */
        float radiusInMeters, /*!< radius of search */
        UtfString& foundStreetName, /*!< name of found street */
        UtfString& pronunKey /*!< pronun key of found street name */
        );

    ReturnCode FindStreetPhoneme(const UtfString& streetName, /*!< name of needed street */
        const WorldPoint& center, /*!< center of search */
        float radiusInMeters, /*!< radius of search */
        UtfString& foundStreetName, /*!< name of found street */
        UtfString& pronunText, /*!< phoneme text of found street name */
        UtfString& pronunDial, /*!< phoneme language dialect */
        int& pronunType /*!< phoneme type */
        );

/*! @brief Gets object within distance

    Gets all obects around specified point within specified distance

    @return
            NCDB_OK - always
*/
    ReturnCode GetObjectInfoWithinDist(const WorldPoint& rQueryPoint, /*!< center of search */
        float distanceInMeters, /*!< radius of search */
        RoadSegmentIDList& rSegmentIDList /*!< list of found objects */
        );


/*! @brief Gets nearest object to specified point

    Gets nearest object to specified point within specified distance

    @return
            NCDB_OK - object found
            NCDB_NOT_FOUND - no objects found
*/
    ReturnCode GetNearestObject(const WorldPoint& rQueryPoint, /*!< center of search */
                                float distanceInMeters, /*!< max distance to search */
                                NodeSegmentID& rSegmentID /*!< found object */
                                );

/*! @brief Gets nearest InternalIntersection or RoundAbout link to specified point

    Gets nearest InternalIntersection or RoundAbout link to specified point

    @return
            NCDB_OK - object found
            NCDB_NOT_FOUND - no objects found
*/
    ReturnCode GetNearestIIRAObjects(   const WorldPoint& rQueryPoint, /*!< center of search */
                                        float distanceInMeters, /*!< max distance to search */
                                        AutoArray<WorldPointList>& wpList,  /*!< array of links points */
                                        RoadSegmentIDList& rdList /*!< found object */
                                    );

/*! @brief Gets nearest object to specified point

    Gets nearest object to specified point within specified distance

    @return
            NCDB_OK - object found
            NCDB_NOT_FOUND - no objects found
*/
    ReturnCode GetNearestObject(const WorldPoint& rQueryPoint, /*!< center of search */
        float distanceInMeters, /*!< max distance to search */
        NodeSegmentID& rSegmentID, /*!< found object */
        StreetSide& rStreetSide, /*!< street side of found object */
        int& rShapeIndex, /*!< shape index */
        WorldPoint& rPoint /*!< location of found object */
        );


/*! @brief Gets connected segments

    Gets list of connected segments

    @return
            NCDB_OK - successfully found all segments
            NCDB_NOT_FOUND - error while calculating needed node of segment

    @see ReturnCode GetConnectedSegments(const NodeSegmentID&, bool, RoadSegmentIDList&);
*/
    ReturnCode GetConnectedSegments(const NodeSegmentID& rSegmentID, /*!< segment for which connected segments are needed */
        bool bIsEndNode, /*!< segment side */
        RoadSegmentIDList& rSegmentIDList /*!< list, where found segments would be kept */
        );


/*! @brief Gets connected segments and maneuver restrictions

    Gets lists of connected segments, maneuver permissions and DTMs

    @return
            NCDB_OK - successfully found all segments
            NCDB_NOT_FOUND - error while calculating needed node of segment

    @see ReturnCode GetConnectedSegments(const NodeSegmentID&, bool, RoadSegmentIDList&);
*/
    ReturnCode GetConnectedSegments(const NodeSegmentID& rSegmentID, /*!< segment for which connected segments are needed */
        bool bIsEndNode, /*!< segment side */
        RoadSegmentIDList& rSegmentIDList, /*!< list, where found segments would be kept */
        MapQuery::ManeuverPermissionList& rManeuversPermissions, /*!< list of maneuver permissions for found segments */
        CDMsGroup& rCdmGroup/*!< list of CDM groups for found segments */
        );

    void        GetStreetPolyLine( const RoadSegmentIDList& streetSegments
                                 , WorldPointList& rStreetPolyLine );

/*! @brief Gets street polyline

    Gets polyline of all segments which are at the same street as specified segment

    @param rSegmentID - segment of needed street
    @param rStreetPolyLine [out] - street polyline

    @return 
            NCDB_OK - always
*/
    ReturnCode  GetStreetPolyLine( const NodeSegmentID& rSegmentID
                                 , WorldPointList& rStreetPolyLine );


/*! @brief Gets sign

    Gets sign text at specified segment

    @return
            NCDB_OK - no errors occured
            NCDB_NOT_FOUND - map link not found, map link info read error
*/
    ReturnCode GetSign(const NodeSegmentID& rSegmentID, /*!< segment */
        UtfString& sSign /*!< sign text */
        );
    ReturnCode GetSign(const NodeSegmentID& rSegmentID, /*!< segment */
        UtfString& sSign, /*!< sign text */
        bool& isExit
        );
    ReturnCode GetSign( const NodeSegmentID& rSegmentID, Ncdb::SegmentSign& segSign );
    
    ReturnCode  FindTmc( const char* tmc, NodeSegmentID& seg_id );

private:

    template< typename SEG_CALLBACK_PRM_T >
    ReturnCode GetConnectedSegments(const NodeSegmentID& rSegmentID,
        bool bIsEndNode,
        void (MapQuery::*ConnectedSegmentCallback)(const NodeSegmentID&, const NodeSegmentID &, bool &, SEG_CALLBACK_PRM_T&),
        SEG_CALLBACK_PRM_T& segCallbackPrm);

    void GetSegmentsOnly(const NodeSegmentID& connectedSegmentID,
        const NodeSegmentID &rSegmentId,
		bool &bRefNode,
        RoadSegmentIDList& rSegmentIDList);

    void GetSegmentsFullInfo(const NodeSegmentID& connectedSegmentID,
        const NodeSegmentID &rSegmentId,
		bool &bRefNode,
        ConnectedSegmentsInfo& connSegInfo);

    Map* GetMap();

    ReturnCode FindStreet2(const WorldPoint& center, float radiusInMeters,
        UtfString& foundStreetName, UtfString& pronunKey,
        UtfString* pronunDial = NULL, int* pronunType = NULL);

    ReturnCode FindStreet2(const UtfString& streetName,
        const WorldPoint& center, float radiusInMeters,
        UtfString& foundStreetName, UtfString& pronunKey,
        UtfString* pronunDial = NULL, int* pronunType = NULL);

    ReturnCode FindStreet(int mapId, const UtfString& streetName, UtfString& pronunKey,
        UtfString * pronunDial = NULL, int * pronunType = NULL);

    UtfString GetStreetName(int mapId, long filePosition, unsigned long& pronunidx,
        int nameIdx=0, char * pronunDial = NULL);

    /* See source file for description. */
    MapNode* GetConnectionNode(const NodeSegmentID& rSegment,
        bool bIsEndNode);

    /* See source file for description. */
    bool GetNodeSegmentByIndex(const NodeSegmentID &rSegmentId,
		bool &bRefNode,
        int index,
        NodeSegmentID& rSegment);

    /* See source file for description. */
    ReturnCode GetManeuverInfo(const NodeSegmentID& rFromSegmentID,
        const NodeSegmentID& rToSegment,
        const NodeSegmentID &rSegmentId,
		bool &bRefNode,
        ManeuverPermission& rManeuverPermission,
        CdmInfoList& rCdmList);

    void GetSegmentNodesWP(const NodeSegmentID& rSeg, WorldPoint& wp1, WorldPoint& wp2);

    ReturnCode GetStreetSegmentsDir(const NodeSegmentID& rSegmentID,
        RoadSegmentIDList& rSegmentIDList,
        bool dir);

    ReturnCode GetStreetSegments(const NodeSegmentID& rSegmentID,
        RoadSegmentIDList& rSegmentIDList);

    bool GetNextDir(const NodeSegmentID& rSeg1,
        const NodeSegmentID& rSeg2);

    ReturnCode GetGeoCodeData(  const WorldPoint& rQueryPoint,
                                float distanceInMeters,
                                void* rFoundData);

    ReturnCode GetGeoCodeIIRAData(  const WorldPoint& rQueryPoint,
                                    float distanceInMeters,
                                    RoadSegmentIDList& rdList);

private:
    MapQuery(const MapQuery&);

    MapQuery& operator =(const MapQuery&);
private:
    Session&                m_session;
    NamesBinFileHandler*    m_namesFile;
};

};

#endif //MAP_QUERY_H

/*! @} */
