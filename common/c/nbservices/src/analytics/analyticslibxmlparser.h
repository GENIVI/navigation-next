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
@file     analyticslibxmlparser.h
*
*\brief A tool class used to convert xml into tps format
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

/*!
    @{
*/
#ifndef __ANALYTICS__LIBXML__PARSER_H_
#define __ANALYTICS__LIBXML__PARSER_H_

#include "nberror.h"
#include "TpsElement.h"
extern "C" {
#include "datautil.h"
#include "data_unpacked_gps.h"
#include "data_gps.h"
#include "data_gps_offset.h"
#include "data_invocation_context.h"
#include "data_analytics_event.h"
#include "nbgpstypes.h"
}
#include <string>
#include <libxml/tree.h>

#include "analyticsxml.h"

struct _xmlSchema;

namespace nbcommon
{

/*! @class AnalyticsLibXmlParser

\brief A tool class used to parse an xml string
*/
class AnalyticsLibXmlParser
{
public:
    /*!This structure contains information in meta data*/
    struct AnalyticsMetadataContent
    {
        uint32          category;         /*!The event category*/
        int64           timeStamp;        /*!The event timestamp*/
        std::string     name;             /*!The event name*/
        std::string     navigateState;    /*!The navigate state, current just for "route-state-event",
                                          this event will be filtered if state is "navigating"*/
    };

public:
    AnalyticsLibXmlParser();
    ~AnalyticsLibXmlParser();

    /*! Initialize the Parser class

    @returns NB_Error If initialize is successful, returns NE_OK, else returns an error code.
    */
    NB_Error Initialize(const std::string& schema);

    /*! The executer of ValidateString

    @param event String: A input string waiting for validation.
    @returns NB_Error If event format is correct, returns NE_OK, otherwise returns one error code.
    */
    NB_Error ValidateString(const std::string& event);

    /*! This function is used to separate metadata from event string

    After execution, content in metadata(name, type, timeStamp) will be read out, and an eventBody without metadata will also be read out.
    @param event The event string to separate.
    @param content The structure contains in meta data.
    @param eventBody The tps event body left after parsing.
    @returns NB_Error, Returns NE_OK if executed successfully, otherwise returns an error code.
    */
    static NB_Error StringToTps(const std::string& event, protocol::TpsElementPtr& ptr);

    /*! Convert a string to tps

    @param event: An input string.
    @param ptr TpsElementPtr structure
    @returns NB_Error Returns NE_OK if successful.
    */
    static NB_Error SeparateMetadata(const std::string& event,
                                     AnalyticsMetadataContent& content,
                                     std::string& eventBody);
private:
    _xmlSchema* m_schemas;
    bool        m_bInitialized;
};

/*! @class GPSXML

 \brief A XML text for GPS probes event.
 */
class GPSXML
{
public:
    GPSXML(std::string const& xmlText);
    GPSXML(shared_ptr<AnalyticsXMLElement> const& root);
    void                                        ToGPS(NB_GpsLocation& gpsLocation) const;
    uint32                                      GetSessionID() const        { return m_navSessionID; }
    std::string const&                          GetPackingVersion() const   { return m_packingVersion; }

private:
    shared_ptr<AnalyticsXMLElement>             m_root;
    uint32                                      m_navSessionID;
    std::string                                 m_packingVersion;
};

/*! @class GPSChecker

 \brief Check whether the GPS point is valid or not.
 */
class GPSChecker
{
public:
    static bool                                 Check(const NB_GpsLocation& gpsLocation);
    static bool                                 CheckRelativity(const NB_GpsLocation& gpsLocation1, const NB_GpsLocation& gpsLocation2);
};

/*! @class InvocationContextXML
 */
class InvocationContextXML
{
public:
    InvocationContextXML(std::string const& xmlText);
    InvocationContextXML(shared_ptr<AnalyticsXMLElement> const& root);
    NB_Error                                    ToInvocationContext(data_util_state* pds, data_invocation_context& dic) const;

private:
    void                                        InitializeWithXMLNode(shared_ptr<AnalyticsXMLElement> const& root);

    shared_ptr<AnalyticsXMLElement>             m_root;
    std::string                                 m_inputSourceValue;
    std::string                                 m_invocationMethodValue;
    std::string                                 m_screenIDValue;
    std::string                                 m_sourceModuleValue;
};

/*! @class AnalyticsEventXML
 */
class AnalyticsEventXML
{
public:
    AnalyticsEventXML(std::string const& xmlText);

    /*! Get attributes from a xml tree.
     */
    std::string                                 GetActiveSession() const    { return m_activeSessionValue; }
    uint64                                      GetID() const               { return m_idValue; }
    uint64                                      GetSessionID() const        { return m_sessionIDValue; }
    uint64                                      GetTS() const               { return m_tsValue; }
    GPSXML                                      GetGPSXML() const           { return GPSXML(m_gpsProbes); }

    /*! Calculation between Analytics Events.
     */
    bool                                        IsGroupedWith(AnalyticsEventXML const& newOne) const;
    bool                                        IsGPSProbesChecked() const;

    /*! Conversion to TPS classes.
     */
    void                                        ToGPS(NB_GpsLocation& gpsLocation) const;
    NB_Error                                    ToInvocationContext(data_util_state* pds, data_invocation_context& dic) const;

private:
    AnalyticsXML                                m_xml;
    std::string                                 m_activeSessionValue;
    uint64                                      m_idValue;
    uint64                                      m_sessionIDValue;
    uint64                                      m_tsValue;
    shared_ptr<AnalyticsXMLElement>             m_gpsProbes;
    shared_ptr<AnalyticsXMLElement>             m_invocationContext;
};

/*! @class AnalyticsEventXMLGroup
 */
class AnalyticsEventXMLGroup
{
public:
    void                                        Add(AnalyticsEventXML const& newPoint);

    NB_Error                                    ToGPSArray(CSL_Vector*& gpsArray) const;

    AnalyticsEventXML const&                    GetFrontOne() const { return m_points.front(); }
    bool                                        Empty() const { return m_points.empty(); }

    NB_Error                                    ToAnalyticsEvent(data_util_state* pds, data_analytics_event& dae) const;

private:
    std::vector<AnalyticsEventXML>              m_points;
};

/*! @class AnalyticsEventXMLList
 */
class AnalyticsEventXMLList
{
public:
    void                                        Add(AnalyticsEventXML const& event);
    NB_Error                                    ToAnalyticsEventList(data_util_state* pds, data_analytics_event*& events, std::size_t& size) const;

private:
    std::vector<std::vector<AnalyticsEventXML> > m_eventVecs;
};

}

#endif /*__ANALYTICS__LIBXML__PARSER_H_*/

/*!
    @}
*/
