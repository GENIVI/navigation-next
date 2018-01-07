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

    @file       CachingEntryProcessor.h

    Class CachingEntryProcessor is an interface. It is used to get and
    save the CachingEntry objects.
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

#ifndef CACHINGENTRYPROCESSOR_H
#define CACHINGENTRYPROCESSOR_H

/*!
    @addtogroup nbcommon
    @{
*/

extern "C"
{
}

#include "CachingEntry.h"
#include <vector>

namespace nbcommon
{

// Types ........................................................................................

/*! Caching entry processor interface */
class CachingEntryProcessor
{
public:
    // Interface Functions ......................................................................

    /*! CachingEntryProcessor destructor */
    virtual ~CachingEntryProcessor()
    {
        // Nothing to do here.
    }

    /*! Get a caching entry

        This function always returns NULL if there is no this caching entry specified by the
        type and name.

        @return A shared pointer to a CachingEntry object
    */
    virtual CachingEntryPtr
    GetEntry(shared_ptr<std::string> type,  /*!< Type of the caching entry to get */
             shared_ptr<std::string> name   /*!< Name of the caching entry to get */
             ) = 0;

    /*! Get all caching entries

        @return None
    */
    virtual void
    GetAllEntries(std::vector<CachingEntryPtr>& entries /*!< Return all caching entries */
                  ) = 0;

    /*! Get the count of all caching entries

        @return Count of all caching entries
    */
    virtual uint32
    GetEntryCount() = 0;

    /*! Add or update a caching entry

        If the caching entry specified by the type and name exists, the found caching
        entry is updated. Otherwise the caching entry is added.

        @return NE_OK if success
    */
    virtual NB_Error
    SetEntry(CachingEntryPtr entry      /*!< A caching entry to add or update */
             ) = 0;

    /*! Remove the caching entries

        Nothing to do if there is no caching entry specified by the type and name. If the
        parameter 'name' is NULL, all caching entries of the specified type are removed.

        @return None
    */
    virtual void
    RemoveEntry(shared_ptr<std::string> type,   /*!< Type of the caching entry to remove */
                shared_ptr<std::string> name    /*!< Name of the caching entry to remove. If it is
                                                     NULL, all caching entries of the specified type
                                                     are removed. */
                ) = 0;

    /*! Check if the caching entry exists

        @return true if the caching entry exists, false otherwise.
    */
    virtual bool
    IsEntryExisting(shared_ptr<std::string> type,   /*!< Type to check if the caching entry exists */
                    shared_ptr<std::string> name    /*!< Name to check if the caching entry exists */
                    ) = 0;

    /*! Set the protected flag to the caching entry

        If the protected flag is true, the caching entry cannot be removed when the cache is full.

        @return None
    */
    virtual void
    SetEntryProtected(shared_ptr<std::string> type, /*!< Type to set the protected flag */
                      shared_ptr<std::string> name, /*!< Name to set the protected flag */
                      bool entryProtected           /*!< Protected flag to set */
                      ) = 0;
};

typedef shared_ptr<CachingEntryProcessor> CachingEntryProcessorPtr;

};  // namespace nbcommon

/*! @} */

#endif  // CACHINGENTRYPROCESSOR_H
