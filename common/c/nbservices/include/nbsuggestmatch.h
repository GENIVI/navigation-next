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

    @file   nbsuggestmatch.h
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

#ifndef NBSUGGESTMATCH_H
#define NBSUGGESTMATCH_H

/*!
    @addtogroup nbsuggestmatch
    @{
*/

#include "nbcontext.h"

// Constants ....................................................................................
#define ICON_ID_MAX_COUNT 8


// Types ........................................................................................

/*! Suggest match returned from suggestion search */
typedef struct NB_SuggestMatch NB_SuggestMatch;

/*!< Match type for the result of suggestion search */
typedef enum
{
    NB_MT_None = 0,                         /*!< No match type */
    NB_MT_POI,                              /*!< POI match type */
    NB_MT_Address,                          /*!< Address match type */
    NB_MT_Airport,                          /*!< Airport match type */
    NB_MT_Gas,                              /*!< Gas match type */
    NB_MT_Category                          /*!< Category match type */
} NB_MatchType;

/*! Data of suggest match to display */
typedef struct
{
    const char* line1;                      /*!< String contents of line 1 */
    const char* line2;                      /*!< String contents of line 2 */
    const char* line3;                      /*!< String contents of line 3 */
    double distance;                        /*!< Distance of suggestion */
    NB_MatchType matchType;                 /*!< Identifies the type of result indicated by the suggestion.
                                                 Used by the client for showing the appropriate icons. */
    const char* iconIDs[ICON_ID_MAX_COUNT];
    int countIconID;
} NB_SuggestMatchData;


// Functions ....................................................................................

/*! Create an instance of 'NB_SuggestMatch' by a data buffer

    This function is used for creating a 'NB_SuggestMatch' object by a data buffer. This data
    buffer should be serialized by the function 'NB_SuggestMatchSerialize'.

    @return NE_OK if success
    @see NB_SuggestMatchDestroy
    @see NB_SuggestMatchSerialize
*/
NB_DEC NB_Error
NB_SuggestMatchCreateBySerializedData(
    NB_Context* context,                    /*!< Pointer to current context */
    const uint8* data,                      /*!< A data buffer containing the results of a previous
                                                 'NB_SuggestMatchSerialize' call */
    nb_size dataSize,                       /*!< The size of the data buffer */
    NB_SuggestMatch** suggestMatch          /*!< On success, returns pointer to 'NB_SuggestMatch' object */
    );

/*! Create an instance of 'NB_SuggestMatch' by copying from another 'NB_SuggestMatch' object

    @return NE_OK if success
    @see NB_SuggestMatchDestroy
*/
NB_DEC NB_Error
NB_SuggestMatchCreateByCopy(
    NB_SuggestMatch* sourceSuggestMatch,    /*!< Source 'NB_SuggestMatch' object to copy */
    NB_SuggestMatch** suggestMatch          /*!< On success, returns pointer to 'NB_SuggestMatch' object */
    );

/*! Destroy an instance of a 'NB_SuggestMatch' object

    @return None
*/
NB_DEC void
NB_SuggestMatchDestroy(
    NB_SuggestMatch* suggestMatch           /*!< 'NB_SuggestMatch' instance */
    );

/*! Get the data to display

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SuggestMatchGetDataToDisplay(
    NB_SuggestMatch* suggestMatch,          /*!< 'NB_SuggestMatch' instance */
    NB_SuggestMatchData* dataToDisplay      /*!< On success, this 'NB_SuggestMatchData' object is filled */
    );

/*! Serialize the 'NB_SuggestMatch' object

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SuggestMatchSerialize(
    NB_SuggestMatch* suggestMatch,          /*!< 'NB_SuggestMatch' instance */
    uint8** data,                           /*!< On success, receives a pointer to a data buffer. The
                                                 pointer must be freed with nsl_free */
    nb_size* dataSize                       /*!< On success, receives the size of the data buffer */
    );

/*! Serialize the 'NB_SuggestMatch' object by a simple way.

    @return NE_OK if success
*/
NB_DEC NB_Error
NB_SuggestMatchSerializeLite(
    NB_SuggestMatch* suggestMatch,          /*!< 'NB_SuggestMatch' instance */
    uint8** data,                           /*!< On success, receives a pointer to a data buffer. The
                                                 pointer must be freed with nsl_free */
    nb_size* dataSize                       /*!< On success, receives the size of the data buffer */
    );

/*! Check if two 'NB_SuggestMatch' objects are equal

    @return TRUE if two 'NB_SuggestMatch' objects are equal, FALSE otherwise.
*/
NB_DEC nb_boolean
NB_SuggestMatchIsEqual(
    NB_SuggestMatch* suggestMatch,          /*!< 'NB_SuggestMatch' instance */
    NB_SuggestMatch* anotherSuggestMatch    /*!< Another 'NB_SuggestMatch' object to compare */
    );

/*! @} */

#endif
