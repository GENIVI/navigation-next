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

    @file       CachingIndex.h

    A CachingIndex object saves the type and name of a caching record.
    The type expresses which type the caching record belongs to. The
    name is the filename of the caching record in the persistent cache.
    And it can be used as a key for std::map.
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

#ifndef CACHINGINDEX_H
#define CACHINGINDEX_H

/*!
    @addtogroup nbcommon
    @{
*/

#include "base.h"
#include "smartpointer.h"
#include <string>

namespace nbcommon
{

// Constants ....................................................................................


// Types ........................................................................................

/*! Caching index for saving the type and name of a caching record */
class CachingIndex : public Base
{
public:
    // Public functions .........................................................................

    /*! CachingIndex default constructor */
    CachingIndex();

    /*! CachingIndex constructor */
    CachingIndex(shared_ptr<std::string> type,  /*!< Type of the caching record */
                 shared_ptr<std::string> name   /*!< Name of the caching record */
                 );

    /*! CachingIndex destructor */
    virtual ~CachingIndex();

    /*! CachingIndex copy constructor */
    CachingIndex(const CachingIndex& rightSide);

    /*! CachingIndex assignment operator */
    CachingIndex& operator=(const CachingIndex& rightSide);

    /*! Compare operator */
    bool
    operator==(const CachingIndex& rightSide) const;

    /*! The less-operator is used so that we can put a CachingIndex into a std::set or std::map */
    bool
    operator<(const CachingIndex& rightSide) const;


    // Public members ...........................................................................

    shared_ptr<std::string> m_type; /*!< Type of the caching record */
    shared_ptr<std::string> m_name; /*!< Name of the caching record */
};

typedef shared_ptr<CachingIndex> CachingIndexPtr;

};  // namespace nbcommon

/*! @} */

#endif  // CACHINGINDEX_H
