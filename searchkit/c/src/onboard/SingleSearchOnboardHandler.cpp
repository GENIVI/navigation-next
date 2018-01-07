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
    @file     SingleSearchOnboardHandler.cpp
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#include "SingleSearchOnboardHandler.h"
#include <QDebug>
#include <ncdb/include/Session.h>
#include <ncdb/include/SingleSearchCommonTypes.h>
#include <ncdb/include/SingleSearch.h>
#include "nbcontextprotected.h"
#include <palstdlib.h>
#include "TpsSingleSearchSourceHandler.h"
#include "SingleSearchRequestPrivate.h"
#include "CommonFunction.h"
#include "SingleSearchOnboardInformation.h"
#include "AnalyticsSearchEventManager.h"
#include "contextbasedsingleton.h"
#include <sstream>
#include <QDateTime>
#include <QDebug>



static Ncdb::SupportedLanguages ToNcdbSupportedLanguages(const char* language)
{
    if (nsl_stricmp(language, "en-us") == 0)
    {
        return Ncdb::US_English;  //English
    }
    else if (nsl_stricmp(language, "zh-cn") == 0 || nsl_stricmp(language, "zh_cn") == 0)
    {
        return Ncdb::Chinese_Simplified;  //Chinese (Modern)
    }
    else if (nsl_strnicmp(language, "es-", 3) == 0 || nsl_strnicmp(language, "es_", 3) == 0)
    {
        return Ncdb::Spanish;  //Spanish
    }
    else if (nsl_strnicmp(language, "en-", 3) == 0 || nsl_strnicmp(language, "en_", 3) == 0)
    {
        return Ncdb::British_English;
    }
    else if (nsl_strnicmp(language, "zh-", 3) == 0 || nsl_strnicmp(language, "zh_", 3) == 0)
    {
        return Ncdb::Chinese;
    }
    else if (nsl_stricmp(language, "fr-ca") == 0 || nsl_stricmp(language, "fr_ca") == 0)
    {
        return Ncdb::Canadian_French;
    }
    else if (nsl_strnicmp(language, "fr-", 3) == 0 || nsl_strnicmp(language, "fr_", 3) == 0)
    {
        return Ncdb::French;
    }
    return Ncdb::US_English;
}

class ElapseTime
{
public:
    ElapseTime(const QString& msg, const char *pre = "Enter ", const char *post = "Exit ") : m_msg(msg), m_post(post) ,m_start(QDateTime::currentDateTime()){
        qDebug() << pre << m_msg << " at: " << m_start.toString(Qt::RFC2822Date);
    }


    ~ElapseTime() {
        QDateTime end(QDateTime::currentDateTime());
        qint64 elapse = end.toMSecsSinceEpoch() - m_start.toMSecsSinceEpoch();
        qDebug() << m_post << m_msg << " at: " <<  end.toString(Qt::RFC2822Date) << " for an elapse of " << QString::number(elapse) << " ms";
    }

private:
    QString m_msg;
    const char *m_post;
    QDateTime m_start;
};


using namespace protocol;
using namespace nbmap;
using namespace nbsearch;

bool SingleSearchOnboardHandler::m_sSearchInitialized = false;

SingleSearchOnboardHandler::SingleSearchOnboardHandler(
        SingleSearchHandlerPrivate* outer,
        NB_Context* context,
        const SingleSearchRequest* user,
        const std::shared_ptr<SingleSearchListener>& listener,
        const std::shared_ptr<SingleSearchParameters>& param,
        const std::shared_ptr<SingleSearchState>& state)
    : m_outer(outer)
    , m_context(context)
    , m_param(param)
    , m_userRequest(user)
    , m_listener(listener)
    , m_cancelled(false)
    , m_state(state)
{
    m_UITaskQueue = shared_ptr<UiTaskQueue>(new UiTaskQueue(NB_ContextGetPal(m_context)));
}

SingleSearchOnboardHandler::~SingleSearchOnboardHandler()
{
}

void SingleSearchOnboardHandler::SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId)
{
    m_inputSource = std::string(inputSource);
    m_invocationMethod = std::string(invocationMethod);
    m_screenId = std::string(screenId);
}

const std::string DLMTR = "|";
const char ESCAPECHAR = '\\';
const std::string COMMA = ",";
const std::string SPACE = " ";
const std::string COMMA_DLMTR = COMMA + SPACE;
const std::string FAILURE = "FAILED";
const std::string FAILURE_NOT_FOUND = "no results";

std::string GetUtfStringBufferChars(Ncdb::UtfString& s)
{
    char* t;
    t = s.GetBuffer();
    std::string x = (t != 0) ? t : "";
    return x;
}

void DumpGeocodeOutput(std::stringstream& ss, Ncdb::SearchMatch& match)
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

        ss << "\t" << res.c_str() ;
        ss  << " | Distance= (" << match.MatchDistance << ")"               \
             << " | GL= (" << match.GeocodedLocation.y << ", " << match.GeocodedLocation.x << ")" \
             << " | DL= (" << match.DisplayLocation.y << ", " << match.DisplayLocation.x << ")" \
             << " | MATCHTYPE=(" << match.MatchType << ")" \
             << " | CONFIDENCE : "<< match.Confidence << " | SCORE: " << match.Score \
         << " | BBOX= (TL=" << match.BBox.topLeft.y << ", " << match.BBox.topLeft.x \
         << " : BR=" << match.BBox.bottomRight.y << ", " << match.BBox.bottomRight.x << ")" << std::endl;

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
            ss << "\t\tFORMATTED_ADDRESS :" << match.AddressOut.FormattedAddress.GetBuffer() << std::endl;
        int compactLineCount = match.AddressOut.compactFormattedAddress.size();
        if (compactLineCount)
        {
            ss << "\t\tCOMPACT_ADDRESS : " << std::endl;
            for(int i = 0; i < compactLineCount; ++i)
                ss << "\t\t\t" << match.AddressOut.compactFormattedAddress[i].GetBuffer() << std::endl;
        }
        int extendedLineCount = match.AddressOut.extendedFormattedAddress.size();
        if (extendedLineCount)
        {
            ss << "\t\tEXTENDED_ADDRESS : " << std::endl;
            for(int i = 0; i < extendedLineCount; ++i)
                ss << "\t\t\t" << match.AddressOut.extendedFormattedAddress[i].GetBuffer() << std::endl;
        }
}

static void DumpSearchOutput(std::stringstream& ss, Ncdb::SearchMatch * match)
{
    if (Ncdb::PoiMatch_Name != match->MatchType)
    {
        DumpGeocodeOutput(ss, *match);
        return;
    }

    ss << "\tPOI_NAME: [ " << match->MatchName.GetBuffer() << " ] | ";
    ss << "PHONE: [ ";
    Ncdb::AutoArray<Ncdb::UtfString>& phones = match->TelephoneNumbers;
    for (size_t i = 0; i < phones.size(); ++i)
    {
        ss << "(" << phones[i].GetBuffer() << ") ";
    }
    ss << "] | ";
    Ncdb::AutoArray<Ncdb::TYPE_INFO>& ti = match->TypeInfos;
    ss << "TYPE_INFO: [ ";
    for (size_t i = 0; i < ti.size(); i++)
    {
        ss << "(" << ti[i].typeId << ":" << ti[i].subTypeId << ") ";
    }
    ss << "]";
    // TODO
    //AutoArray<TYPE_DATA>& td = match->TypeDatas;
    DumpGeocodeOutput(ss, *match);
}

NB_Error SingleSearchOnboardHandler::error(NB_Error err)
{
    this->NotifyError(err);
    return err;
}

NB_Error SingleSearchOnboardHandler::Request(NB_Context* /*context*/)
{
    ElapseTime et1("ONBOARD SingleSearchOnboardHander::Request");

    if (!m_state->isValid()) {
        return NE_CANCELLED;  // no need to async notify uper layer.
    }

    qDebug() << "ONBOARD Request";
    if (m_param == NULL) {
        return error(NE_BADDATA);
    }
    if (m_cancelled) {
        return error(NE_CANCELLED);
    }

    NB_Error err = NE_OK;

//#define DEBUG_ONBOARD_SEARCH
#ifdef DEBUG_ONBOARD_SEARCH
    // for testing purpose
    Ncdb::Session ncdbSession;
    std::string path = "/home/tom/navdata/mapdata/USA";
    Ncdb::ReturnCode rcSession = ncdbSession.Add(path.c_str());
    if (rcSession != Ncdb::NCDB_OK) {
        qDebug() << "ONBOARD search Failed to add " << path.c_str() << ", error: " << rcSession;
        err = NE_BADDATA;
        this->NotifyError(err);
        return err;
    }
    std::string sessionPath = "/nav/data/mapdata_WM";
    if (ncdbSession.OpenWorld(sessionPath.c_str()) != Ncdb::NCDB_OK)
    {
        qWarning() << "ONBOARD search Failed to load world mapdata from " << sessionPath.c_str();
    }
    Ncdb::SimpleSearch* simplesearch = ncdbSession.GetSimpleSearch();
#else
    shared_ptr<Ncdb::Session> ncdbSession =  ContextBasedSingleton<NcdbObjectsManager>::getInstance(m_context)->GetNcdbSession();
    Ncdb::Session *session = ncdbSession.get();
    if (!session) {
        qWarning() << "ONBOARD ncdb session is not ready.";
        this->NotifyError(NE_UNEXPECTED);
        return NE_UNEXPECTED;
    }
    if (!session->IsOpened()) {
        this->NotifyError(NE_BUSY);
        qWarning() << "ONBOARD ncdb session is not opened.";
        return NE_BUSY;
    }
    Ncdb::SimpleSearch* simplesearch = session->GetSimpleSearch(true);
    // combines both POI and Geocode initialization per latest Karma
    if (!m_sSearchInitialized)
    {
        Ncdb::WorldPoint wp(m_param->m_originPoint.GetLongitude(), m_param->m_originPoint.GetLatitude());
        qDebug() << "ONBOARD search (lat,lon) init: (" << wp.y << ", " << wp.x << ")";
        {
            ElapseTime et("ONBOARD search POI initialization", "Before calling ", "After calling ");
            simplesearch->InitialisePOI(wp);
        }
        {
            ElapseTime et("ONBOARD search Geocode initialization", "Before calling ", "After calling ");
            simplesearch->InitialiseGeocode(wp);
        }
        m_sSearchInitialized = true;
    }
#endif
    Ncdb::AutoArray<Ncdb::AutoSharedPtr<Ncdb::SearchMatch> > geoPoiMatch;
    Ncdb::SearchRequest req;

    //**********POPULATE various elements of “req”***************
    if (m_param->resultStyleType() == NB_SRST_Suggestion) {
        req.ReqType = Ncdb::SIMPLE_SUGGEST;
    } else {
        req.ReqType = Ncdb::SIMPLE_SEARCH;
    }

    req.WantFlags = WANT_SINGLELINE_ADDRESS | WANT_COMPACT_ADDRESS | WANT_EXTENDED_ADDRESS | WANT_COUNTRY_NAME;
    req.PoiSearchCriteria.searchType = Ncdb::ALL_POI_SEARCH;
    req.PoiSearchCriteria.typeToSearch.typeId = -1;
    req.PoiSearchCriteria.typeToSearch.subTypeId = -1;
    req.PoiSearchCriteria.phoneToSearch = "";
    req.PoiSearchCriteria.nameToSearch = "";
    req.PoiSearchCriteria.SearchResultId = "";
    req.PoiSearchCriteria.codesArray = "";
    req.InLanguageCode = ToNcdbSupportedLanguages(m_param->m_language.c_str());
    req.SearchLocation.Set(m_param->m_searchPoint.GetLongitude(), m_param->m_searchPoint.GetLatitude());
    req.UserLocation.Set(m_param->m_originPoint.GetLongitude(), m_param->m_originPoint.GetLatitude());

    if (m_param->searchSource() == NB_SS_Explore) {
       req.ReqType = Ncdb::POI_SEARCH;
       req.PoiSearchCriteria.searchType = Ncdb::TYPE_SEARCH_ACODE;
       req.PoiSearchCriteria.codesArray = m_param->categoryCode().c_str();
    } else if (m_param->searchSource() == NB_SS_POILayer) {
        req.ReqType = Ncdb::POI_SEARCH;
        req.PoiSearchCriteria.searchType = Ncdb::NAME_LOOKUP;
        req.PoiSearchCriteria.nameToSearch = m_param->name().c_str();
    } else {
        req.SearchString = m_param->m_name.c_str();
    }

    int rc = simplesearch->Search(req, geoPoiMatch);
    if (Ncdb::NCDB_OK == rc || Ncdb::NCDB_END_OF_QUERY == rc || Ncdb::NCDB_MORE_RECORDS == rc) {
        qDebug() << "\t" << "ONBOARD SUCCESS" << " - " << rc << endl;
        qDebug() << "\t" << "ONBOARD geoPoiMatch.size() : << " << geoPoiMatch.size() << endl;
        for (int i = 0; i < (int)geoPoiMatch.size(); i++) {
            std::stringstream ss;
            DumpSearchOutput(ss, geoPoiMatch[i]);
            qDebug() << ss.str().c_str();
        }
        NotifySuccess(new OnboardSingleSearchInformationImpl(geoPoiMatch, 0));
        geoPoiMatch.clear();
    } else  if (rc == Ncdb::NCDB_NOT_FOUND) {
        qDebug() << "\t" << "ONBOARD FAILURE_NOT_FOUND" << " - " << rc << endl;
        NotifySuccess(new OnboardSingleSearchInformationImpl());
    } else if (rc == Ncdb::NCDB_UNSUPPORTED) {
        qDebug() << "\t ONBOARD Feature Not Supported." << " - " << rc << endl;
        err = NE_NOSUPPORT;
    } else if (rc == Ncdb::NCDB_TIME_OUT) {
        qDebug() << "\t ONBOARD Time Out." << endl;
        err = NESERVER_TIMEDOUT;
    } else {
        qDebug() << "\t" << "ONBOARD FAILURE" << " - " << rc << endl;
        err = NESERVER_FAILED;
    }

    if (err != NE_OK) {
        this->NotifyError(err);
    }

    return err;
}

void SingleSearchOnboardHandler::Cancel()
{
    m_cancelled = true;
}

bool SingleSearchOnboardHandler::IsInProgress()
{
    return false;
}

void SingleSearchOnboardHandler::UI_NotifySuccess(const OnboardSingleSearchTask* pTask)
{
    const OnboardSingleSearchUITaskSuccess* task = static_cast<const OnboardSingleSearchUITaskSuccess*>(pTask);
    if (m_state->isValid() && m_listener)
    {
        m_listener->OnSuccess(*this->m_userRequest, *task->m_information);
    }
}

void SingleSearchOnboardHandler::UI_NotifyError(const OnboardSingleSearchTask* pTask)
{
    const OnboardSingleSearchUITaskError* task = static_cast<const OnboardSingleSearchUITaskError*>(pTask);
    if (m_state->isValid() && m_listener)
    {
        m_listener->OnError(*this->m_userRequest, task->m_error);
    }
}

void SingleSearchOnboardHandler::UI_NotifyProgress(const OnboardSingleSearchTask* pTask)
{
    const OnboardSingleSearchUITaskProgress* task = static_cast<const OnboardSingleSearchUITaskProgress*>(pTask);
    if (m_state->isValid() && m_listener)
    {
        m_listener->OnProgressUpdated(*this->m_userRequest, task->m_percentage);
    }
}

void SingleSearchOnboardHandler::NotifySuccess(SingleSearchInformation *info)
{
    if (!m_cancelled && m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new OnboardSingleSearchUITaskSuccess(this, &SingleSearchOnboardHandler::UI_NotifySuccess, info, m_state));
    }
}

void SingleSearchOnboardHandler::NotifyError(NB_Error error)
{
    if (!m_cancelled && m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new OnboardSingleSearchUITaskError(this, &SingleSearchOnboardHandler::UI_NotifyError, error, m_state));
    }
}

void SingleSearchOnboardHandler::NotifyProgress(int percentage)
{
    if (!m_cancelled && m_UITaskQueue)
    {
        m_UITaskQueue->AddTask(new OnboardSingleSearchUITaskProgress(this, &SingleSearchOnboardHandler::UI_NotifyProgress, percentage, m_state));
    }
}


/*! @} */
