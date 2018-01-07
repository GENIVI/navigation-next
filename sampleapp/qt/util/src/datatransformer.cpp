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
#include "datatransformer.h"
#include <QJsonArray>
#include <QUuid>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QJsonDocument>
#include <QBuffer>
#include <QImage>
#include "lane.h"
#include <QDebug>
#include "patternsegment.h"


namespace locationtoolkit
{

QString DataTransformer::dataFileBasePath = "/tmp/tcs/";
QString DataTransformer::navFilekey = "nav";
QMultiMap<QString,QString> DataTransformer::fileData;

bool DataTransformer::toJson(const Coordinates& data, QJsonObject& value)
{
    value["latitude"] = data.latitude;
    value["longitude"] = data.longitude;
    return true;
}

bool DataTransformer::toJson(const RoadSign& data, QJsonObject& value)
{
    value["landscape_information"] = data.SignInformation();
    value["portrait_sign_information"] = data.PortraitSignInformation();
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    data.GetImageData().save(&buffer, "PNG");
    QString path;
    if(saveDataToFile(ba, navFilekey, path))
    {
        value["landscape_image"] = path;
    }
    data.GetPortraitImageData().save(&buffer,"PNG");
    if(saveDataToFile(ba, navFilekey, path))
    {
        value["portrait_image_data"] = path;
    }
    return true;
}

bool DataTransformer::toJson(const ManeuverList& data, QJsonArray& value)
{
    for(int i = 0; i < data.GetNumberOfManeuvers(); i++)
    {
        QJsonObject maneuver;
        if(DataTransformer::toJson(*data.GetManeuver(i), maneuver))
        {
            value.append(maneuver);
        }
    }
    return true;
}

bool DataTransformer::toJson(const locationtoolkit::LaneInformation& data, QJsonObject& value)
{
    value["selectedLanePosition"] = (int)data.SelectedLanePosition();
    value["selectedLanes"] = (int)data.SelectedLanes();
    QJsonArray lanes;
    for(int i = 0; i < data.GetLanes().size(); i++)
    {
        QJsonObject lane;
        if(toJson(*(data.GetLanes()[i]), lane))
        {
            lanes.append(lane);
        }
    }
    value["lanes"] = lanes;
    return true;
}

bool DataTransformer::toJson(const locationtoolkit::Lane& data, QJsonObject& value)
{
    value["high_lighted"] = data.GetHighlighted();
    value["non_highlighted"] = data.GetNonHighlighted();
    return true;
}

bool DataTransformer::toJson(const SpeedLimitInformation& data, QJsonObject& value)
{
    value["type"] = data.Type();
    value["warning"] = data.IsWarning();
    value["speed_limit"] = data.SpeedLimit();
    value["current_speed"] = data.CurrentSpeed();
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    data.Information().save(&buffer, "PNG");
    QString path;
    if(saveDataToFile(ba, navFilekey, path))
    {
        value["image"] = path;
    }
    return false;
}

bool DataTransformer::toJson(const Maneuver& data, QJsonObject& value)
{
    value["command"] = data.GetCommand();
    value["distance"] = data.GetDistance();
    QJsonArray polyline;
    if(toJson(data.GetPolyline(), polyline))
    {
        value["polyline"] = polyline;
    }
    value["id"] = data.GetManeuverID();
    value["routing_TTF"] = data.GetRoutingTTF();
    value["primary_street"] = data.GetPrimaryStreet();
    value["secondary_street"] = data.GetSecondaryStreet();
    value["time"] = data.GetTime();
    QJsonObject point;
    if(DataTransformer::toJson(data.GetPoint(), point))
    {
        value["point"] = point;
    }
    value["description"] = data.GetDescription(true);
    value["traffic_delay"] = data.GetTrafficDelay();
    value["stack_advise"] = data.GetStackAdvise();
    value["exit_number"] = data.GetExitNumber();
    value["is_destination"] = data.IsDestination();
    return true;
}

bool DataTransformer::toJson(const RouteInformation& data, QJsonObject& value)
{
    QJsonArray maneuvers;
    if(DataTransformer::toJson(*data.GetManeuverList(), maneuvers))
    {
        QString path;
        QJsonDocument doc(maneuvers);
        if(DataTransformer::saveDataToFile(doc.toJson(), navFilekey, path))
        {
            value["maneuver_list"] = path;
        }
    }
    value["id"] = data.GetRouteID();
    value["distance"] = data.GetDistance();
    value["time"] = (int)data.GetTime();
    value["delay"] = (int)data.GetDelay();
    QJsonObject origin;
    if(DataTransformer::toJson(data.GetOrigin(), origin))
    {
        value["origin"] = origin;
    }
    QJsonObject destination;
    if(DataTransformer::toJson(data.GetDestination(), destination))
    {
        value["destination"] = destination;
    }
    QJsonObject box;
    if(DataTransformer::toJson(data.GetBoundingBox(), box))
    {
        value["bounding_box"] = box;
    }
    QJsonArray polyline;
    if(toJson(data.GetPolyline(), polyline))
    {
        QString path;
        QJsonDocument doc(polyline);
        if(saveDataToFile(doc.toJson(),navFilekey,path))
        {
            value["polyline"] = path;
        }
    }
    value["initial_guidance_text"] = data.GetInitialGuidanceText();
    value["descriptions"] = data.GetRouteDescriptions();
    QJsonArray properties;
    if(DataTransformer::toJson(data.GetRouteProperties(), properties))
    {
        value["properties"] = properties;
    }
    QJsonArray traffics;
    quint32 trafficCount = data.GetNumberOfTrafficEvent();
    for(quint32 i = 0; i< trafficCount; i++)
    {
        QJsonObject traffic;
        if(DataTransformer::toJson(*data.GetTrafficEvent(i), traffic))
        {
            traffics.append(traffic);
        }
    }
    if(traffics.size() > 0)
    {
        value["traffic_event"] = traffics;
    }
    value["traffic_color"] = data.GetTrafficColor();
    value["startup_guidance_text"] = data.GetStartupGuidanceText();
    value["destination_street_side"] = data.GetDestinationStreetSide();
    value["first_major_road"] = data.GetFirstMajorRoad();
    QJsonObject point;
    if(DataTransformer::toJson(data.GetLabelPoint(), point))
    {
        value["label_point"] = point;
    }
    return true;
}

bool DataTransformer::toJson(const Place& data, QJsonObject& value)
{
    return DataTransformer::toJson(data.GetLocation(), value);
}

bool DataTransformer::toJson(const BoundingBox& data, QJsonObject& value)
{
    QJsonObject point1;
    QJsonObject point2;
    if(DataTransformer::toJson(data.point1, point1))
    {
        value["point1"] = point1;
    }
    else
    {
        return false;
    }
    if(DataTransformer::toJson(data.point2, point2))
    {
        value["point2"] = point2;
    }
    else
    {
        return false;
    }
    return true;
}

bool DataTransformer::toJson(const TrafficEvent& data, QJsonObject& value)
{
    value["delay"] = (int)data.GetDelay();
    value["description"] = data.GetDescription();
    value["end_time"] = data.GetEndTime().toString();
    value["distance_to_traffic_item"] = data.GetDistanceToTrafficItem();
    value["length"] = data.GetLength();
    value["maneuver_number"] = (int)data.GetManeuverNumber();
    value["report_time"] = data.GetReportTime().toString();
    value["road_name"] = data.GetRoadName();
    value["severity"] = data.GetSeverity();
    value["start_from_trip"] = data.GetStartFromTrip();
    value["type"] = data.GetType();
    return true;
}

bool DataTransformer::toJson(const MapLocation &data, QJsonObject& value)
{
    value["city"] = data.city;
    value["country"] = data.country;
    value["latitude"] = data.center.latitude;
    value["longitude"] = data.center.longitude;
    value["name"] = data.freeform;
    value["street"] = data.street;
    value["number"] = data.number;
    value["postalCode"] = data.postal;
    return true;
}

bool DataTransformer::toJson(const SpecialRegionInformation& /*data*/, QJsonObject& /*value*/)
{
    return false;
}

bool DataTransformer::toJson(const Announcement& data, QJsonObject& value)
{
    value["text"] = data.GetText();
    value["mime_type"] = data.GetAudioMimeType();
    QString path;
    const QByteArray& audio = data.GetAudio();
    if(audio.size() > 0 && saveDataToFile(audio, navFilekey, path))
    {
        value["audio"] = path;
    }
    return true;
}

bool DataTransformer::toJson(const QVector<RouteInformation::RouteProperty>&data, QJsonArray &value)
{
    for(int i = 0; i< data.size(); i++)
    {
        value.append(data[i]);
    }
    return true;
}

bool DataTransformer::toJson(const QVector<Coordinates*>& data, QJsonArray& value)
{
    for(int i = 0; i< data.size(); i++)
    {
        QJsonObject coord;
        if(DataTransformer::toJson(*data[i], coord))
        {
            value.append(coord);
        }
    }
    return true;
}

bool DataTransformer::fromJson(const QJsonObject& data, Place& place)
{
    MapLocation location;
    QJsonValue value = data["latitude"];
    if(value.isDouble())
    {
        location.center.latitude = value.toDouble();
    }
    value = data["longitude"];
    if(value.isDouble())
    {
        location.center.longitude = value.toDouble();
    }
    value = data["city"];
    if(value.isString())
    {
        location.city = value.toString();
    }
    value = data["country"];
    if(value.isString())
    {
        location.country = value.toString();
    }
    value = data["name"];
    if(value.isString())
    {
        place.SetName(value.toString());
    }
    value = data["number"];
    if(value.isString())
    {
        location.number = value.toString();
    }
    value = data["postalCode"];
    if(value.isString())
    {
        location.postal = value.toString();
    }
    value = data["street"];
    if(value.isString())
    {
        location.street = value.toString();
    }
    place.SetLocation(location);
    return true;
}

bool DataTransformer::fromJson(const QJsonObject& /*data*/, MapLocation& /*location*/)
{
    return false;
}

bool DataTransformer::fromJson(const QJsonObject& /*data*/, RouteInformation& /*location*/)
{
    return false;
}

bool DataTransformer::fromJson(const QJsonObject& data, BoundingBox& value)
{
    bool ret = true;
    QJsonObject jsonPoint1 = data["point1"].toObject();
    QJsonObject jsonPoint2 = data["point2"].toObject();
    ret = ret && DataTransformer::fromJson(jsonPoint1, value.point1);
    ret = ret && DataTransformer::fromJson(jsonPoint2, value.point2);
    return ret;
}


bool DataTransformer::saveDataToFile(const QByteArray data, QString key, QString& path)
{
    bool ret = false;
    QDir dir(dataFileBasePath);
    if(!dir.exists())
    {
        dir.mkdir(dataFileBasePath);
    }
    //get a file name
    QUuid id = QUuid::createUuid();
    path = dataFileBasePath + id.toString();
    QFile file(path);
    if(file.open(QFile::WriteOnly))
    {
        file.write(data);
        file.close();
        fileData.insert(key,path);
        ret = true;
    }
    path = "file://" + path;
    return ret;
}

bool DataTransformer::getDataFromFile(QByteArray& data, QString path)
{
    bool ret = false;
    int index = path.indexOf("://");
    QString protocol = path.left(index);
    if(protocol == "file")
    {
        path = path.mid(index + 3);
        QFile file(path);
        if(file.open(QFile::ReadOnly))
        {
           data = file.readAll();
           ret = data.count() > 0 ? true : false;
        }
        else
        {
            qDebug()<<"DataTransformer::getDataFromFile failed, file path:"<<path;
        }
    }
    else
    {
        qDebug()<<"unsupported protocol:"<<protocol;
    }
    return ret;
}

bool DataTransformer::deleteDataFile(QString key)
{
    bool ret = true;
    QMap<QString,QString>::iterator result = fileData.find(key);
    while(result != fileData.end())
    {
        QFile file(result.value());
        ret = file.remove();
        result++;
    }
    fileData.remove(key);
    return ret;
}

bool DataTransformer::deleteDataFile()
{
    fileData.clear();
    QDir dir(dataFileBasePath);
    return dir.removeRecursively();
}

bool DataTransformer::fromJson(const QJsonObject& data, SearchLocation& location)
{
    bool ret = false;
    if(data["city"].isString())
    {
        location.city = data["city"].toString();
        ret = true;
    }

    if(data["country"].isString())
    {
        location.country = data["country"].toString();
        ret = true;
    }

    if(data["name"].isString())
    {
        location.name = data["name"].toString();
        ret = true;
    }

    if(data["number"].isString())
    {
        location.streetNumer = data["number"].toString();
        ret = true;
    }

    if(data["postal_code"].isString())
    {
        location.postal = data["postal_code"].toString();
        ret = true;
    }

    if(data["province"].isString())
    {
        location.state = data["province"].toString();
        ret = true;
    }

    if(data["street"].isString())
    {
        location.street = data["street"].toString();
        ret = true;
    }

    if(data["cross_street"].isString())
    {
        location.crossStreet = data["cross_street"].toString();
        ret = true;
    }

    if(data["county"].isString())
    {
        location.county = data["county"].toString();
        ret = true;
    }

    if(data["country_name"].isString())
    {
        location.countryName = data["country_name"].toString();
        ret = true;
    }

    if(data["airport"].isString())
    {
        location.airport = data["airport"].toString();
        ret = true;
    }

    if(data["free_form"].isString())
    {
        location.freeForm = data["free_form"].toString();
        ret = true;
    }

    QJsonObject extended_address_obj = data["extended_address"].toObject();
    QJsonArray extended_address_array = extended_address_obj["address_lines"].toArray();
    int extendedadd_size = extended_address_array.size();
    if(extendedadd_size > 0)
    {
        ret = true;
        for(int i = 0; i < extendedadd_size; i++)
        {
            location.extendedAddress.addressLines.append(extended_address_array[i].toString());
        }
    }

    if(DataTransformer::fromJson( data["compact_address"].toObject(), location.compactAddress))
    {
        ret = true;
    }

    if(DataTransformer::fromJson( data["type"], location.type))
    {
        ret = true;
    }

    return ret;
}

bool DataTransformer::fromJson(const QJsonObject& data, SearchCompactAddress& compactAddress)
{
    bool ret = false;
    if(data["line_separator"].isString())
    {
        compactAddress.lineSeparator = data["line_separator"].toString();
        ret = true;
    }

    QJsonArray compact_address_array = data["address_lines"].toArray();
    int compactadd_size = compact_address_array.size();
    if(compactadd_size > 0)
    {
        ret = true;
        for(int i = 0; i < compactadd_size; i++)
        {
            compactAddress.addressLines.append(compact_address_array[i].toString());
        }
    }

    if(data["formatted_line"].isString())
    {
        compactAddress.formattedLine = data["formatted_line"].toString();
        ret = true;
    }

    return ret;
}

bool DataTransformer::fromJson(const QJsonValue& data, SearchLocationType& type)
{
    //use address as its default value.
    type = SLT_Address;
    if(!data.isString())
    {
         qDebug()<<"SearchLocationType fromJson error, data is not string";
         return false;
    }
    QJsonObject type_obj;
    for(int i = SLT_None; i <= SLT_AddressFreeForm; i++)
    {
        DataTransformer::toJson((SearchLocationType)i, type_obj);
        if(type_obj.value("type") == data)
        {
            type = (SearchLocationType)i;
            return true;
        }
    }
    qDebug()<<"SearchLocationType fromJson error, no matched string: "<<data.toString();
    return false;
}

bool DataTransformer::toJson(const SearchInformation& information, QJsonObject& value)
{
    bool ret = true;
    ret = ret&&DataTransformer::toJson(information.type, value);
    value["has_more"] = information.hasMore;
    ret = ret&&DataTransformer::toJson(information.descriptions, value);
    if(information.fuelSummary)
    {
        ret = ret&&DataTransformer::toJson(*(information.fuelSummary), value);
    }

    QJsonArray result_array;
    for(int i = 0; i < information.results.count(); i++)
    {
        bool ret_search_results = false;
        QJsonObject result_obj;
        switch (information.results[i]->searchResultType)
        {
            case SRT_POI:
                {
                    SearchPOI* pSearchPOI = static_cast<SearchPOI*> (information.results[i].data());
                    ret_search_results = DataTransformer::toJson(*pSearchPOI, result_obj);
                }
                break;
            case SRT_Suggest:
                {
                    SearchSuggestion* pSuggestion = static_cast<SearchSuggestion* >(information.results[i].data());
                    ret_search_results = DataTransformer::toJson(*pSuggestion, result_obj);
                }
                break;
            default:
                {
                    qDebug()<<"SearchInformation toJson error, results["<<i<<"] type is "<<information.results[i]->searchResultType;
                }
                break;
        }
        if(ret_search_results)
        {
            result_array.push_back(result_obj);
        }
        else
        {
            qDebug()<<"SearchResult["<<i<<"] toJson error, SearchResultType is "<<information.results[i]->searchResultType;
        }
    }

    value["results"] = result_array;
    return ret;
}

bool DataTransformer::toJson(const SearchResultInformationType& type, QJsonObject& value)
{
    bool ret = true;
    QString resultsType;
    switch (type)
    {
        case SRIT_None:
            resultsType = "none";
            break;
        case SRIT_POI:
            resultsType = "POI";
            break;
        case SRIT_Suggestion:
            resultsType="suggestion";
            break;
        case SRIT_SuggestionList:
            resultsType="suggestion_list";
            break;
        case SRIT_Theater:
            resultsType="theater";
            break;
        case SRIT_Fuel:
            resultsType="fuel";
            break;
        case SRIT_Movie:
        case SRIT_MovieShowing:
        default:
            ret = false;
            break;
    }
    if(ret)
    {
        value["type"]= resultsType;
    }
    else
    {
        qDebug()<<"SearchResultInformationType toJson error, no matched type: "<<type;
    }

    return ret;
}

bool DataTransformer::toJson(const QList<SearchResultDescription> & data, QJsonObject& value)
{
    QJsonArray descriptions_array;
    for(int i = 0; i < data.count(); i++)
    {
        descriptions_array.push_back(QJsonArray::fromStringList(data[i].lines));
    }

    if(descriptions_array.size() > 0)
    {
        value["descriptions"] = descriptions_array;
    }
    return true;
}

bool DataTransformer::toJson(const SearchFuelSummary& data,QJsonObject& value)
{
    bool ret = true;
    QJsonObject fuel_summary_obj;
    ret = ret&&DataTransformer::toJson(data.average, fuel_summary_obj, "average");
    ret = ret&&DataTransformer::toJson(data.low, fuel_summary_obj, "low");
    value["fuel_summary"] = fuel_summary_obj;
    return ret;
}

bool DataTransformer::toJson(const SearchFuelProduct& data, QJsonObject& value, const QString& key)
{
    bool ret = true;
    QJsonObject fuelproduct_obj;
    ret = ret&&DataTransformer::toJson(data.price, fuelproduct_obj);
    ret = ret&&DataTransformer::toJson(data.type, fuelproduct_obj);
    fuelproduct_obj["units"] = data.units;
    if(ret)
    {
        if(key == "")
        {
            value = fuelproduct_obj;
        }
        else
        {
           value[key]= fuelproduct_obj;
        }
    }
    else
    {
        qDebug()<<"SearchFuelProduct toJson error";
    }

    return ret;
}

bool DataTransformer::toJson(const SearchFuelPrice& data,QJsonObject& value)
{
    QJsonObject price_obj;
    price_obj["modTime"] = data.modTime;
    price_obj["value"] = data.value;
    price_obj["currency"] = data.currency;
    value["price"] = price_obj;
    return true;
}

bool DataTransformer::toJson(const SearchFuelType& data,QJsonObject& value)
{
    QJsonObject fueltype_obj;
    fueltype_obj["name"] = data.productName;
    fueltype_obj["code"] = data.code;
    fueltype_obj["type"] = data.typeName;
    value["type"] = fueltype_obj;
    return true;
}

bool DataTransformer::toJson(const SearchPOIType& data,QJsonObject& value)
{
    bool ret = true;
    QString strtype;
    switch(data)
    {
        case SPOIT_Locaton:
            strtype = "location";
            break;
        case SPOIT_POI:
            strtype = "POI";
            break;
        default:
            ret = false;
            break;
    }
    if(!ret)
    {
        qDebug()<<"SearchPOIType toJson error, SearchPOIType is "<<data;
    }
    else
    {
        value["type"] = strtype;
    }

    return ret;
}

bool DataTransformer::toJson(const SearchPlace& data,QJsonObject& value)
{
    bool ret = true;
    QJsonObject place_obj;
    place_obj["name"] = data.name;
    ret = ret&&DataTransformer::toJson(data.location, place_obj);
    ret = ret&&DataTransformer::toJson(data.phones, place_obj);
    ret = ret&&DataTransformer::toJson(data.categories, place_obj);
    place_obj["cookie"] = data.cookie;

    if(ret)
    {
        value["place"] = place_obj;
    }
    else
    {
        qDebug()<<"SearchPlace toJson error";
    }

    return ret;
}

bool DataTransformer::toJson(const SearchCompactAddress& data, QJsonObject& value)
{
    QJsonObject compactAddress_obj;
    compactAddress_obj["line_separator"] = data.lineSeparator;
    compactAddress_obj["address_lines"] = QJsonArray::fromStringList(data.addressLines);
    compactAddress_obj["formatted_line"] = data.formattedLine;
    value["compact_address"] = compactAddress_obj;
    return true;
}

bool DataTransformer::fromJson(const QJsonArray& data, QList<Coordinates>& points)
{
    bool ret = false;
    int pointsSize = data.size();
    for(int i = 0; i < pointsSize; i++)
    {
        QJsonObject jsonCoordinate = data[i].toObject();
        QJsonValue jsonLatitude = jsonCoordinate["latitude"];
        QJsonValue jsonLongitude = jsonCoordinate["longitude"];
        if(jsonLatitude.isDouble() && jsonLongitude.isDouble())
        {
            Coordinates coordinates;
            coordinates.latitude = jsonLatitude.toDouble();
            coordinates.longitude = jsonLongitude.toDouble();
            if((coordinates.latitude>=-90. && coordinates.latitude<=90. ) &&
                    (coordinates.longitude>=-180. && coordinates.longitude<=180.))
            {
                points.append(coordinates);
            }
        }
    }

    if(!points.isEmpty())
    {
        ret = true;
    }

    return ret;
}


bool DataTransformer::fromJson(const QJsonArray &data, QMap<QString, Coordinates> &coordinates)
{
    bool succeed = false;
    for(int i = 0; i < data.count(); i++)
    {
        QJsonObject jsonSearchResult = data[i].toObject();
        QJsonObject jsonPlace = jsonSearchResult["place"].toObject();
        QJsonObject jsonLocation = jsonPlace["location"].toObject();
        QJsonValue jsonLatitude = jsonLocation["latitude"];
        QJsonValue jsonLongitude = jsonLocation["longitude"];
        if(jsonLatitude.isDouble() && jsonLongitude.isDouble())
        {
            Coordinates coordinate;
            coordinate.latitude = jsonLatitude.toDouble();
            coordinate.longitude = jsonLongitude.toDouble();
            coordinates.insert(QString::number(i), coordinate);
            succeed = true;
        }
    }
    return succeed;
}


bool DataTransformer::fromJson(const QJsonObject& data, QColor& color)
{
    QJsonValue jsonRed = data["red"];
    QJsonValue jsonGreen = data["green"];
    QJsonValue jsonBlue= data["blue"];
    QJsonValue jsonAlpha= data["alpha"];
    bool ret = jsonRed.isDouble() && jsonGreen.isDouble() && jsonBlue.isDouble();
    if(ret)
    {
        color.setRed(jsonRed.toDouble());
        color.setGreen(jsonGreen.toDouble());
        color.setBlue(jsonBlue.toDouble());
        color.setAlpha(jsonAlpha.toDouble());
    }
    else
    {
        qDebug()<<"Invalid QColor";
    }
    return ret;
}

bool DataTransformer::toJson(const SearchLocation& data,QJsonObject& value)
{
    bool ret = true;
    QJsonObject location_obj;
    location_obj["city"] = data.city;
    location_obj["country"] = data.country;
    location_obj["latitude"] = data.coordinate.latitude;
    location_obj["longitude"] = data.coordinate.longitude;
    location_obj["name"] = data.name;
    location_obj["number"] = data.streetNumer;
    location_obj["province"] = data.state;
    location_obj["postal_code"] = data.postal;
    location_obj["street"] = data.street;
    location_obj["cross_street"] = data.crossStreet;
    location_obj["county"] = data.county;
    location_obj["country_name"] = data.countryName;
    location_obj["airport"] = data.airport;
    location_obj["free_form"] = data.freeForm;
    QJsonObject address_lines_obj;
    address_lines_obj["address_lines"] = QJsonArray::fromStringList(data.extendedAddress.addressLines);
    location_obj["extended_address"] = address_lines_obj;
    ret = ret&&DataTransformer::toJson(data.compactAddress, location_obj);
    ret = ret&&DataTransformer::toJson(data.type, location_obj);

    if(ret)
    {
        value["location"] = location_obj;
    }
    else
    {
        qDebug()<<"SearchLocation toJson error";
    }

    return ret;
}

bool DataTransformer::toJson(const SearchLocationType& data,QJsonObject& value)
{
    bool ret = true;
    QString strtype;
    switch(data)
    {
        case SLT_None:
            strtype = "none";
            break;
        case SLT_Address:
            strtype = "address";
            break;
        case SLT_Intersection:
            strtype = "intersection";
            break;
        case SLT_Airport:
            strtype = "airport";
            break;
        case SLT_LatLon:
            strtype = "lat_lon";
            break;
        case SLT_MyGPS:
            strtype= "my_gps";
            break;
        case SLT_Route:
            strtype= "route";
            break;
        case SLT_RouteGPS:
            strtype = "route_gps";
            break;
        case SLT_MyDirection:
            strtype= "my_direction";
            break;
        case SLT_AddressFreeForm:
            strtype = "address_free_form";
            break;
        default:
            ret = false;
            break;
    }
    if(ret)
    {
        value["type"] = strtype;
    }
    else
    {
        qDebug()<<"SearchLocationType toJson error, no matched type "<<data;
    }

    return ret;
}

bool DataTransformer::toJson(const SearchPOIContent& data,QJsonObject& value)
{
    bool ret = true;
    QJsonObject content_obj;
    content_obj["tag"] = data.tagLine;
    ret = ret&&DataTransformer::toJson(data.pairs, content_obj);
    ret = ret&&DataTransformer::toJson(data.vendorContents, content_obj);

    if(ret)
    {
        value["content"] = content_obj;
    }
    else
    {
        qDebug()<<"SearchPOIContent toJson error";
    }
    return ret;
}

bool DataTransformer::toJson(const QList<SearchPOIContentKeyValuePair>& data,QJsonObject& value)
{
    QJsonArray attributes_array;
    QJsonObject attributes_obj;
    for(int i = 0; i < data.count(); i++)
    {
        attributes_obj["value"] = data[i].value;
        if(DataTransformer::toJson(data[i].key, attributes_obj))
        {
            attributes_array.push_back(attributes_obj);
        }
        else
        {
            qDebug()<<"QList SearchPOIContentKeyValuePair ["<<i<<"]toJson error";
        }
    }

    value["attributes"] = attributes_array;
    return true;
}

bool DataTransformer::toJson(const SearchPOIConentKey& data, QJsonObject& value)
{
    bool ret = true;
    QString strtype;
    switch (data)
    {
        case SPOICK_Description:
            strtype = "description";
            break;
        case SPOICK_Cuisines:
            strtype = "cuisines";
            break;
        case SPOICK_Features:
            strtype = "features";
            break;
        case SPOICK_HoursOfOperation:
            strtype = "hours_of_operation";
            break;
        case SPOICK_Parking:
            strtype = "parking";
            break;
        case SPOICK_Price:
            strtype = "price";
            break;
        case SPOICK_PaymentMethod:
            strtype = "payment_method";
            break;
        case SPOICK_Reservations:
            strtype = "reservations";
            break;
        case SPOICK_SiteUrl:
            strtype = "site_url";
            break;
        case SPOICK_SpecialFeatures:
            strtype = "special_features";
            break;
        case SPOICK_ThumbnailPhotoUrl:
            strtype = "thumbnail_photo_url";
            break;
        case SPOICK_PhotoUrl:
            strtype = "photo_url";
            break;
        case SPOICK_Wifi:
            strtype = "wifi";
            break;
        case SPOICK_Dresscode:
            strtype = "dress_code";
            break;
        case SPOICK_Smoking:
            strtype = "smoking";
            break;
        case SPOICK_Shuttleservice:
            strtype = "shuttle_service";
            break;
        case SPOICK_Tips:
            strtype = "tips";
            break;
        default:
            ret = false;
            break;
    }
    if(ret)
    {
        value["name"] = strtype;
    }
    else
    {
        qDebug()<<"SearchPOIConentKey toJson error, no matched type "<<data;
    }

    return ret;
}

bool DataTransformer::toJson(const QList<SearchVendorContent>& data, QJsonObject& value)
{
    QJsonArray vendors_array;
    QJsonObject vendors_obj;
    for(int i = 0; i < data.count(); i++)
    {
        vendors_obj["name"] = data[i].name;
        vendors_obj["rating_count"] = (double)data[i].ratingCount;
        vendors_obj["average_rating"] = data[i].averageRating;
        if(DataTransformer::toJson(data[i].pairs, vendors_obj))
        {
            vendors_array.push_back(vendors_obj);
        }
        else
        {
            qDebug()<<"QList SearchVendorContent["<<i<<"] toJson error";
        }
    }

    value["vendors"] = vendors_array;
    return true;
}

bool DataTransformer::toJson(const QList<SearchStringPair>& data, QJsonObject& value)
{
    QJsonArray attributes_array;
    QJsonObject attributes_objJson;
    for(int i = 0; i < data.count(); i++)
    {
        attributes_objJson["name"] = data[i].key;
        attributes_objJson["value"] = data[i].value;
        attributes_array.push_back(attributes_objJson);
    }

    value["attributes"] = attributes_array;
    return true;
}

bool DataTransformer::toJson(const QList<SearchPhone>& data, QJsonObject& value)
{
    QJsonArray phones_array;
    QJsonObject phones_obj;
    for(int i = 0; i < data.count(); i++)
    {
        if(!DataTransformer::toJson(data[i].type, phones_obj))
        {
            qDebug()<<"QList SearchPhone["<<i<<"] toJson error";
            continue;
        }
        phones_obj["number"] = data[i].number;
        phones_obj["country"] = data[i].country;
        phones_obj["area"] = data[i].area;
        phones_obj["formatted_text"] = data[i].formattedText;
        phones_array.push_back(phones_obj);
    }

    value["phones"] = phones_array;
    return true;
}

bool DataTransformer::toJson(const SearchPhoneType& data,QJsonObject& value)
{
    bool ret = true;
    QString strtype;
    switch (data)
    {
        case SPT_Primary:
            strtype = "primary";
            break;
        case SPT_Secondary:
            strtype = "secondary";
            break;
        case SPT_National:
            strtype = "national";
            break;
        case SPT_Mobile:
            strtype = "mobile";
            break;
        case SPT_Tax:
            strtype = "fax";
            break;
        default:
            ret = false;
            break;
    }
    if(ret)
    {
        value["type"] = strtype;
    }
    else
    {
        qDebug()<<"SearchPhoneType toJson error, no matched type "<<data;
    }

    return ret;
}

bool DataTransformer::toJson(const SearchFuelDetails& data, QJsonObject& value)
{
    QJsonArray fuelDetails_array;
    QJsonObject fuelDetails_obj;
    for(int i = 0; i < data.fuelProducts.count(); i++)
    {
        if(DataTransformer::toJson(data.fuelProducts[i], fuelDetails_obj, ""))
        {
            fuelDetails_array.push_back(fuelDetails_obj);
        }
        else
        {
            qDebug()<<"SearchFuelDetails["<<i<<"] toJson error";
        }
    }

    value["fuelDetails"] = fuelDetails_array;
    return true;
}

bool DataTransformer::toJson(const QList<QSharedPointer<SearchCategory> >& data, QJsonObject& value)
{
    QJsonArray categories_array;
    for(int i = 0; i < data.count(); i++)
    {
        QJsonArray subArrayJson;
        subArrayJson.push_back( data[i]->code);
        subArrayJson.push_back( data[i]->name);
        categories_array.push_back(subArrayJson);
    }

    value["categories"] = categories_array;
    return true;
}

bool DataTransformer::toJson(const SearchPOI& data,QJsonObject& value)
{
    bool ret = true;
    ret = ret&&DataTransformer::toJson(data.type, value);
    ret = ret&&DataTransformer::toJson(data.place, value);
    value["distance"] = data.distance;
    ret = ret&&DataTransformer::toJson(data.poiContent, value);
    value["premium"] = data.isPremiumPlacement;
    value["unmappable"] = data.isUnmappable;
    value["enhanced"] = data.isEnhancedPoi;
    ret = ret&&DataTransformer::toJson(data.fuelDetails, value);
    if(!ret)
    {
        qDebug()<<"SearchPOI toJson error";
    }
    return ret;
}

bool DataTransformer::toJson(const SearchSuggestion& data,QJsonObject& value)
{
    value["lines"] = QJsonArray::fromStringList((QStringList)data.lines);
    value["iconIds"] = QJsonArray::fromStringList((QStringList)data.iconIds);
    value["distance"] = data.distance;
    return DataTransformer::toJson(data.type, value);
}

bool DataTransformer::toJson(const SearchSuggestionType& data,QJsonObject& value)
{
    bool ret = true;
    QString strtype;
    switch(data)
    {
        case SST_None:
            strtype = "none";
            break;
        case SST_POI:
            strtype = "POI";
            break;
        case SST_Address:
            strtype = "address";
            break;
        case SST_Airport:
            strtype = "airport";
            break;
        case SST_Gas:
            strtype = "gas";
            break;
        case SST_Category:
            strtype = "category";
            break;
        case SST_Local:
            strtype = "local";
            break;
        case SST_Movie:
            strtype = "movie";
            break;
        default:
            ret = false;
            break;
    }

    if(!ret)
    {
        qDebug()<<"SearchSuggestionType toJson error, no matched type :"<<data;
    }
    else
    {
        value["type"] = strtype;
    }

    return ret;
}

bool DataTransformer::fromJson(const QJsonObject &data, QPoint &pos)
{
    QJsonValue xValue = data["x"];
    QJsonValue yValue = data["y"];
    bool result = xValue.isDouble() && yValue.isDouble();
    if(result)
    {
        pos.setX(xValue.toDouble());
        pos.setY(yValue.toDouble());
    }

    return result;
}

bool DataTransformer::fromJson(const QJsonObject &data, QRect &geometry)
{
    QJsonValue xValue = data["x"];
    QJsonValue yValue = data["y"];
    QJsonValue wValue = data["width"];
    QJsonValue hValue = data["height"];
    bool result = xValue.isDouble() && yValue.isDouble() && wValue.isDouble() && hValue.isDouble();
    if(result)
    {
        geometry.setX(xValue.toInt());
        geometry.setY(yValue.toInt());
        geometry.setWidth(wValue.toInt());
        geometry.setHeight(hValue.toInt());
    }

    return result;
}

bool DataTransformer::fromJson(const QJsonObject &data, Location &location)
{
    bool ret = false;
    QJsonValue latValue = data["latitude"];
    QJsonValue lonValue = data["longitude"];
    QJsonValue headingValue = data["heading"];
    QJsonValue accuracyValue = data["accuracy"];
    if(latValue.isDouble() && lonValue.isDouble() && headingValue.isDouble() && accuracyValue.isDouble())
    {
        location.latitude = latValue.toDouble();
        location.longitude = lonValue.toDouble();
        location.heading = headingValue.toDouble();
        location.horizontalUncertaintyAlongAxis = accuracyValue.toDouble();
        location.valid = 507;
        ret = true;
    }
    return ret;
}

bool DataTransformer::fromJson(const QJsonObject &data, Coordinates &coord)
{
    QJsonValue latValue = data["latitude"];
    QJsonValue lonValue = data["longitude"];
    bool result = latValue.isDouble() && lonValue.isDouble();
    if(result)
    {
        coord.latitude = latValue.toDouble();
        coord.longitude = lonValue.toDouble();
        result = (coord.latitude>=-90. && coord.latitude<=90. ) &&
                    (coord.longitude>=-180. && coord.longitude<=180.);
    }
    return result;
}

bool DataTransformer::toJson(const CameraParameters &camera, QJsonObject &data)
{
    QJsonObject mapCenter;
    if(!toJson(camera.GetPosition(), mapCenter))
    {
        return false;
    }
    data["position"] = mapCenter;
    data["zoom_level"] = camera.GetZoomLevel();
    data["tilt"] = camera.GetTiltAngle();
    data["heading"] = camera.GetHeadingAngle();
    return true;
}

bool DataTransformer::fromJson(const QJsonObject& data, locationtoolkit::PinParameters& pinParameters)
{
    locationtoolkit::Coordinates position;
    locationtoolkit::PinImageInfo selectedPin;
    locationtoolkit::PinImageInfo unselectedPin;
    locationtoolkit::RadiusParameters radius;

    bool result = true;
    result = (!result)?result:fromJson(data["position"].toObject(), position);
    result = (!result)?result:fromJson(data["selected_image_info"].toObject(), selectedPin);
    result = (!result)?result:fromJson(data["unselected_image_info"].toObject(), unselectedPin);
    result = (!result)?result:fromJson(data["radius_info"].toObject(), radius);

    if(result)
    {
        pinParameters.SetPosition(position);
        pinParameters.SetSelectedImage(&selectedPin);
        pinParameters.SetUnselectedImage(&unselectedPin);
        pinParameters.SetRadiusParameters(radius);
        pinParameters.SetBubble(NULL);
        pinParameters.SetVisible(true);
        pinParameters.SetTitle("");
        pinParameters.SetSubtitle("");
    }
    return true;
}

bool DataTransformer::fromJson(const QJsonObject& data, locationtoolkit::PinImageInfo& pinImageInfo)
{
    QJsonValue anchorXValue = data["anchor_x"];
    QJsonValue anchorYValue = data["anchor_y"];
    QJsonValue imageValue = data["image"];

    if(anchorXValue.isDouble() && anchorYValue.isDouble() && imageValue.isString())
    {
        int anchorX = anchorXValue.toInt();
        int anchorY = anchorYValue.toInt();
        QString filepath = imageValue.toString();
        QPixmap pixmap;
        pixmap.load(filepath);
        pinImageInfo.SetPinAnchor(anchorX, anchorY);
        pinImageInfo.SetPixmap(pixmap);
        return true;
    }
    return false;
}

bool DataTransformer::fromJson(const QJsonObject& data, locationtoolkit::RadiusParameters &radiusParameters)
{
    QJsonValue radiusValue = data["radius"];
    if(radiusValue.isDouble())
    {
        int radius = radiusValue.toInt();
        int color = 0x6721D826;

        QJsonValue colorValue = data["color"];
        if(fromJson(colorValue.toObject(), color))
        {
            radiusParameters.SetFillColor(color);
            radiusParameters.SetWidth(radius);
            return true;
        }
    }
    return false;
}

static inline bool checkColorComponect(int c)
{
    return c>=0 && c<=255;
}

bool DataTransformer::fromJson(const QJsonObject& data, int& color)
{
    QJsonValue redValue = data["red"];
    QJsonValue greenValue = data["green"];
    QJsonValue blueValue = data["blue"];
    QJsonValue alphaValue = data["alpha"];

    if(redValue.isDouble() && greenValue.isDouble()
            && blueValue.isDouble() && alphaValue.isDouble())
    {
        int red = redValue.toInt();
        int green = greenValue.toInt();
        int blue = blueValue.toInt();
        int alpha = alphaValue.toInt();
        if(checkColorComponect(red) && checkColorComponect(green)
                && checkColorComponect(blue) && checkColorComponect(alpha))
        {
            color = (red<<24) | (green<<16) | (blue<<8) | alpha;
        }
        return true;
    }
    return false;
}


}
