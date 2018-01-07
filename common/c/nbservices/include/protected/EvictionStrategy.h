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

    @file       EvictionStrategy.h

    Class EvictionStrategy is an interface. It monitors the operations
    such as adding, removing and using. And user could get the first
    element needed to remove by the function GetFirstToRemove.
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

#ifndef EVICTIONSTRATEGY_H
#define EVICTIONSTRATEGY_H

/*!
    @addtogroup nbcommon
    @{
*/

namespace nbcommon
{

// Constants ....................................................................................


// Types ........................................................................................

/*! Index operations */
typedef enum
{
    ADDING_INDEX,   /*!< Adding index */
    REMOVING_INDEX, /*!< Removing index */
    USING_INDEX     /*!< Using index */
} IndexOperation;

/*! Eviction strategy for monitoring operations and supporting a eviction function */
template <class T>
class EvictionStrategy
{
public:
    // Interface Functions ......................................................................

    /*! EvictionStrategy destructor */
    virtual ~EvictionStrategy()
    {
        // Nothing to do here.
    }

    /*! Notify an index is operated

        @return None
    */
    virtual void
    IndexOperated(T index,                  /*!< A operated index */
                  IndexOperation operation  /*!< An operation */
                  ) = 0;

    /*! Get the first index to remove

        @return Index to remove
    */
    virtual T
    GetFirstToRemove() = 0;

    /*! Clear all indexes

        @return None
    */
    virtual void
    Clear() = 0;
};

};  // namespace nbcommon

/*! @} */

#endif  // EVICTIONSTRATEGY_H
