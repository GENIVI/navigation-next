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

    @file       CachingEntry.h

    A CachingEntry object saves the information of a caching record. It
    contains a caching index and additional data. A caching index
    contains a type and a name. The type expresses which type the
    caching record belongs to. The name is the filename of the caching
    record in the persistent cache. And user could get a value of
    additional data by a key.
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

#ifndef CACHINGENTRY_H
#define CACHINGENTRY_H

/*!
    @addtogroup nbcommon
    @{
*/

#include "base.h"
#include "CachingIndex.h"
#include <map>

namespace nbcommon
{

// Constants ....................................................................................


// Types ........................................................................................

class CachingIndex;

/*! Caching entry for saving the information of a caching record */
class CachingEntry : public Base
{
public:
    // Public functions .........................................................................

    /*! CachingEntry constructor */
    CachingEntry(shared_ptr<std::string> type,  /*!< Type of the caching entry */
                 shared_ptr<std::string> name,  /*!< Name of the caching entry */
                 shared_ptr<std::map<std::string, shared_ptr<std::string> > > additionalData    /*!< Additional data of key and value */
                 );

    /*! CachingEntry destructor */
    virtual ~CachingEntry();

    /*! Get the type

        @return Type of the caching entry
    */
    shared_ptr<std::string>
    GetType();

    /*! Get the name

        @return Name of the caching entry
    */
    shared_ptr<std::string>
    GetName();

    /*! Get the additional data of key and value

        @return Additional data of key and value
    */
    shared_ptr<std::map<std::string, shared_ptr<std::string> > >
    GetAdditionalData();

    /*! Set the additional data of key and value

        @return None
    */
    void
    SetAdditionalData(shared_ptr<std::map<std::string, shared_ptr<std::string> > > additionalData   /*!< Additional data of key and value to set */
                      );

    /*! Get the value of the additional data by a key

        @return A value of the additional data
    */
    shared_ptr<std::string>
    GetAdditionalValue(const std::string& key   /*!< A key to get the value of the additional data */
                       );

    /*! Set the value of the additional data by a key

        @return None
    */
    void
    SetAdditionalValue(const std::string& key,      /*!< A key to set the value of the additional data */
                       shared_ptr<std::string> value/*!< A value of the additional data to set */
                       );

    /*! Get the protected flag

        @return Protected flag
    */
    bool
    GetProtected() const;

    /*! Set the protected flag

        @return None
    */
    void
    SetProtected(bool isProtected);


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    CachingEntry(const CachingEntry& entry);
    CachingEntry& operator=(const CachingEntry& entry);


    // Private members ..........................................................................

    bool m_isProtected;             /*!< Is the caching entry protected? */
    shared_ptr<std::string> m_type; /*!< Type of the caching entry */
    shared_ptr<std::string> m_name; /*!< Name of the caching entry */
    shared_ptr<std::map<std::string, shared_ptr<std::string> > > m_additionalData;  /*!< Additional data of key and value */
};

typedef shared_ptr<CachingEntry> CachingEntryPtr;

};  // namespace nbcommon

/*! @} */

#endif  // CACHINGENTRY_H
