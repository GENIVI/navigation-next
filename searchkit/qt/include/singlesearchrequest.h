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

/*
 * (C) Copyright 2015 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

/*! @{ */

#ifndef SINGLESEARCHREQUEST_H
#define SINGLESEARCHREQUEST_H

#include "searchrequest.h"
#include "coordinate.h"
#include "ltkcontext.h"

namespace locationtoolkit
{

/**
 * Single Search Request.
 */
class SingleSearchRequest
{
public:
    virtual ~SingleSearchRequest() {}

public:
    /**
     * Create one Single Search Request for POI.
     *
     * @param ltkContext LTKContext
     * @param keyword The word to be searched as a key.
     * @param current Current coordinate.
     * @param search Target coordinate for search.
     * @param onboardOnly Search onboard database only, do not go over network.
     */
    static SearchRequest* createRequest(LTKContext& ltkContext,
                                        const QString& keyword,
                                        const Coordinates& current,
                                        const Coordinates& search,
                                        bool onboardOnly = false,
                                        const QString& lang = "en-US");

    /**
     * Create one Single Search Request for POI.
     *
     * @param ltkContext LTKContext
     * @param suggestion The suggestion for search.
     * @param current Current coordinate.
     * @param search Target coordinate for search.
     */
    static SearchRequest* createRequest(LTKContext& ltkContext,
                                        const SearchSuggestion& suggestion,
                                        const Coordinates& current,
                                        const Coordinates& search,
                                        const QString& lang = "en-US");

    /**
     * Create one Single Search Request for Static POI.
     *
     * @param ltkContext LTKContext
     * @param id Static POI id
     * @param name Static POI name
     * @param current Current coordinate.
     * @param search Target coordinate for search.
     */
    static SearchRequest* createStaticRequest(LTKContext& ltkContext,
                                              const QString& id,
                                              const QString& name,
                                              const Coordinates& current,
                                              const Coordinates& search,
                                              const QString& lang = "en-US");
    /**
     * Create one Single Search Request for POI.
     *
     * @param ltkContext LTKContext
     * @param categoryCode The category code for search.
     * @param current Current coordinate.
     * @param search Target coordinate for search.
     */
    static SearchRequest* createRequestWithCategory(LTKContext& ltkContext,
                                                    const QString& categoryCode,
                                                    const Coordinates& current,
                                                    const Coordinates& search,
                                                    const QString& lang = "en-US");

    /**
     * Create one Single Search Request for POI.
     *
     * @param ltkContext LTKContext
     * @param location The location for search.
     */
    static SearchRequest* createRequest(LTKContext& ltkContext,
                                        const SearchLocation& location,
                                        const QString& lang = "en-US");

    /**
     * Create one Single Search Request for POI.
     *
     * @param ltkContext LTKContext
     * @param request The based request.
     * @param information The search result of based request.
     * @param direction Start, previous or next.
     */
    static SearchRequest* createRequest(LTKContext& ltkContext, SearchRequest* request,
                                        const SearchInformation& information, const SearchDirection& direction);

};

}

#endif // SINGLESEARCHREQUEST_H

/*! @} */
