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

#include "searchobject.h"
#include "datatransformer.h"
#include "util.h"
#include <singlesearchrequest.h>
#include "suggestionsearchrequest.h"
#include "searchreversegeocoderequest.h"
#include "addresssearchrequest.h"

const double INVALID_LATITUDE                   = -999.0;
const double INVALID_LONGITUDE                  = -999.0;

SearchObject::SearchObject(QObject *parent) : QObject(parent), m_searchRequest(NULL)
{

}

locationtoolkit::SearchRequest* SearchObject::createSingleSearchRequest(const SearchParam& param)
{
    double lat = param.lat;
    double lon = param.lon;
    locationtoolkit::Coordinates centerCoordinates(lat, lon);
    locationtoolkit::Coordinates userCoordinates(lat, lon);
    locationtoolkit::SearchRequest* request = locationtoolkit::SingleSearchRequest::createRequest(*GetLTKContext(),
                                                                param.text,
                                                                userCoordinates,
                                                                centerCoordinates);
    return request;
}

locationtoolkit::SearchRequest* SearchObject::createCategorySearchRequest(const SearchParam& param)
{
    double lat = param.lat;
    double lon = param.lon;
    locationtoolkit::Coordinates centerCoordinates(lat, lon);
    locationtoolkit::Coordinates userCoordinates(lat, lon);
    locationtoolkit::SearchRequest* request = locationtoolkit::SingleSearchRequest::createRequestWithCategory(*GetLTKContext(),
                                                                param.text,
                                                                userCoordinates,
                                                                centerCoordinates);
    return request;
}

locationtoolkit::SearchRequest* SearchObject::createSuggestionRequest(const SearchParam& param)
{
    return NULL;
}

locationtoolkit::SearchRequest* SearchObject::createGeocodeRequest(const SearchParam& param)
{
    locationtoolkit::Coordinates userCoordinates;
    userCoordinates.latitude = INVALID_LATITUDE;
    userCoordinates.longitude = INVALID_LONGITUDE;
    locationtoolkit::Coordinates centerCoordinates(userCoordinates);
    locationtoolkit::SearchRequest* request = locationtoolkit::AddressSearchRequest::createRequest(*GetLTKContext(),
                                                                param.text,
                                                                userCoordinates,
                                                                centerCoordinates);
    return request;
}

locationtoolkit::SearchRequest* SearchObject::createReverseGeocodeRequest(const SearchParam& param)
{
    double lat = param.lat;
    double lon = param.lon;
    locationtoolkit::Coordinates coordinate(lat, lon);
    locationtoolkit::SearchRequest* request = locationtoolkit::SearchReverseGeocodeRequest::createRequest(*GetLTKContext(), coordinate);
    return request;
}


void SearchObject::request(const SearchParam& param)
{
    locationtoolkit::SearchRequest* request = NULL;

    switch(param.st) {
    case SINGLE_SEARCH:
        request = createSingleSearchRequest(param);
        break;
    case SUGGESTION_SEARCH:
        request = createSuggestionRequest(param);
        break;
    case GEOCODE:
        request = createGeocodeRequest(param);
        break;
    case REVERSE_GEOCODE:
        request = createReverseGeocodeRequest(param);
        break;
    case CATEGORY_SEARCH:
        request = createCategorySearchRequest(param);
        break;
    }

    if (request != NULL) {
        sendRequest(request);
    } else {
        qDebug() << "request is NULL for search type: " << param.st;
    }

}


void SearchObject::onSuccess(QSharedPointer<locationtoolkit::SearchInformation> info)
{
    QJsonObject json;
    bool result = locationtoolkit::DataTransformer::toJson(*info, json);
    SearchResult sr;
    if (result) {
        QJsonDocument doc(json);
        QString str(doc.toJson(QJsonDocument::Indented));
        sr.data = str;
    } else {
        sr.data = "Failed to convert SearchInformation to JsonObject";
    }
    emit response(sr);
}

void SearchObject::onError(int errorCode)
{
    SearchResult sr;
    sr.data = QString("Error return from search: %1").arg(errorCode);
    emit response(sr);
}

void SearchObject::onProgressUpdated(int percentage)
{

}

void SearchObject::sendRequest(locationtoolkit::SearchRequest* request)
{
    if(m_searchRequest)
    {
        disconnect(m_searchRequest, SIGNAL(OnSuccess(QSharedPointer<locationtoolkit::SearchInformation>)),
                   this, SLOT(onSuccess(QSharedPointer<locationtoolkit::SearchInformation>)));
        disconnect(m_searchRequest, SIGNAL(OnError(int)), this, SLOT(onError(int)));
        disconnect(m_searchRequest, SIGNAL(OnProgressUpdated(int)), this, SLOT(onProgressUpdated(int)));
        delete m_searchRequest;
    }
    m_searchRequest = request;
    connect(m_searchRequest, SIGNAL(OnSuccess(QSharedPointer<locationtoolkit::SearchInformation>)),
            this, SLOT(onSuccess(QSharedPointer<locationtoolkit::SearchInformation>)));
    connect(m_searchRequest, SIGNAL(OnError(int)), this, SLOT(onError(int)));
    connect(m_searchRequest, SIGNAL(OnProgressUpdated(int)), this, SLOT(onProgressUpdated(int)));
    m_searchRequest->StartRequest();
}
