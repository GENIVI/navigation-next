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

 @file     analyticsxml.h
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

#ifndef __nbservices__analyticsxml__
#define __nbservices__analyticsxml__

#include <string>
#include <vector>
#include <libxml/tree.h>
#include "smartpointer.h"
#include "paltypes.h"

/*! @class AnalyticsXMLElement

 \brief A XML element for Analytics.
 */
class AnalyticsXMLElement
{
public:
    AnalyticsXMLElement(xmlNodePtr node);
    bool                        IsValid() const             { return m_isValid; }
    std::string const&          GetNodeName() const         { return m_nodeName; }
    std::string const&          GetAttributeValue() const   { return m_attributeValue; }
    bool                        HasAttribute() const        { return !m_attributeValue.empty(); }
    bool                        IsAttributeTrue() const;
    std::string const&          GetTypeValue() const        { return m_typeValue; }
    bool                        HasType() const             { return !m_typeValue.empty(); }
    std::string const&          GetNodeContent() const      { return m_nodeContent; }
    bool                        HasNodeContent() const      { return !m_nodeContent.empty(); }
    shared_ptr<AnalyticsXMLElement> FindChildElementByName(std::string const& nodeName) const;
    float                       GetContentFloatValue() const;
    double                      GetContentDoubleValue() const;
    uint64                      GetContentUInt64Value() const;
    uint32                      GetContentUInt32Value() const;
    std::string                 GetContentStdStringValue() const { return m_nodeContent; }

private:
    bool                        m_isValid;
    std::string                 m_nodeName;
    std::string                 m_attributeValue;
    std::string                 m_typeValue;
    std::string                 m_nodeContent;
    std::vector<shared_ptr<AnalyticsXMLElement> > m_children;
};

/*! @class AnalyticsXML

 \brief A XML tree for Analytics.
 */
class AnalyticsXML
{
public:
    AnalyticsXML(std::string const& xml);
    shared_ptr<AnalyticsXMLElement> const&      GetRootElement() const { return m_root; }
    shared_ptr<AnalyticsXMLElement>             FindElementByName(std::string const& nodeName) const;

private:
    shared_ptr<AnalyticsXMLElement>             m_root;
};

#endif /* defined(__nbservices__analyticsxml__) */

/* @} */
