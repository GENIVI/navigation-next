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

    @file       downloadrequest.cpp

    See header file for description.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

extern "C"
{
#include "palstdlib.h"
}

#include "downloadrequest.h"
#include "downloadmanager.h"
#include "datastream.h"

/*! @{ */

using namespace std;
using namespace nbcommon;

// Local Constants ..............................................................................

static const char COLON[]           = ":";
static const char RETURN_LINEFEED[] = "\r\n";
static const char SPACE[]           = " ";
static const char TAB[]             = "\t";


// Local Types ..................................................................................


// Public functions .............................................................................

/* See header file for description */
DownloadRequest::~DownloadRequest()
{
    // Nothing to do here.
}

/* See header file for description */
uint32
DownloadRequest::GetPriority() const
{
    return m_priority;
}

/* See header file for description */
shared_ptr<string>
DownloadRequest::GetRequestHeaders() const
{
    return m_requestHeaders;
}

/* See header file for description */
NB_Error
DownloadRequest::AddContentForRequestHeaderName(const string& name,
                                                const string& content)
{
    // Field name cannot be empty string. But field content is optional.
    if (name.empty())
    {
        return NE_INVAL;
    }

    // Initialize the string of request headers if it is NULL.
    if (!m_requestHeaders)
    {
        string* requestHeadersString = new string("");
        if (!requestHeadersString)
        {
            return NE_NOMEM;
        }

        m_requestHeaders = shared_ptr<std::string>(requestHeadersString);
    }

    /* Append the field name and content with a colon and a space saperated. And then
       add a CRLF for both headers separated. (field-name: field-content\r\n)
    */
    {
        string& requestHeadersReference = *m_requestHeaders;

        requestHeadersReference += name;
        requestHeadersReference += COLON;
        requestHeadersReference += SPACE;
        requestHeadersReference += content;
        requestHeadersReference += RETURN_LINEFEED;
    }

    return NE_OK;
}

/* See header file for description */
void
DownloadRequest::SetRequestHeaders(shared_ptr<string> requestHeaders)
{
    m_requestHeaders = requestHeaders;
}

/* See header file for description */
void
DownloadRequest::SetResponseHeaders(shared_ptr<string> responseHeaders)
{
    m_responseHeaders = responseHeaders;
}

/* See header file for description */
shared_ptr<string>
DownloadRequest::GetContentByResponseHeaderName(const string& name)
{
    int position = 0;
    int contentStart = 0;
    int nameWithColonLength = 0;
    int responseHeadersLength = 0;
    int spaceLength = nsl_strlen(SPACE);
    int tabLength = nsl_strlen(TAB);
    int returnLinefeedLength = nsl_strlen(RETURN_LINEFEED);
    string nameWithColon;

    // Check if field name is an empty string or there is no response headers.
    if (name.empty() ||
        (!m_responseHeaders) ||
        m_responseHeaders->empty())
    {
        return shared_ptr<string>();
    }

    // Append the field name with a colon.
    nameWithColon = name + COLON;
    nameWithColonLength = nameWithColon.size();

    // Try to find the specified field name in the header.
    responseHeadersLength = m_responseHeaders->size();
    while (position < responseHeadersLength)
    {
        // Find the field name with a colon and ignore the case of the string.
        position = FindStringCaseInsensitive(*m_responseHeaders, nameWithColon, position);
        if (position < 0)
        {
            return shared_ptr<string>();
        }

        if (position < returnLinefeedLength)
        {
            // It is the first field of the header.
            break;
        }

        /* Check if there is a CRLF before the found position. Because it is a rule to
           check if it is a field name.
        */
        if (m_responseHeaders->compare(position - returnLinefeedLength,
                                       returnLinefeedLength,
                                       RETURN_LINEFEED) == 0)
        {
            // The specified field name is found.
            break;
        }

        // The found string is not a field name. Try to find another one.
        position += nameWithColonLength;
    }

    // Locate to the end of the field name.
    position += nameWithColonLength;
    if (position >= responseHeadersLength)
    {
        return shared_ptr<string>();
    }

    /* Filter optional linear white space (LWS) and get the starting position of the
       field content.
    */
    position = FilterLinearWhiteSpace(*m_responseHeaders, position);
    contentStart = position;

    // Find the end of the field content.
    while (position < responseHeadersLength)
    {
        int nextLinePosition = 0;

        position = m_responseHeaders->find(RETURN_LINEFEED, position);
        if (position < 0)
        {
            // This is the last field content.
            position = responseHeadersLength;
            break;
        }

        // Check if the next line starts with space or tab.
        nextLinePosition = position + returnLinefeedLength;
        if ((m_responseHeaders->compare(nextLinePosition, spaceLength, SPACE) != 0) &&
            (m_responseHeaders->compare(nextLinePosition, tabLength, TAB) != 0))
        {
            // The end of this field content is found. It saves in 'position'.
            break;
        }

        position = nextLinePosition;
    }

    // The end of the field content is not greater than the length of the header.
    position = (position <= responseHeadersLength) ? position : responseHeadersLength;

    if (contentStart == position)
    {
        /* Return an empty string if the field content is empty. Cannot return NULL here.
           Because the field name is found.
        */
        return shared_ptr<string>(new string(""));
    }
    return shared_ptr<string>(new string(*m_responseHeaders, contentStart, position - contentStart));
}

/* See header file for description */
nb_boolean
DownloadRequest::GetInProgressFlag() const
{
    return m_isInProgress;
}

/* See header file for description */
void
DownloadRequest::SetInProgressFlag(nb_boolean isInProgress)
{
    m_isInProgress = isInProgress;
}

/* See header file for description */
DataStreamPtr
DownloadRequest::GetDataStream() const
{
    return m_downloadData;
}

/* See header file for description */
void
DownloadRequest::SetDataStream(DataStreamPtr dataStream)
{
    m_downloadData = dataStream;
}


// Protected functions ..........................................................................

/* See header file for description */
DownloadRequest::DownloadRequest(uint32 priority) : m_respnseError(NE_NOENT),
                                                    m_priority(priority),
                                                    m_currentRetriedTimes(0),
                                                    m_isInProgress(FALSE)
{
    // Nothing to do here.
}


// Private functions ............................................................................

/*! Find a substring in the source string but ignore the case of both strings

    @return The index of found substring (in characters from zero). Returns -1
            if the substring is not found.
*/
int
DownloadRequest::FindStringCaseInsensitive(const string& sourceString,  /*!< Source string */
                                           const string& stringToFind,  /*!< Substring to find */
                                           int start                    /*!< Starting position of source
                                                                             string to find */
                                           )
{
    const char* sourceChar = NULL;
    const char* foundChar = NULL;

    if (sourceString.empty() ||
        stringToFind.empty() ||
        (start < 0) ||
        (start >= (int) (sourceString.size())))
    {
        return -1;
    }

    // Find the substring in source string by a C function of PAL.
    sourceChar = sourceString.c_str();
    foundChar = nsl_stristr(sourceChar + start, stringToFind.c_str());
    if (!foundChar)
    {
        return -1;
    }

    return (int) (foundChar - sourceChar);
}

/*! Filter optional linear white space (LWS) of the string

    This function filters LWS from the 'start' position of the string. Return the
    position which is not a LWS.

    Reference the common forms from Hypertext Transfer Protocol:

    CR             = <carriage return>
    LF             = <linefeed>
    SP             = <space>
    HT             = <horizontal-tab>
    LWS            = [CRLF] 1*( SP | HT )

    @return The position which is not a LWS from 'start'. Return the length of entire
            string if there is no other substring besides LWS.
*/
int
DownloadRequest::FilterLinearWhiteSpace(const string& sourceString, /*!< String to filter */
                                        int start                   /*!< Starting position to filter */
                                        )
{
    int spaceLength = nsl_strlen(SPACE);
    int tabLength = nsl_strlen(TAB);
    int returnLinefeedLength = nsl_strlen(RETURN_LINEFEED);
    int entireLength = sourceString.size();

    while (start < entireLength)
    {
        if (sourceString.compare(start, spaceLength, SPACE) == 0)
        {
            start += spaceLength;
        }
        else if (sourceString.compare(start, tabLength, TAB) == 0)
        {
            start += tabLength;
        }
        else if (sourceString.compare(start, returnLinefeedLength, RETURN_LINEFEED) == 0)
        {
            start += returnLinefeedLength;
        }
        else
        {
            // Find the position which is not a LWS.
            return start;
        }
    }

    // There is no other substring besides LWS.
    return entireLength;
}

/* See description in header file. */
bool DownloadRequest::RetriedTimeIncreased()
{
    return (++m_currentRetriedTimes < m_maxRetryTimes) ? true : false;
}


void DownloadRequest::SetMaxRetryTime(uint32 retryTimes)
{
    m_maxRetryTimes = retryTimes;
}
/*! @} */
