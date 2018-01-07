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

#include "OnboardReverseGeocodeHandler.h"
#include "contextbasedsingleton.h"
#include "ncdbobjectsmanager.h"
#include "ncdb/include/SingleSearch.h"
#include "ncdb/include/ReverseGeocode.h"
#include <QDebug>
#include "OnboardReverseGeocodeInformationImpl.h"
#include <unistd.h>
#include "RouteManager.h"


namespace nbcommon
{

static Ncdb::SupportedLanguages ConvertStr2Lang(const char* lang)
{
    // TODO: add other languages
    if (nsl_stricmp(lang, "en-us") == 0)
    {
        return Ncdb::US_English;
    }
    else if (nsl_strnicmp(lang, "es-", 3) == 0)
    {
        return Ncdb::Spanish;
    }
    else if (nsl_strnicmp(lang, "en-", 3) == 0)
    {
        return Ncdb::US_English;
    }
    else if (nsl_stricmp(lang, "zh-cn") == 0)
    {
        return Ncdb::Chinese;
    }
    else if (nsl_strnicmp(lang, "zh-", 3) == 0)
    {
        return Ncdb::Chinese;
    }

    return Ncdb::Invalid_Language;
}


OnboardReverseGeocodeHandler::OnboardReverseGeocodeHandler(ReverseGeocodeRequestPrivate* requestPrivate)
    : m_requestPrivate(requestPrivate)
{

}

OnboardReverseGeocodeHandler::~OnboardReverseGeocodeHandler()
{
}

NB_Error OnboardReverseGeocodeHandler::Request(NB_Context* context)
{
    qDebug() << "ONBOARD reverse geocode request";
    if (m_sparam == NULL)
    {
        m_requestPrivate->NotifyError(NE_BADDATA);
        return NE_BADDATA;
    }

    shared_ptr<Ncdb::Session> ncdbSession =  ContextBasedSingleton<NcdbObjectsManager>::getInstance(context)->GetNcdbSession();
    Ncdb::Session* session = ncdbSession.get();
    if (!session) {
        qWarning() << "ONBOARD ncdb session is not ready.";
        m_requestPrivate->NotifyError(NE_UNEXPECTED);
        return NE_UNEXPECTED;
    }
    if (!session->IsOpened()) {
        m_requestPrivate->NotifyError(NE_BUSY);
        qWarning() << "ONBOARD ncdb session is not opened.";
        return NE_BUSY;
    }

    Ncdb::ReverseGeocode revGeocode = Ncdb::ReverseGeocode(*session);
    NB_Error err = NE_OK;
    Ncdb::WorldPoint location(m_sparam->GetLongitude(), m_sparam->GetLatitude());
    Ncdb::WorldPoint foundLocation;
    Ncdb::Address address;
    Ncdb::StreetSide foundSide;
    qDebug() << "\t" << "ONBOARD language: " << m_sparam->config().language().c_str() << endl;
    Ncdb::SupportedLanguages supportedLanguage = ConvertStr2Lang(m_sparam->config().language().c_str());
    int rc = revGeocode.FindAddress(location, 200.0, 0, supportedLanguage, address, foundLocation, foundSide);

    if (Ncdb::NCDB_OK == rc || Ncdb::NCDB_END_OF_QUERY == rc || Ncdb::NCDB_MORE_RECORDS == rc) {
        qDebug() << "\t" << "ONBOARD reverse geocode SUCCESS" << " - " << rc << endl;

        Ncdb::RouteManager rm = Ncdb::RouteManager(*session);
        float utcTimeOffset = 0;
        rm.GetPointTimeAsUTCOffset(location, utcTimeOffset);

        m_requestPrivate->NotifySuccess(new OnboardReverseGeocodeInformationImpl(address, foundLocation, utcTimeOffset));
    } else  if (rc == Ncdb::NCDB_NOT_FOUND) {
        qDebug() << "\t" << "ONBOARD reverse geocode FAILURE_NOT_FOUND" << " - " << rc << endl;
        err = NE_NOENT;
    } else if (rc == Ncdb::NCDB_UNSUPPORTED) {
        qDebug() << "\t ONBOARD reverse geocode Feature Not Supported." << " - " << rc << endl;
        err = NE_NOSUPPORT;
    } else if (rc == Ncdb::NCDB_TIME_OUT) {
        qDebug() << "\t ONBOARD reverse geocode Time Out." << endl;
        err = NESERVER_TIMEDOUT;
    } else {
        qDebug() << "\t" << "ONBOARD reverse geocode FAILURE" << " - " << rc << endl;
        err = NESERVER_FAILED;
    }



    if (err != NE_OK) {
        m_requestPrivate->NotifyError(err);
    }

    return err;
}

void OnboardReverseGeocodeHandler::Cancel()
{
    // do nothing
}

bool OnboardReverseGeocodeHandler::IsInProgress()
{
    return false;
}


/*!
 *  ReverseGeocodeHandler will take ownership of the parameter
 */
void OnboardReverseGeocodeHandler::SetParameter(const std::shared_ptr<ReverseGeocodeParameter>& sparam)
{
    m_sparam = sparam;
}


/*! See the ReverseGeocodeRequest header.
 */
void OnboardReverseGeocodeHandler::SetInvocationContext(NB_Context* context, const char* inputSource, const char* invocationMethod, const char* screenId)
{
    // do nothing
}


}
