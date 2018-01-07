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

 @file     analyticsxml.cpp
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/* @{ */

#include "palstdlib.h"
#include "nbmacros.h"
#include "analyticsxml.h"

#define XML_ATTRIBUTE                   "attribute"
#define XML_IS_ATTRIBUTE                "true"
#define XML_NOT_ATTRIBUTE               "false"
#define XML_TYPE                        "type"

AnalyticsXMLElement::AnalyticsXMLElement(xmlNodePtr node): m_isValid(true)
{
    if (node == NULL || node->type != XML_ELEMENT_NODE)
    {
        m_isValid = false;
        return;
    }

    if (node->name != NULL)
    {
        m_nodeName.assign(reinterpret_cast<const char*>(node->name));
    }

    xmlChar* attributeValue = xmlGetProp(node, reinterpret_cast<const xmlChar*>(XML_ATTRIBUTE));
    if (attributeValue != NULL)
    {
        m_attributeValue.assign(reinterpret_cast<const char*>(attributeValue));
        xmlFree(attributeValue);
    }

    xmlChar* typeValue = xmlGetProp(node, reinterpret_cast<const xmlChar*>(XML_TYPE));
    if (typeValue != NULL)
    {
        m_typeValue.assign(reinterpret_cast<const char*>(typeValue));
        xmlFree(typeValue);
    }

    xmlChar* nodeContent = xmlNodeGetContent(node);
    if (nodeContent != NULL && IsAttributeTrue())
    {
        m_nodeContent.assign(reinterpret_cast<const char*>(nodeContent));
    }
    if (nodeContent != NULL)
    {
        xmlFree(nodeContent);
    }

    if (node->children != NULL)
    {
        xmlNodePtr children = node->children;
        for (; children != NULL; children = children->next)
        {
            if (children->type == XML_ELEMENT_NODE)
            {
                shared_ptr<AnalyticsXMLElement> newChildNode(CCC_NEW AnalyticsXMLElement(children));
                m_children.push_back(newChildNode);
            }
        }
    }
}

bool AnalyticsXMLElement::IsAttributeTrue() const
{
    return HasAttribute() && m_attributeValue == XML_IS_ATTRIBUTE;
}

shared_ptr<AnalyticsXMLElement> AnalyticsXMLElement::FindChildElementByName(std::string const& nodeName) const
{
    nsl_assert(nodeName.empty() == false);

    shared_ptr<AnalyticsXMLElement> found;

    if (m_children.empty())
    {
        return found;
    }

    for (std::vector<shared_ptr<AnalyticsXMLElement> >::size_type i = 0; i < m_children.size(); ++i)
    {
        if (m_children[i]->GetNodeName() == nodeName)
        {
            found = m_children[i];
            break;
        }
        else
        {
            found = m_children[i]->FindChildElementByName(nodeName);
            if (found != NULL)
            {
                break;
            }
        }
    }

    return found;
}

float AnalyticsXMLElement::GetContentFloatValue() const
{
    double value = nsl_atof(m_nodeContent.c_str());
    return static_cast<float>(value);
}

double AnalyticsXMLElement::GetContentDoubleValue() const
{
    double value = nsl_atof(m_nodeContent.c_str());
    return value;
}

uint64 AnalyticsXMLElement::GetContentUInt64Value() const
{
    uint64 value = nsl_strtoull(m_nodeContent.c_str(), NULL, 10);
    return value;
}

uint32 AnalyticsXMLElement::GetContentUInt32Value() const
{
    uint32 value = nsl_atoi(m_nodeContent.c_str());
    return value;
}

AnalyticsXML::AnalyticsXML(std::string const& xml): m_root()
{
    xmlDocPtr doc = xmlReadMemory(xml.c_str(), xml.length(), NULL, NULL, 0);
    if (doc == NULL)
    {
        return;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root != NULL && root->type == XML_ELEMENT_NODE)
    {
        m_root.reset(CCC_NEW AnalyticsXMLElement(root));
    }

    xmlFreeDoc(doc);
}

shared_ptr<AnalyticsXMLElement> AnalyticsXML::FindElementByName(std::string const& nodeName) const
{
    shared_ptr<AnalyticsXMLElement> found, root(GetRootElement());

    if (root != NULL)
    {
        if (root->GetNodeName() == nodeName)
        {
            return root;
        }

        found = root->FindChildElementByName(nodeName);
    }

    return found;
}

/* @} */
