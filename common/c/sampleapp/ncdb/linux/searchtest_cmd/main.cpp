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

#include <iostream>
#include <sstream>
#include "searchtest.h"
#include "searchparam.h"
#include <unistd.h>
#include <limits.h>
#include <sys/resource.h>

using namespace std;


// category search
//Integration changes  needed in CC

//OUTPUT: No change. You will continue to consume POI results and their attributes in the same way as today.


//INPUT: Right now you set the ReqType to  Ncdb::SIMPLE_SEARCH and pass user input text in  SearchRequest.SearchString
//      which  is of type UtfString

//For searching by category codes[ACodes] you will set various input attributes as below
//searchReq.ReqType                                   // set it to Ncdb::POI_SEARCH
//searchReq.PoiSearchCriteria.searchType              // set it to  Ncdb::TYPE_SEARCH_ACODE
//searchReq.PoiSearchCriteria.codesArray              // ”codesArray” is of type UtfString. It will contain ACode to search.
                                                      // If more than 1 Code need to passed, you will combine them with comma
//Few ACodes you should try to ensure integration is fine:
//•    AAB                       // Banks
//•    AEA                        // American Restaurants
//•    AEYP                      //Thai Restaurants
//•    AKD                       //Grocery Stores
//•    AEC                        //Bars
//•    AEG                       //Indian Restaurants
//•    ACI                         //Auto Clubs
//•    AN                          // Hospitals

//Try them around  location {33.578164, -117.730536}
//At my end they can return valid POIs belonging to these categories.
void doCategorySearch(SearchTest& st)
{
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::TYPE_SEARCH_ACODE, 33.578164, -117.730536, "AAB"));
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::TYPE_SEARCH_ACODE, 33.578164, -117.730536, "AEA"));
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::TYPE_SEARCH_ACODE, 33.578164, -117.730536, "AEYP"));
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::TYPE_SEARCH_ACODE, 33.578164, -117.730536, "AKD"));
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::TYPE_SEARCH_ACODE, 33.578164, -117.730536, "AEC"));
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::TYPE_SEARCH_ACODE, 33.578164, -117.730536, "AEG"));
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::TYPE_SEARCH_ACODE, 33.578164, -117.730536, "ACI"));
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::TYPE_SEARCH_ACODE, 33.578164, -117.730536, "AN"));
    cout.flush();
}

void doStaticPoiSearch(SearchTest& st)
{
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::NAME_LOOKUP, 33.5259895, -117.71759, "Pizza Hut"));
    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, Ncdb::NAME_LOOKUP, 33.6123085, -117.713013, "Starbucks"));
    cout.flush();
}
void doSingleSearch(SearchTest& st)
{
    //st.DoSearch(SearchParam(Ncdb::SIMPLE_SEARCH, 33.8090, -117.9190, "yardhouse"));
    st.DoSearch(SearchParam(Ncdb::SIMPLE_SEARCH, 33.8090, -117.9190, "pizza"));

    //st.DoSearch(SearchParam(Ncdb::SIMPLE_SEARCH, 33.8090, -117.9190, "pizza"));
    //st.DoSearch(SearchParam(Ncdb::SIMPLE_SEARCH, 33.6833, -117.794, "Pizza"));
    //st.DoSearch(SearchParam(Ncdb::SIMPLE_SEARCH, 33.6833, -117.794, "walmart"));
    //“6 liberty” around “33.578164, -117.730536”
    // “22501 chase” around “33.578164, -117.730536”
    // req.SearchString = "22501 chase";
    //st.DoSearch(SearchParam(Ncdb::SIMPLE_SEARCH, 33.578164, -117.730536, "6 liberty"));
    //“Ralphs” around “33.559063, -117.729195”
    //“Pizza” around “33.559063, -117.729195”
//    st.DoSearch(SearchParam(Ncdb::POI_SUGGEST, 33.559063, -117.729295, "Pizza"));
//    st.DoSearch(SearchParam(Ncdb::SIMPLE_SEARCH, 33.559063, -117.729295, "Pizza"));
//    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, 33.559063, -117.729295, "Pizza"));
//    st.DoSearch(SearchParam(Ncdb::GEOCODE_SEARCH, 33.559063, -117.729295, "Pizza"));
    //st.DoReverseGeocode();
//    st.DoSearch(SearchParam(Ncdb::SIMPLE_SEARCH, 33.559063, -117.729295, "SNA"));
//    st.DoSearch(SearchParam(Ncdb::POI_SEARCH, 33.559063, -117.729295, "SNA"));
//    st.DoSearch(SearchParam(Ncdb::GEOCODE_SEARCH, 33.559063, -117.729295, "SNA"));
//    st.DoSearch(SearchParam(Ncdb::AIRPORT_SEARCH, 33.559063, -117.729295, "SNA"));
    cout.flush();
}

void doGeocodeSearch(SearchTest& st)
{
    st.DoSearch(SearchParam(Ncdb::SIMPLE_SEARCH, 33.8090, -117.9190, "6A Liberty Aliso Viejo CA 92615"));
}

void doSuggestionSearch(SearchTest& st)
{
    st.DoSearch(SearchParam(Ncdb::SIMPLE_SUGGEST, 33.578164, -117.730536, "Restaurants"));
    cout.flush();
}
void doReverseGeocode(SearchTest& st)
{
    st.DoReverseGeocode();
    cout.flush();
}

void outputMapVersion(SearchTest& st)
{
    st.PrintMapDataVersion();
}

void doSystemLimit()
{
    int max_file = ::sysconf(_SC_OPEN_MAX);
    cout << "Number of max file can be openned: " << max_file << std::endl;

    struct rlimit rl = {1000 * 5, 1000 * 10};
    int ret = setrlimit(RLIMIT_NOFILE, &rl);
    cout << "Increase max concurrent file openning returns " << ret;
}

int main()
{
    ElapseTime et("main");
    SearchTest st;

    outputMapVersion(st);
    //doSuggestionSearch(st);
    //doGeocodeSearch(st);
    //doSingleSearch(st);
    //doCategorySearch(st);
    //doReverseGeocode(st);
    //doStaticPoiSearch(st);

    doSystemLimit();
    return 0;
}
