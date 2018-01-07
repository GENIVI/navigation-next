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

    @file       UnifiedTileType.h

    An UnifiedTileType object is used to specify the URL template and
    content ID template.
*/
/*
    (C) Copyright 2015 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef UNIFIEDTILETYPE_H
#define UNIFIEDTILETYPE_H

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
}

#include <string>
#include "smartpointer.h"
#include "base.h"
#include "TileType.h"

namespace nbmap
{

// Constants ....................................................................................


// Types ........................................................................................

/*! An UnifiedTileType object is used to specify the URL template and content ID template */
class UnifiedTileType : public TileType, public Base
{
public:
    // Public functions .........................................................................

    /*! UnifiedTileType constructor */
    UnifiedTileType(shared_ptr<std::string> dataType,          /*!< Data type of the tile */
                    shared_ptr<std::string> urlTemplate,       /*!< URL template */
                    shared_ptr<std::string> contentIdTemplate  /*!< Content ID template */
                    );

    /*! UnifiedTileType destructor */
    virtual ~UnifiedTileType(void);

    /* See description in TileType.h */
    shared_ptr<std::string> GetDataType(void);
    shared_ptr<std::string> GetUrlTemplate(void);
    shared_ptr<std::string> GetContentIDTemplate(void);

private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    UnifiedTileType(const UnifiedTileType& tileType);
    UnifiedTileType& operator=(const UnifiedTileType& tileType);

    // Private members ..........................................................................

    shared_ptr<std::string> m_dataType;         /*!< Data type of the tile */
    shared_ptr<std::string> m_urlTemplate;      /*!< URL template to download */
    shared_ptr<std::string> m_contentIdTemplate;/*!< Content ID template to cache */
};

typedef shared_ptr<UnifiedTileType> UnifiedTileTypePtr;

}  // namespace nbmap

/*! @} */

#endif  // UNIFIEDTILETYPE_H
