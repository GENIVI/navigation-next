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
    @file     analyticslibxmlparser.cpp
*/
/*
    See file description in header file.
    (C) Copyright 2004 - 2014 by Networks In Motion, Inc.
    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/
/*! @{ */

#include "analyticslibxmlparser.h"
#define LIBXML_SCHEMAS_ENABLED
#include <libxml/xmlschemastypes.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlmemory.h>
#include "cslutil.h"
#include "nbmacros.h"
#include "palclock.h"
#include "transformutility.h"
#include "nbgpstypes.h"
#include "gpsutil.h"
using namespace nbcommon;

#define ANALYTICS_INT32_TYPE            "int32"
#define ANALYTICS_UINT32_TYPE           "uint32"
#define ANALYTICS_DOUBLE_TYPE           "double"
#define ANALYTICS_FLOAT_TYPE            "float"
#define ANALYTICS_BINARY_TYPE           "binary"
#define ANALYTICS_BYTE_TYPE             "byte"
#define ANALYTICS_UINT64_TYPE           "uint64"
#define ANALYTICS_STRING_TYPE           "string"
#define XML_METADATA                    "metadata"
#define XML_METADATA_NAME               "name"
#define XML_METADATA_CATEGORY           "category"
#define XML_METADATA_TIMESTAMP          "timestamp"
#define XML_METADATA_NAVIGATE_STATE     "route-state"
#define XML_ATTRIBUTE                   "attribute"
#define XML_IS_ATTRIBUTE                "true"
#define XML_NOT_ATTRIBUTE               "false"
#define XML_TYPE                        "type"

#define XML_SESSION_ID                  "nav-session-id"
#define XML_PACKING_VERSION             "packing-version"

#define GPS_LAT_OFFSET_MIN              -0.032767
#define GPS_LAT_OFFSET_MAX              0.032767
#define GPS_LON_OFFSET_MIN              -0.032767
#define GPS_LON_OFFSET_MAX              0.032767

NB_Error AnalyticsLibXmlParser::ValidateString(const std::string& event)
{
    /*! @TODO: Validate need to be uncommented , but because tps lib does not fit CCC_NEW schema now, so
    let it pass directly*/
    return NE_OK;
//     if(!m_bInitialized)
//     {
//         return NE_NOTINIT;
//     }
//     xmlDocPtr doc = xmlReadMemory(event.c_str(), event.length(), NULL, NULL, 0);
//
//     NB_Error error = NE_OK;
//     if (doc == NULL)
//     {
//         return NE_BADDATA;
//     }
//     else
//     {
//         xmlSchemaValidCtxtPtr ctxt = xmlSchemaNewValidCtxt(m_schemas);
//         int ret = xmlSchemaValidateDoc(ctxt, doc);
//         if (ret == 0)
//         {
//             error = NE_OK;
//         }
//         else if (ret > 0)
//         {
//             error = NE_BADDATA;
//         }
//         else
//         {
//             error = NE_UNEXPECTED;
//         }
//         xmlSchemaFreeValidCtxt(ctxt);
//         xmlFreeDoc(doc);
//     }
//     return error;

}

/*! If this node is an "attribute node", retrieve its name, value,and type out*/
static NB_Error ParseXmlAttr(xmlNodePtr node, std::string& attributeName, std::string& attributeValue, std::string& attributeType)
{
    if (node == NULL)
    {
        return NE_INVAL;
    }
    const xmlChar* name = NULL;
    xmlChar *type = NULL, *innerContent = NULL;
    name = node->name;
    innerContent = xmlNodeGetContent(node);
    type = xmlGetProp(node, (const xmlChar*)XML_TYPE);
    if(!name || !type || (xmlStrcmp(type, (const xmlChar*)ANALYTICS_STRING_TYPE) != 0 && !innerContent))
    {
        if(type)
        {
            xmlFree(type);
        }
        if(innerContent)
        {
            xmlFree(innerContent);
        }
        return NE_BADDATA;
    }

    attributeName = (char*)name;
    attributeValue = (char*)innerContent;
    attributeType = (char*)type;
    xmlFree(type);
    xmlFree(innerContent);
    return NE_OK;
}

/*! The status of seek attribute symbol*/
enum SeekAttributeStatus
{
    AS_ISATTRIBUTE,  /*This is an attribute*/
    AS_NOTATTRIBUTE, /*This is not an attribute*/
    AS_NULL          /*Such symbol not existing*/
};

/*! If it's an attribute, returns AS_ISATTRIBUTE, if not, returns AS_NOTATTRIBUTE, if something wrong, returns AS_NULL*/
static SeekAttributeStatus GetIsAttibute(xmlNodePtr node)
{
    xmlChar* isAttribute = xmlGetProp(node, (const xmlChar*)XML_ATTRIBUTE);

    if(isAttribute == NULL)
    {
        return AS_NULL;
    }

    SeekAttributeStatus nRet = AS_NOTATTRIBUTE;
    if (xmlStrcmp(isAttribute, (const xmlChar*)XML_IS_ATTRIBUTE) == 0)
    {
        nRet = AS_ISATTRIBUTE;
    }
    xmlFree(isAttribute);
    return nRet;

}

/*! Parse an node and convert it to tps element*/
static NB_Error ParseXmlElement(xmlNode* node, protocol::TpsElementPtr& ptr)
{
    if (node == NULL)
    {
        return NE_INVAL;
    }

    ptr = shared_ptr<protocol::TpsElement>(
          CCC_NEW protocol::TpsElement(CHAR_PTR_TO_UTF_STRING_PTR((char*)node->name)));

    if(!ptr)
    {
        return NE_UNEXPECTED;
    }

    for(xmlNodePtr pchild = node->children; pchild != NULL; pchild = pchild->next)
    {
        switch (pchild->type)
        {
        case XML_ELEMENT_NODE:
        {
            NB_Error error = NE_OK;
            SeekAttributeStatus nAttribute = GetIsAttibute(pchild);
            if (nAttribute == AS_ISATTRIBUTE)
            {
                std::string name, value, type;
                error = ParseXmlAttr(pchild, name, value, type);
                if(error != NE_OK)
                {
                    return error;
                }

                if (type == ANALYTICS_BINARY_TYPE)
                {
                    uint32 len = value.length() << 1;
                    char* val = CCC_NEW char[len];
                    if(!val)
                    {
                        return NE_UNEXPECTED;
                    }
                    nsl_memset(val, 0, len);
                    int binLen = decode_base64(value.c_str(), val);
                    if(binLen <=0 )
                    {
                        delete[] val;
                        return NE_BADDATA;
                    }
                    ptr->SetAttribute(CHAR_PTR_TO_UTF_STRING_PTR(name.c_str()), val, binLen);
                    delete[] val;
                }
                else if(type == ANALYTICS_BYTE_TYPE)
                {
                    byte val = (byte)nsl_atoi(value.c_str());
                    ptr->SetAttributeByte(CHAR_PTR_TO_UTF_STRING_PTR(name.c_str()), val);
                }
                else if(type == ANALYTICS_INT32_TYPE)
                {
                    int val = nsl_atoi(value.c_str());
                    ptr->SetAttributeInt32(CHAR_PTR_TO_UTF_STRING_PTR(name.c_str()), val);
                }
                else if(type == ANALYTICS_UINT32_TYPE)
                {
                    uint32 val = (uint32)nsl_atoll(value.c_str());
                    ptr->SetAttributeUint32(CHAR_PTR_TO_UTF_STRING_PTR(name.c_str()), val);
                }
                else if(type == ANALYTICS_UINT64_TYPE)
                {
                    uint64 val = nsl_strtoull(value.c_str(),NULL,10);
                    ptr->SetAttributeUint64(CHAR_PTR_TO_UTF_STRING_PTR(name.c_str()), val);
                }
                else if(type == ANALYTICS_FLOAT_TYPE)
                {
                    float val = (float)nsl_atof(value.c_str());
                    ptr->SetAttributeFloat(CHAR_PTR_TO_UTF_STRING_PTR(name.c_str()), val);
                }
                else if(type == ANALYTICS_DOUBLE_TYPE)
                {
                    double val = nsl_atof(value.c_str());
                    ptr->SetAttributeDouble(CHAR_PTR_TO_UTF_STRING_PTR(name.c_str()), val);
                }
                else if(type == ANALYTICS_STRING_TYPE)
                {
                    ptr->SetAttributeString(CHAR_PTR_TO_UTF_STRING_PTR(name.c_str()), CHAR_PTR_TO_UTF_STRING_PTR(value.c_str()));
                }
            }
            else if(nAttribute == AS_NOTATTRIBUTE)
            {
                protocol::TpsElementPtr sub;
                error = ParseXmlElement(pchild, sub);
                if(error!= NE_OK)
                {
                    return error;
                }
                ptr->Attach(sub);
            }
            else
            {
                return NE_UNEXPECTED;
            }
            break;
        }
        default:
            break;
        }
    }

    if(!ptr->IsValid())
    {
        return NE_UNEXPECTED;
    }

    return NE_OK;
}

/*! Separate Meta data from event body and read its information out*/
static NB_Error SeparateMetadataFromXml(xmlNode* node, std::string& strName, uint32& nCategory, int64& ntimeStamp,
                                        std::string& strNavigateState)
{
    if (node == NULL)
    {
        return NE_INVAL;
    }

    if(xmlStrcmp(node->name, (const xmlChar*)XML_METADATA) == 0)
    {
        xmlChar* name = NULL, *category = NULL, *navigateState = NULL;
        xmlNodePtr pchild = node->children;
        for (; pchild; pchild = pchild->next)
        {
            if(pchild->type == XML_ELEMENT_NODE)
            {
                if(xmlStrcmp(pchild->name, (xmlChar*)XML_METADATA_NAME) == 0)
                {
                    name = xmlNodeGetContent(pchild);
                }
                if(xmlStrcmp(pchild->name, (xmlChar*)XML_METADATA_CATEGORY) == 0)
                {
                    category = xmlNodeGetContent(pchild);
                }
                if(xmlStrcmp(pchild->name, (xmlChar*)XML_METADATA_NAVIGATE_STATE) == 0)
                {
                    navigateState = xmlNodeGetContent(pchild);
                }
            }
        }

        if(!name||!category)
        {
            if(name)
            {
                xmlFree(name);
            }
            if(category)
            {
                xmlFree(category);
            }
            if(navigateState)
            {
                xmlFree(navigateState);
            }
            return NE_BADDATA;
        }
        strName = (char*)name;
        nCategory = (uint32)atoi((char*)category);
        ntimeStamp = (int64)PAL_ClockGetGPSTime();
        if (navigateState)
        {
            strNavigateState = (char*)navigateState;
            xmlFree(navigateState);
        }
        xmlFree(name);
        xmlFree(category);
        xmlUnlinkNode(node);
        xmlFreeNode(node);
        return NE_OK;
    }

    NB_Error error = NE_BADDATA;
    for(xmlNodePtr pchild = node->children; pchild != NULL; pchild = pchild->next)
    {
        switch (pchild->type)
        {
        case XML_ELEMENT_NODE:
        {
            error = SeparateMetadataFromXml(pchild, strName, nCategory, ntimeStamp, strNavigateState);
        }
        default:
            break;
        }
        if(error == NE_OK)
        {
            break;
        }
    }
    return error;
}

NB_Error AnalyticsLibXmlParser::StringToTps(const std::string& event, protocol::TpsElementPtr& ptr)
{
    xmlDocPtr doc = xmlReadMemory(event.c_str(), event.length(), NULL, NULL, 0);
    if(!doc)
    {
        return NE_BADDATA;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if(!root)
    {
        xmlFreeDoc(doc);
        return NE_INVAL;
    }

    NB_Error error = ParseXmlElement(root, ptr);
    xmlFreeDoc(doc);
    return error;
}

NB_Error AnalyticsLibXmlParser::Initialize(const std::string& schema)
{
    if(m_bInitialized)
    {
        return NE_UNEXPECTED;
    }

    /*! @TODO: A schema need to be read here, but we do not need validation for now, so directly returns NE_OK*/
    m_bInitialized = true;
    return NE_OK;
//     xmlSchemaParserCtxtPtr ctxt;
//     xmlLineNumbersDefault(1);
//     ctxt = xmlSchemaNewMemParserCtxt(schema.c_str(), schema.length());
//     if(!ctxt)
//     {
//         return NE_BADDATA;
//     }
//     m_schemas = xmlSchemaParse(ctxt);
//     xmlSchemaFreeParserCtxt(ctxt);
//     m_bInitialized = m_schemas? true:false;
//     return m_schemas?NE_OK:NE_BADDATA;
}

AnalyticsLibXmlParser::~AnalyticsLibXmlParser()
{
    if(!m_bInitialized)
    {
        return;
    }

    if(m_schemas != NULL)
    {
        xmlSchemaFree(m_schemas);
    }
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();
}

AnalyticsLibXmlParser::AnalyticsLibXmlParser():
    m_schemas(NULL),
    m_bInitialized(false)
{
}

NB_Error AnalyticsLibXmlParser::SeparateMetadata(const std::string& event,
                                                 AnalyticsMetadataContent& content,
                                                 std::string& eventBody)
{
    xmlDocPtr doc = xmlReadMemory(event.c_str(), event.length(), NULL, NULL, 0);
    if(!doc)
    {
        return NE_BADDATA;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if(!root)
    {
        xmlFreeDoc(doc);
        return NE_BADDATA;
    }

    NB_Error error = SeparateMetadataFromXml(root, content.name, content.category, content.timeStamp, content.navigateState);
    if(error != NE_OK)
    {
        xmlFreeDoc(doc);
        return error;
    }

    xmlChar* xml = NULL;
    int len;
    xmlDocDumpMemory(doc, &xml, &len);
    if(len<=0)
    {
        xmlFreeDoc(doc);
        return NE_BADDATA;
    }

    eventBody.clear();
    eventBody.append((char*)xml, len);
    xmlFree(xml);
    xmlFreeDoc(doc);
    return NE_OK;
}

/**
 * GPSXML
 */
GPSXML::GPSXML(std::string const& xmlText): m_navSessionID((uint32)-1), m_packingVersion()
{
    AnalyticsXML xml(xmlText);
    m_root = xml.FindElementByName("gps-probes-event");

    if (m_root == NULL)
    {
        return;
    }

    shared_ptr<AnalyticsXMLElement> xmlElement(m_root->FindChildElementByName(XML_SESSION_ID));
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_UINT32_TYPE)
        {
            m_navSessionID = xmlElement->GetContentUInt32Value();
        }
    }

    xmlElement = m_root->FindChildElementByName(XML_PACKING_VERSION);
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_STRING_TYPE)
        {
            m_packingVersion = xmlElement->GetContentStdStringValue();
        }
    }
}

GPSXML::GPSXML(shared_ptr<AnalyticsXMLElement> const& root): m_root(root), m_navSessionID((uint32)-1), m_packingVersion()
{
    if (root == NULL)
    {
        return;
    }

    shared_ptr<AnalyticsXMLElement> xmlElement(m_root->FindChildElementByName(XML_SESSION_ID));
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_UINT32_TYPE)
        {
            m_navSessionID = xmlElement->GetContentUInt32Value();
        }
    }

    xmlElement = m_root->FindChildElementByName(XML_PACKING_VERSION);
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_STRING_TYPE)
        {
            m_packingVersion = xmlElement->GetContentStdStringValue();
        }
    }
}

void GPSXML::ToGPS(NB_GpsLocation& gpsLocation) const
{
    gpsLocation.status = NE_OK;

    shared_ptr<AnalyticsXMLElement> xmlElement(m_root->FindChildElementByName("lat"));
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_DOUBLE_TYPE)
        {
            gpsLocation.latitude = xmlElement->GetContentDoubleValue();
            gpsLocation.valid |= NGV_Latitude;
        }
    }

    xmlElement = m_root->FindChildElementByName("lon");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_DOUBLE_TYPE)
        {
            gpsLocation.longitude = xmlElement->GetContentDoubleValue();
            gpsLocation.valid |= NGV_Longitude;
        }
    }

    xmlElement = m_root->FindChildElementByName("altitude");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_FLOAT_TYPE)
        {
            gpsLocation.altitude = xmlElement->GetContentFloatValue();
            gpsLocation.valid |= NGV_Altitude;
        }
    }

    xmlElement = m_root->FindChildElementByName("heading");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_FLOAT_TYPE)
        {
            gpsLocation.heading = xmlElement->GetContentFloatValue();
            gpsLocation.valid |= NGV_Heading;
        }
    }

    xmlElement = m_root->FindChildElementByName("speed");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_FLOAT_TYPE)
        {
            gpsLocation.horizontalVelocity = xmlElement->GetContentFloatValue();
            gpsLocation.valid |= NGV_HorizontalVelocity;
        }
    }

    xmlElement = m_root->FindChildElementByName("time");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_UINT64_TYPE)
        {
            gpsLocation.gpsTime = (nb_gpsTime)xmlElement->GetContentUInt64Value();
            // Time flag not found in NGV_....
        }
    }

    xmlElement = m_root->FindChildElementByName("ua");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_FLOAT_TYPE)
        {
            gpsLocation.horizontalUncertaintyAlongAxis = xmlElement->GetContentFloatValue();
            gpsLocation.valid |= NGV_AxisUncertainty;
        }
    }

    xmlElement = m_root->FindChildElementByName("uang");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_FLOAT_TYPE)
        {
            gpsLocation.horizontalUncertaintyAngleOfAxis = xmlElement->GetContentFloatValue();
            gpsLocation.valid |= NGV_HorizontalUncertainty;
        }
    }

    // Umag is not found in NB_GpsLocation.

    xmlElement = m_root->FindChildElementByName("up");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_FLOAT_TYPE)
        {
            gpsLocation.horizontalUncertaintyAlongPerpendicular = xmlElement->GetContentFloatValue();
            gpsLocation.valid |= NGV_PerpendicularUncertainty;
        }
    }
}

/**
 * GPSChecker
 */

bool GPSChecker::Check(const NB_GpsLocation& gpsLocation)
{
    bool checked = true;

    if (gpsLocation.gpsTime == 0 ||
        gpsLocation.latitude == INVALID_LATITUDE ||
        gpsLocation.longitude == INVALID_LONGITUDE)
    {
        checked = false;
    }

    return checked;
}

bool GPSChecker::CheckRelativity(const NB_GpsLocation& gpsLocation1, const NB_GpsLocation& gpsLocation2)
{
    if (!Check(gpsLocation1) || !Check(gpsLocation2))
    {
        return false;
    }

    bool checked = true;

    do
    {
        uint64 timeOffset = gpsLocation2.gpsTime - gpsLocation1.gpsTime;
        if (timeOffset > 255)
        {
            checked = false;
            break;
        }

        double latOffset = gpsLocation2.latitude - gpsLocation1.latitude;
        if ((latOffset < GPS_LAT_OFFSET_MIN) ||
            (latOffset > GPS_LAT_OFFSET_MAX))
        {
            checked = false;
            break;
        }

        double lonOffset = gpsLocation2.longitude - gpsLocation1.longitude;
        if ((lonOffset < GPS_LON_OFFSET_MIN) ||
            (lonOffset > GPS_LON_OFFSET_MAX))
        {
            checked = false;
            break;
        }


    } while (0);

    return checked;
}

InvocationContextXML::InvocationContextXML(std::string const& xmlText)
{
    AnalyticsXML xml(xmlText);
    m_root = xml.FindElementByName("invocation-context");

    InitializeWithXMLNode(m_root); // The pointer to root node has been checked inside.
}

InvocationContextXML::InvocationContextXML(shared_ptr<AnalyticsXMLElement> const& root): m_root(root)
{
    InitializeWithXMLNode(root); // The pointer to root node has been checked inside.
}

NB_Error InvocationContextXML::ToInvocationContext(data_util_state* pds, data_invocation_context& dic) const
{
    NB_Error err = NE_OK;

    do {
        data_string_set(pds, &dic.input_source, m_inputSourceValue.c_str());
        if (err != NE_OK)
        {
            break;
        }

        data_string_set(pds, &dic.invocation_method, m_invocationMethodValue.c_str());
        if (err != NE_OK)
        {
            break;
        }

        data_string_set(pds, &dic.screen_id, m_screenIDValue.c_str());
        if (err != NE_OK)
        {
            break;
        }

        data_string_set(pds, &dic.source_module, m_sourceModuleValue.c_str());
        if (err != NE_OK)
        {
            break;
        }

    } while (0);

    return err;
}

void InvocationContextXML::InitializeWithXMLNode(shared_ptr<AnalyticsXMLElement> const& root)
{
    if (root == NULL)
    {
        return;
    }

    shared_ptr<AnalyticsXMLElement> xmlElement(m_root->FindChildElementByName("input-source"));
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_STRING_TYPE)
        {
            m_inputSourceValue = xmlElement->GetContentStdStringValue();
        }
    }

    xmlElement = m_root->FindChildElementByName("invocation-method");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_STRING_TYPE)
        {
            m_invocationMethodValue = xmlElement->GetContentStdStringValue();
        }
    }

    xmlElement = m_root->FindChildElementByName("screen-id");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_STRING_TYPE)
        {
            m_screenIDValue = xmlElement->GetContentStdStringValue();
        }
    }

    xmlElement = m_root->FindChildElementByName("source-module");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_STRING_TYPE)
        {
            m_sourceModuleValue = xmlElement->GetContentStdStringValue();
        }
    }
}

AnalyticsEventXML::AnalyticsEventXML(std::string const& xmlText): m_xml(xmlText), m_idValue((uint64)-1), m_sessionIDValue((uint64)-1), m_tsValue((uint64)-1)
{
    shared_ptr<AnalyticsXMLElement> xmlElement(m_xml.FindElementByName("active-session"));
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_STRING_TYPE)
        {
            m_activeSessionValue = xmlElement->GetContentStdStringValue();
        }
    }

    xmlElement = m_xml.FindElementByName("id");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_UINT64_TYPE)
        {
            m_idValue = xmlElement->GetContentUInt64Value();
        }
    }

    xmlElement = m_xml.FindElementByName("session-id");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_UINT64_TYPE)
        {
            m_sessionIDValue = xmlElement->GetContentUInt64Value();
        }
    }

    xmlElement = m_xml.FindElementByName("ts");
    if (xmlElement != NULL && xmlElement->IsAttributeTrue())
    {
        if (xmlElement->GetTypeValue() == ANALYTICS_UINT64_TYPE)
        {
            m_tsValue = xmlElement->GetContentUInt64Value();
        }
    }

    m_gpsProbes = m_xml.FindElementByName("gps-probes-event");

    m_invocationContext = m_xml.FindElementByName("invocation-context");
}

bool AnalyticsEventXML::IsGroupedWith(AnalyticsEventXML const& newOne) const
{
    GPSXML gps1(m_gpsProbes);
    GPSXML gps2(newOne.m_gpsProbes);

    NB_GpsLocation gpsLocation1, gpsLocation2;
    gps1.ToGPS(gpsLocation1);
    gps2.ToGPS(gpsLocation2);

    return GPSChecker::CheckRelativity(gpsLocation1, gpsLocation2);
}

bool AnalyticsEventXML::IsGPSProbesChecked() const
{
    GPSXML gps(m_gpsProbes);

    NB_GpsLocation gpsLocation;
    gps.ToGPS(gpsLocation);

    bool checked = GPSChecker::Check(gpsLocation);
    return checked;
}

void AnalyticsEventXML::ToGPS(NB_GpsLocation& gpsLocation) const
{
    GPSXML gps(m_gpsProbes);
    gps.ToGPS(gpsLocation);
}

NB_Error AnalyticsEventXML::ToInvocationContext(data_util_state* pds, data_invocation_context& dic) const
{
    InvocationContextXML invocationContext(m_invocationContext);
    return invocationContext.ToInvocationContext(pds, dic);
}

void AnalyticsEventXMLGroup::Add(AnalyticsEventXML const& newPoint)
{
    m_points.push_back(newPoint);
}

NB_Error AnalyticsEventXMLGroup::ToGPSArray(CSL_Vector*& gpsArray) const
{
    if (m_points.empty() || gpsArray == NULL)
    {
        return NE_BADDATA;
    }

    for (std::vector<AnalyticsEventXML>::size_type i = 0; i < m_points.size(); ++i)
    {
        NB_GpsLocation gpsLocation;
        nsl_memset(&gpsLocation, 0, sizeof(NB_GpsLocation));

        m_points[i].ToGPS(gpsLocation);
        CSL_VectorAppend(gpsArray, &gpsLocation);
    }

    return NE_OK;
}

NB_Error AnalyticsEventXMLGroup::ToAnalyticsEvent(data_util_state* pds, data_analytics_event& dae) const
{
    NB_Error err = NE_OK;

    do {
        // gps-probe-event
        data_gps_probes_event gpsProbeEvent;
        DATA_INIT(pds, err, &gpsProbeEvent, data_gps_probes_event);
        if (err != NE_OK)
        {
            break;
        }

        err = this->ToGPSArray(gpsProbeEvent.gps_locations);
        if (err != NE_OK)
        {
            DATA_FREE(pds, &gpsProbeEvent, data_gps_probes_event);
            break;
        }

        AnalyticsEventXML const& frontOne = GetFrontOne();
        GPSXML gpsProbes(frontOne.GetGPSXML());

        gpsProbeEvent.nav_session_id = gpsProbes.GetSessionID();
        err = data_string_set(NULL, &gpsProbeEvent.packing_version, gpsProbes.GetPackingVersion().c_str());
        if (err != NE_OK)
        {
            DATA_FREE(pds, &gpsProbeEvent, data_gps_probes_event);
            break;
        }

        // analytics-event
        data_analytics_event analyticsEvent;
        DATA_INIT(pds, err, &analyticsEvent, data_analytics_event);
        if (err != NE_OK)
        {
            DATA_FREE(pds, &gpsProbeEvent, data_gps_probes_event);
            break;
        }

        err = data_analytics_event_set_event_type(pds, &analyticsEvent, analytics_event_gps_probes);
        if (err != NE_OK)
        {
            DATA_FREE(pds, &gpsProbeEvent, data_gps_probes_event);
            DATA_FREE(pds, &analyticsEvent, data_analytics_event);
            break;
        }

        DATA_COPY(pds, err, &analyticsEvent.event.gps_probes_event, &gpsProbeEvent, data_gps_probes_event);
        if (err != NE_OK)
        {
            DATA_FREE(pds, &gpsProbeEvent, data_gps_probes_event);
            DATA_FREE(pds, &analyticsEvent, data_analytics_event);
            break;
        }

        err = frontOne.ToInvocationContext(pds, analyticsEvent.invocation_context);
        if (err != NE_OK)
        {
            DATA_FREE(pds, &gpsProbeEvent, data_gps_probes_event);
            DATA_FREE(pds, &analyticsEvent, data_analytics_event);
            break;
        }

        data_string_set(pds, &analyticsEvent.active_session, frontOne.GetActiveSession().c_str());
        analyticsEvent.id = (uint32)frontOne.GetID();
        analyticsEvent.ts = (uint32)frontOne.GetTS();
        analyticsEvent.session_id = (uint32)frontOne.GetSessionID();

        DATA_COPY(pds, err, &dae, &analyticsEvent, data_analytics_event);
        if (err != NE_OK)
        {
            DATA_FREE(pds, &gpsProbeEvent, data_gps_probes_event);
            DATA_FREE(pds, &analyticsEvent, data_analytics_event);
            break;
        }

        DATA_FREE(pds, &gpsProbeEvent, data_gps_probes_event);
        DATA_FREE(pds, &analyticsEvent, data_analytics_event);

    } while (0);

    return err;
}

void AnalyticsEventXMLList::Add(AnalyticsEventXML const& event)
{
    if (m_eventVecs.empty())
    {
        std::vector<AnalyticsEventXML> events;
        events.push_back(event);
        m_eventVecs.push_back(events);
    }
    else
    {
        AnalyticsEventXML const& lastOne = m_eventVecs.back().back();
        if (lastOne.IsGroupedWith(event))
        {
            m_eventVecs.back().push_back(event);
        }
        else
        {
            std::vector<AnalyticsEventXML> events;
            events.push_back(event);
            m_eventVecs.push_back(events);
        }
    }
}

NB_Error AnalyticsEventXMLList::ToAnalyticsEventList(data_util_state* pds, data_analytics_event*& events, std::size_t& size) const
{
    if (events != NULL || m_eventVecs.empty())
    {
        return NE_BADDATA;
    }

    data_analytics_event* eventList = (data_analytics_event*)nsl_malloc(sizeof(data_analytics_event) * m_eventVecs.size());
    if (eventList == NULL)
    {
        return NE_NOMEM;
    }

    NB_Error err = NE_OK;

    for (std::vector<std::vector<AnalyticsEventXML> >::size_type i = 0; i < m_eventVecs.size(); ++i)
    {
        AnalyticsEventXMLGroup group;

        for (std::vector<AnalyticsEventXML>::size_type ii = 0; ii < m_eventVecs[i].size(); ++ii)
        {
            group.Add(m_eventVecs[i][ii]);
        }

        if (!group.Empty())
        {
            data_analytics_event* event = eventList + i;
            DATA_INIT(pds, err, event, data_analytics_event);
            if (err != NE_OK)
            {
                for (std::size_t j = 0; j < i; ++j)
                {
                    DATA_FREE(pds, eventList + j, data_analytics_event);
                }

                nsl_free(eventList);
                eventList = NULL;

                return err;
            }

            err = group.ToAnalyticsEvent(pds, *event);
            if (err != NE_OK)
            {
                for (std::size_t j = 0; j < i; ++j)
                {
                    DATA_FREE(pds, eventList + j, data_analytics_event);
                }

                nsl_free(eventList);
                eventList = NULL;

                return err;
            }
        }
    }

    events = eventList;
    size = m_eventVecs.size();

    return err;
}

/*!
 * @}
*/
