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
   @file        poiimageidentifierimplementation.cpp

   Identifier of a POI Image. This could be used to identify a POI image
   uniquely. It was exposed to PoiImageListener.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of Networks In
   Motion is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

#include "poiimageidentifierimplementation.h"

using namespace nbsearch;

// Refer to header file for documents.

PoiImageIdentifierImplementation::PoiImageIdentifierImplementation(shared_ptr<std::string> url,
                                                                   bool isThumbnail) : PoiImageIdentifier()
{
    m_url         = url;
    m_isThumbnail = isThumbnail;
}

PoiImageIdentifierImplementation::~PoiImageIdentifierImplementation()
{
}

bool
PoiImageIdentifierImplementation::IsIdentical(NB_SingleSearchInformation* information, int index,
                                              bool isThumbnail)
{
    if (information == NULL)
    {
        return false;
    }
    const char *url = NB_SingleSearchInformationGetPoiImageUrl(information, index, isThumbnail);
    return CompareToPoi(url, isThumbnail);
}

bool
PoiImageIdentifierImplementation::IsIdentical(NB_SearchInformation* information, int index,
                                              bool isThumbnail)
{
    if (information == NULL)
    {
        return false;
    }
    //@todo: not supported by now.
    return false;
}

bool
PoiImageIdentifierImplementation::CompareToPoi(const char* url, bool isThumbnail)
{
    bool  result    = false;
    if (url && (isThumbnail == m_isThumbnail) &&
        m_url && (m_url->compare(url) == 0))
    {
        result = true;
    }

    return result;
}

shared_ptr<std::string> PoiImageIdentifierImplementation::GetUrl()
{
    return m_url;
}

bool PoiImageIdentifierImplementation::IsThumbnail()
{
    return m_isThumbnail;
}

