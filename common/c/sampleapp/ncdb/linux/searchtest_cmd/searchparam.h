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

#ifndef SEARCHPARAM_H
#define SEARCHPARAM_H

#include <sstream>
#include "SingleSearchTypes.h"


class SearchParam
{
public:
    SearchParam(Ncdb::SearchReqType stv, float lat, float lon, const std::string& s)
        : mSearchType(stv), mLatitude(lat), mLongitude(lon), mSearchText(s), mPoiSearchType(Ncdb::ALL_POI_SEARCH) {}
    SearchParam(Ncdb::SearchReqType stv, enum Ncdb::poi_search_type poiSearchType, float lat, float lon, const std::string& s)
        : mSearchType(stv), mLatitude(lat), mLongitude(lon), mSearchText(s), mPoiSearchType(poiSearchType) {}

    Ncdb::SearchReqType searchType() const{ return mSearchType; }
    void setSearchType(Ncdb::SearchReqType st) { mSearchType = st; }

    float latitude() const { return mLatitude; }
    void setLatitude(float lat) { mLatitude = lat; }

    float longitude() const { return mLongitude; }
    void setLongitude(float lon) { mLongitude = lon; }

    const std::string& searchText() const { return mSearchText; }
    void setSearchText(const std::string& s) { mSearchText = s; }

    enum Ncdb::poi_search_type poiSearchType() const { return mPoiSearchType; }
    void setPoiSearchType(enum Ncdb::poi_search_type poiSearchType) { mPoiSearchType = poiSearchType; }

    std::string toString() const {
       std::stringstream ss;
       ss << "(SearchType: " << mSearchType;
       ss << ", Latitude: " << mLatitude;
       ss << ", Longitude: " << mLongitude;
       ss << ", SearchText: " << mSearchText;
       ss << ")";
       return ss.str();
    }

private:
    Ncdb::SearchReqType mSearchType;
    float mLatitude;
    float mLongitude;
    std::string mSearchText;
    enum Ncdb::poi_search_type mPoiSearchType;
};


#endif // SEARCHPARAM_H

