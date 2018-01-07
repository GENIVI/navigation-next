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

#include "searchwindow.h"
#include "ui_searchwindow.h"
#include <iostream>
#include <sstream>
#include <QDebug>
#include <SingleSearch.h>
#include <NcdbTypes.h>
#include <ReverseGeocode.h>


const std::string DLMTR = "|";
const char ESCAPECHAR = '\\';
const std::string COMMA = ",";
const std::string SPACE = " ";
const std::string COMMA_DLMTR = COMMA + SPACE;
const std::string FAILURE = "FAILED";
const std::string FAILURE_NOT_FOUND = "no results";

const char* MAP_DATA_PATH = "/home/tom/navdata/mapdata/USA";
const char* MAP_DATA_INI = "/home/tom/navdata/mapdata/USA/main.ini";
const char* CONFIG_PATH = "/home/tom/navdata/Config";

Ncdb::ReturnCode SearchWindow::OpenIni(const char* path) {
    Ncdb::ReturnCode rc = m_ncdbSession.Open(path);
    if (rc != Ncdb::NCDB_OK) {
        qDebug() << "Failed to open " << path << ", error: " << rc;
    }
    return rc;
}

Ncdb::ReturnCode SearchWindow::AddPath(const char* path) {
    Ncdb::ReturnCode rc = m_ncdbSession.Add(path);
    if (rc != Ncdb::NCDB_OK) {
        qDebug() << "Failed to open " << path << ", error: " << rc;
    }
    return rc;
}

Ncdb::ReturnCode SearchWindow::OpenConfig(const char* path) {
    Ncdb::ReturnCode rc = m_ncdbSession.OpenMainAndMapInfo(path);
    if (rc != Ncdb::NCDB_OK) {
        qDebug() << "Failed to open " << path << ", error: " << rc;
    }
    return rc;
}

SearchWindow::SearchWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SearchWindow)
{
    ui->setupUi(this);


    Ncdb::ReturnCode rc = Ncdb::NCDB_OK;

    //if (rc == Ncdb::NCDB_OK) rc = OpenConfig(CONFIG_PATH);
    //if (rc == Ncdb::NCDB_OK) rc = AddPath(MAP_DATA_PATH);
    if (rc == Ncdb::NCDB_OK) rc = OpenIni(MAP_DATA_INI);
    if (!m_ncdbSession.IsOpened()) {
        qDebug() << "Session is not openned.";
    }
}

SearchWindow::~SearchWindow()
{
    delete ui;
    Ncdb::ReturnCode rc = m_ncdbSession.Close();
    if (rc != Ncdb::NCDB_OK) {
        qDebug() << "Failed to close ncdb, error: " << rc;
    }
    m_ncdbSession.~Session();
}

std::string GetUtfStringBufferChars(Ncdb::UtfString& s)
{
    char* t;
    t = s.GetBuffer();
    std::string x = (t != 0) ? t : "";
    return x;
}

void DumpGeocodeOutput(Ncdb::SearchMatch& match)
{
        std::string res, num, streetname, crossstreet, namedarea, city, zone, county, state, zip, country;
        //string streetParts;

        num = GetUtfStringBufferChars(match.AddressOut.StartNumber);
        if(!match.AddressOut.StreetName.IsEmpty())
        {
            streetname = match.AddressOut.StreetName.GetBuffer();
            /*if(g_bVerbose)
            {
                GetStreetParts(match.AddressOut.StreetList[0], streetParts);
                streetname = streetname + streetParts;
            }*/
        }

        if(!match.AddressOut.City.IsEmpty())
            city = match.AddressOut.City.GetBuffer();
        if(!match.AddressOut.StateFullName.IsEmpty())
            state = match.AddressOut.StateFullName.GetBuffer();
        if( match.AddressOut.Postal.GetBuffer() )
            zip = match.AddressOut.Postal.GetBuffer();

        country = GetUtfStringBufferChars(match.AddressOut.Country);

        res = num + SPACE + streetname + COMMA_DLMTR + city + COMMA_DLMTR + state + COMMA_DLMTR + zip + COMMA_DLMTR + country;

        namedarea = GetUtfStringBufferChars(match.AddressOut.Subcity1);
        zone = GetUtfStringBufferChars(match.AddressOut.Subcity2);
        county = GetUtfStringBufferChars(match.AddressOut.County);
        if(!match.AddressOut.CrossStreet.IsEmpty())
        {
            crossstreet = match.AddressOut.CrossStreet.GetBuffer();
            /*if(g_bVerbose)
            {
                GetStreetParts(match.AddressOut.CrossStreetList[0], streetParts);
                crossstreet = crossstreet + streetParts;
            }*/
        }

        res = res + " | " + namedarea + COMMA_DLMTR + zone + COMMA_DLMTR + county;
        res = res + " | " + crossstreet;
        if(!match.AddressOut.AirportCode.IsEmpty())
        {
            res = res + " | AirportCode = " + match.AddressOut.AirportCode.GetBuffer();
            res = res + " | AirportName = " + match.AddressOut.AirportName.GetBuffer();
        }
        /*outstring << "<OPTION><ADDRESS><HNO>" << num << "</HNO>" \
                  << "<STREET>" << streetname << "</STREET>" \
                  << "<XSTREET>" << crossstreet << "</XSTREET>" \
                  << "<SUBCITY1>" << namedarea << "</SUBCITY1>" \
                  << "<SUBCITY2>" << zone << "</SUBCITY2>"           \
                  << "<CITY>" << city << "</CITY>" \
                  << "<COUNTY>" << county << "</COUNTY>" \
                  << "<STATE>" << state << "</STATE>" \
                  << "<POSTAL>" << zip << "</POSTAL>" \
                  << "<COUNTRY>" << country << "</COUNTRY>" \
                  << "</ADDRESS>";
        outstring.precision(9);*/

        qDebug() << "\t" << res.c_str() ;
        qDebug()  << " | Distance= (" << match.MatchDistance << ")"               \
             << " | GL= (" << match.GeocodedLocation.y << ", " << match.GeocodedLocation.x << ")" \
             << " | DL= (" << match.DisplayLocation.y << ", " << match.DisplayLocation.x << ")" \
             << " | MATCHTYPE=(" << match.MatchType << ")" \
             << " | CONFIDENCE : "<< match.Confidence << " | SCORE: " << match.Score \
         << " | BBOX= (TL=" << match.BBox.topLeft.y << ", " << match.BBox.topLeft.x \
         << " : BR=" << match.BBox.bottomRight.y << ", " << match.BBox.bottomRight.x << ")" <<endl;

        /*outstring << "<IP>" << match.InterpolatedLocation.x << "," << match.InterpolatedLocation.y << "</IP>" \
                  << "<GP>" << match.GeocodedLocation.x << "," << match.GeocodedLocation.y << "</GP>" \
                  << "<DP>" << match.DisplayLocation.x << "," << match.DisplayLocation.y << "</DP>" \
                  << "<DISTANCE>" << match.Distance << "</DISTANCE>" \
                  << "<SCORE>" << match.Confidence << "</SCORE>" \
                  << "<COMPSCORE>" << match.Score << "</COMPSCORE>" \
                  << "<SRCID>" << match.addrSegmentId.GetSrcId() << "</SRCID>";

        outstring << "</OPTION>";

        if(g_bVerbose)
        {
            std::stringstream strSrcId;
            strSrcId << match.addrSegmentId.GetSrcId();
            cout << "\t\tSRCID=" << strSrcId.str() << endl;
            PrintAltNames(match.AddressOut);
        }*/
        if(!match.AddressOut.FormattedAddress.IsEmpty())
            qDebug() << "\t\tFORMATTED_ADDRESS :" << match.AddressOut.FormattedAddress.GetBuffer() << endl;
        int compactLineCount = match.AddressOut.compactFormattedAddress.size();
        if (compactLineCount)
        {
            qDebug() << "\t\tCOMPACT_ADDRESS : " << endl;
            for(int i = 0; i < compactLineCount; ++i)
                qDebug() << "\t\t\t" << match.AddressOut.compactFormattedAddress[i].GetBuffer() << endl;
        }
        int extendedLineCount = match.AddressOut.extendedFormattedAddress.size();
        if (extendedLineCount)
        {
            qDebug() << "\t\tEXTENDED_ADDRESS : " << endl;
            for(int i = 0; i < extendedLineCount; ++i)
                qDebug() << "\t\t\t" << match.AddressOut.extendedFormattedAddress[i].GetBuffer() << endl;
        }
}

void DumpSearchOutput(std::basic_ostringstream<char>& stm, Ncdb::SearchMatch * match)
{
    if (Ncdb::PoiMatch_Name != match->MatchType)
    {
        DumpGeocodeOutput(*match);
        return;
    }

    stm << "\tPOI_NAME: [ " << match->MatchName.GetBuffer() << " ] | ";
    stm << "PHONE: [ ";
    Ncdb::AutoArray<Ncdb::UtfString>& phones = match->TelephoneNumbers;
    for (size_t i = 0; i < phones.size(); ++i)
    {
        stm << "(" << phones[i].GetBuffer() << ") ";
    }
    stm << "] | ";
    Ncdb::AutoArray<Ncdb::TYPE_INFO>& ti = match->TypeInfos;
    stm << "TYPE_INFO: [ ";
    for (size_t i = 0; i < ti.size(); i++)
    {
        stm << "(" << ti[i].typeId << ":" << ti[i].subTypeId << ") ";
    }
    stm << "]";
    // TODO
    //AutoArray<TYPE_DATA>& td = match->TypeDatas;
    std::cout << stm.str() << endl;
    DumpGeocodeOutput(*match);
}


void SearchWindow::on_pushButtonSingleSearch_clicked()
{
    qDebug() << "SearchWindow::on_pushButtonSingleSearch_clicked";

    Ncdb::SimpleSearch* simplesearch = m_ncdbSession.GetSimpleSearch();
    Ncdb::AutoArray<Ncdb::AutoSharedPtr<Ncdb::SearchMatch> > geoPoiMatch;
    Ncdb::SearchRequest req;

    //**********POPULATE various elements of “req”***************
    //disneyland lat/lon 33.8090° N, 117.9190° W
    //33.8090, -117.9190
    //req.ReqType = Ncdb::SIMPLE_SEARCH;
    req.ReqType = Ncdb::SIMPLE_SEARCH;
    req.SearchLocation.Set(-117.9190f, 33.80900);
    req.SearchString = "6A Liberty Aliso Viejo CA 92615";
    //req.SearchString = "925 Westlake Ave N, Seattle 98109";
    //req.SearchString = "starbucks";
    req.NumResults = 2;
    req.PageID = 1;
    req.WantFlags = WANT_SINGLELINE_ADDRESS;
    req.Country = "USA";
    req.InLanguageCode = Ncdb::Unknown_Langauge;
    req.PoiSearchCriteria.searchType = 0;
    req.PoiSearchCriteria.typeToSearch.typeId = -1;
    req.PoiSearchCriteria.typeToSearch.subTypeId = -1;
    req.PoiSearchCriteria.phoneToSearch = "";
    req.PoiSearchCriteria.nameToSearch = "";
    req.PoiSearchCriteria.SearchResultId = "";

//    //! for more records to be retrieved
//	NCDB_MORE_RECORDS    = 201,
//    //! query end condition: no more records to read
//    NCDB_END_OF_QUERY    = 202,
//	//! no objects are found by query.
//    NCDB_NOT_FOUND       = 203,
//	NCDB_TIME_OUT = 204, //Request Timedout
//	//! retrieved data contain no information
//	NCDB_EMPTY_DATA      = 211,
//	//! output buffer failure
//	NCDB_BUFFER_OVERFLOW = 212,
//	//! query handler is not valid
//	NCDB_INVALID_HANDLER = 221,
//	//! feature not supported
//	NCDB_UNSUPPORTED     = 222,

    int rc = simplesearch->Search(req, geoPoiMatch);
    if (Ncdb::NCDB_OK == rc || Ncdb::NCDB_MORE_RECORDS == rc || Ncdb::NCDB_END_OF_QUERY) {

        if (Ncdb::NCDB_OK != rc) {
            qDebug() << "\t" << "querry end" << " - " << rc << endl;
        }
        for (int i = 0; i < (int)geoPoiMatch.size(); i++) {
            std::basic_ostringstream<char> stm;
            DumpSearchOutput(stm, geoPoiMatch[i]);
            ui->textEdit->setText(stm.str().c_str());
        }
        geoPoiMatch.clear();
    } else  if (rc == Ncdb::NCDB_NOT_FOUND) {
        qDebug() << "\t" << "FAILURE_NOT_FOUND" << " - " << rc << endl;
    } else if (rc == Ncdb::NCDB_UNSUPPORTED) {
        qDebug() << "\t Feature Not Supported." << " - " << rc << endl;
    } else if (rc == Ncdb::NCDB_TIME_OUT) {
        qDebug() << "\t Time Out." << endl;
    } else{
        qDebug() << "\t" << "FAILURE" << " - " << rc << endl;
    }
}


void SearchWindow::on_pushButtonReverseGeocode_clicked()
{
    qDebug() << "SearchWindow::on_pushButtonSingleSearch_clicked";
    Ncdb::ReverseGeocode revGeocode = Ncdb::ReverseGeocode(m_ncdbSession);
    Ncdb::WorldPoint location(-117.9190f, 33.80900);
    Ncdb::WorldPoint foundLocation;
    Ncdb::Address address;
    Ncdb::StreetSide foundSide;
    Ncdb::SupportedLanguages supportedLanguage = Ncdb::US_English;

    int rc = revGeocode.FindAddress(location, 100.0, 0, supportedLanguage, address, foundLocation, foundSide);
    if (Ncdb::NCDB_OK == rc) {
        qDebug() << "\t" << "querry reverse geocode OK" << " - " << rc << endl;
    } else  if (rc == Ncdb::NCDB_NOT_FOUND) {
        qDebug() << "\t" << "FAILURE_NOT_FOUND" << " - " << rc << endl;
    } else if (rc == Ncdb::NCDB_UNSUPPORTED) {
        qDebug() << "\t Feature Not Supported." << " - " << rc << endl;
    } else if (rc == Ncdb::NCDB_TIME_OUT) {
        qDebug() << "\t Time Out." << endl;
    } else{
        qDebug() << "\t" << "FAILURE" << " - " << rc << endl;
    }

}
