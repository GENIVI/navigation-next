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

    @file       CachingIndex.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "CachingIndex.h"
#include "StringUtility.h"

/*! @{ */

using namespace std;
using namespace nbcommon;

// Local Constants ..............................................................................


// Public functions .............................................................................

/* See header file for description */
CachingIndex::CachingIndex() : m_type(),
                               m_name()
{
    // Nothing to do here.
}

/* See header file for description */
CachingIndex::CachingIndex(shared_ptr<string> type,
                           shared_ptr<string> name) : m_type(type),
                                                      m_name(name)
{
    // Nothing to do here.
}

/* See header file for description */
CachingIndex::~CachingIndex()
{
    // Nothing to do here.
}

/* See header file for description */
CachingIndex::CachingIndex(const CachingIndex& rightSide) : m_type(rightSide.m_type),
                                                            m_name(rightSide.m_name)
{
    // Nothing to do here.
}

/* See header file for description */
CachingIndex&
CachingIndex::operator=(const CachingIndex& rightSide)
{
    if (this == (&rightSide))
    {
        return *this;
    }

    m_type = rightSide.m_type;
    m_name = rightSide.m_name;

    return *this;
}

/* See header file for description */
bool
CachingIndex::operator==(const CachingIndex& rightSide) const
{
    if (this == (&rightSide))
    {
        return true;
    }

    return (StringUtility::IsStringEqual(m_type, rightSide.m_type) &&
            StringUtility::IsStringEqual(m_name, rightSide.m_name));
}

/* See header file for description */
bool
CachingIndex::operator<(const CachingIndex& rightSide) const
{
    if (this == (&rightSide))
    {
        return true;
    }

    // Check the names first here. Because the names maybe different.
    if (StringUtility::IsStringLess(m_name, rightSide.m_name))
    {
        return true;
    }

    if (!StringUtility::IsStringEqual(m_name, rightSide.m_name))
    {
        return false;
    }

    return StringUtility::IsStringLess(m_type, rightSide.m_type);
}

/*! @} */
