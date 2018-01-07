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

/*
    (C) Copyright 2015 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _DATA_TRANSFORMER_H_
#define _DATA_TRANSFORMER_H_

#include <QObject>
#include <QJsonObject>
#include <QMultiMap>
#include <QByteArray>
#include <navigation.h>
#include <searchdatatypes.h>
#include <mapwidget.h>


namespace locationtoolkit
{
class DataTransformer
{
public:
    //common
    static bool fromJson(const QJsonObject& data, Coordinates& coordinate);
    //nav
    static QString dataFileBasePath;
    static QString navFilekey;
    /** toJson will convert data struct to an json object
     *
     * @param data the data struct
     * @value value a json boject
     * @return if convert success will return true else will return false
     */
    static bool toJson(const Coordinates& data, QJsonObject& value);
    static bool toJson(const RoadSign& data, QJsonObject& value);
    static bool toJson(const ManeuverList& data, QJsonArray &value);
    static bool toJson(const locationtoolkit::LaneInformation& data, QJsonObject& value);
    static bool toJson(const SpeedLimitInformation& data, QJsonObject& value);
    static bool toJson(const Maneuver& data, QJsonObject& value);
    static bool toJson(const Lane& data, QJsonObject& value);
    static bool toJson(const RouteInformation&data, QJsonObject& value);
    static bool toJson(const Place& data, QJsonObject& value);
    static bool toJson(const BoundingBox& data, QJsonObject& value);
    static bool toJson(const TrafficEvent&data, QJsonObject& value);
    static bool toJson(const MapLocation&data, QJsonObject& value);
    static bool toJson(const SpecialRegionInformation& data, QJsonObject& value);
    static bool toJson(const Announcement& data, QJsonObject& value);
    static bool toJson(const QVector<RouteInformation::RouteProperty>&data, QJsonArray& value);
    static bool toJson(const QVector<Coordinates*>& data, QJsonArray& value);
    /** fromJson will convert jason object to a data struct.
     * @return if convert success will return true, else return false
     */
    static bool fromJson(const QJsonObject& data, Place& place);
    static bool fromJson(const QJsonObject& data, MapLocation& location);
    static bool fromJson(const QJsonObject& data, RouteInformation& location);
    static bool fromJson(const QJsonObject& data, BoundingBox& value);
    /** save data to a system file
     * this function will save data to a file and keep track this file by the key
     *
     * @param data the data to be saved
     * @param key the data's key, user can use key to ident the data
     * @param path the data file's full path after save data.
     * @return if success save data return true else return fasle
     */
    static bool saveDataToFile(const QByteArray data, QString key, QString& path);
    /** get data from a system file
     * this function will get data from the file
     *
     * @param data output file data
     * @param path the data file's full path.
     * @return if get data success return true else return fasle
     */
    static bool getDataFromFile(QByteArray& data, QString path);
    /** delete saved file by key
     *
     * @return if success will return true
     **/
    static bool deleteDataFile(QString key);
    /** delete all saved data file. */
    static bool deleteDataFile();
    //search
    static bool toJson(const SearchInformation& information, QJsonObject& value);
    static bool fromJson(const QJsonObject& data, SearchLocation& location);

    //for map
    static bool fromJson(const QJsonObject& data, QPoint& pos);
    static bool fromJson(const QJsonObject& data, QRect& geometry);
    static bool fromJson(const QJsonObject& data, PolylineParameters& parameters);
    static bool fromJson(const QJsonObject& data, locationtoolkit::Location& location);
    static bool fromJson(const QJsonObject& data, locationtoolkit::PinParameters& pinParameters);
    static bool fromJson(const QJsonArray& data, QMap<QString, Coordinates>& coordinates);

    static bool toJson(const CameraParameters& camera, QJsonObject& data);

private:
    static bool fromJson(const QJsonValue& data, SearchLocationType& type);
    static bool fromJson(const QJsonObject& data, SearchCompactAddress& address);
    static bool toJson(const QList<SearchResultDescription> & information, QJsonObject& value);
    static bool toJson(const SearchResultInformationType& type, QJsonObject& value);
    static bool toJson(const SearchFuelSummary& data, QJsonObject& value);
    static bool toJson(const SearchFuelProduct& data, QJsonObject& value, const QString& key);
    static bool toJson(const SearchFuelPrice& data, QJsonObject& value);
    static bool toJson(const SearchFuelType& data, QJsonObject& value);
    static bool toJson(const SearchSuggestion& data, QJsonObject& value);
    static bool toJson(const SearchPOI& data, QJsonObject& value);
    static bool toJson(const SearchPOIType& data, QJsonObject& value);
    static bool toJson(const SearchPlace& data, QJsonObject& value);
    static bool toJson(const SearchLocation& data, QJsonObject& value);
    static bool toJson(const SearchLocationType& data, QJsonObject& value);
    static bool toJson(const QList<QSharedPointer<SearchCategory> >& data, QJsonObject& value);
    static bool toJson(const QList<SearchPhone>& data, QJsonObject& value);
    static bool toJson(const SearchPhoneType& data, QJsonObject& value);
    static bool toJson(const SearchPOIContent& data, QJsonObject& value);
    static bool toJson(const QList<SearchPOIContentKeyValuePair>& data, QJsonObject& value);
    static bool toJson(const SearchPOIConentKey& data, QJsonObject& value);
    static bool toJson(const QList<SearchVendorContent> & data, QJsonObject& value);
    static bool toJson(const QList<SearchStringPair> & data, QJsonObject& value);
    static bool toJson(const SearchFuelDetails& data, QJsonObject& value);
    static bool toJson(const SearchSuggestionType& data, QJsonObject& value);
    static bool toJson(const SearchCompactAddress& data, QJsonObject& value);

    //for map
    static bool fromJson(const QJsonArray& data, QList<Coordinates>& points);
    static bool fromJson(const QJsonArray& data, QList<SegmentAttribute*>& segmentAttributes, PolylineParameters::SegmentAttributeType& type);
    static bool fromJson(const QJsonObject& data, QColor& color);
    static bool fromJson(const QJsonObject& data, CapParameter& cap);
    static bool fromJson(const QJsonObject& data, locationtoolkit::PinImageInfo& pinImageInfo);
    static bool fromJson(const QJsonObject& data, locationtoolkit::RadiusParameters& radiusParameters);
    static bool fromJson(const QJsonObject& data, int& color);

private:
    static QMultiMap<QString,QString> fileData;

};
}

#endif // _DATA_TRANSFORMER_H_
