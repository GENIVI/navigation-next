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

    @file     nbenchancedcontentmanagerprivate.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef NBENHANCEDCONTENTMANAGERPRIVATE_H
#define NBENHANCEDCONTENTMANAGERPRIVATE_H

/*!
    @addtogroup nbenhancedcontentmanagerprivate
    @{
*/

#include "nbenhancedcontentmanagerprotected.h"

// Private Constants ..............................................................................


// Private Types ..................................................................................

/*! Enhanced content data identifier.  Used to refer to enhanced data items (or entire datasets) 

    Use create/destroy functions below to create and destroy.

    @see NB_EnhancedContentIdentifierCreate
    @see NB_EnhancedContentIdentifierDestroy
*/
typedef struct
{
    NB_EnhancedContentDataType type;    /*!< Data type */
    char* dataset;                      /*!< Pointer to dataset string */
    char* itemId;                       /*!< Data item ID. If NULL, refers to entire dataset content */
    char* fileVersion;                  /*!< File version of the file to be downloaded. */
} NB_EnhancedContentIdentifier;


// Private Functions ..............................................................................

/*! Create an enhanced content identifier.

    Call NB_EnhancedContentIdentifierDestroy() to destroy the object.

    @return NB_EnhancedContentIdentifier on success, NULL on failure.
    @see NB_EnhancedContentIdentifierDestroy
*/
NB_EnhancedContentIdentifier*
NB_EnhancedContentIdentifierCreate(NB_EnhancedContentDataType type, /*!< see NB_EnhancedContentIdentifier for description */
                                   const char* dataset,             /*!< dataset of this identifier */
                                   const char* itemId,              /*!< item id of this identifier */
                                   const char* fileVersion          /*!< file version of this content */
                                   );

/*! Destroy an enhanced content identifier.

    @return None
    @see NB_EnhancedContentIdentifierCreate
*/
void
NB_EnhancedContentIdentifierDestroy(NB_EnhancedContentIdentifier* identifier    /*!< Identifier to destroy */
                                    );

/*! Set an enhanced content identifier.

@return NE_OK on success
*/
NB_Error
NB_EnhancedContentIdentifierSet(NB_EnhancedContentIdentifier* identifier,       /*!< Enhanced content identifier instance */
                                NB_EnhancedContentDataType type,                /*!< see NB_EnhancedContentIdentifier for description */
                                const char* dataset,
                                const char* itemId,
                                const char* fileVersion
                                );

/*! @} */

#endif
