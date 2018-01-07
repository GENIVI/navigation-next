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

    @file     nbsearchfilter.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, Inc. is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef NBSEARCHFILTER_H
#define NBSEARCHFILTER_H

#include "nbexp.h"
#include "nbcontext.h"
#include "paltypes.h"
#include "nberror.h"

/*!
    @addtogroup nbsearchfilter
    @{
*/


/*! @struct NB_SearchFilter
*/
typedef struct NB_SearchFilter NB_SearchFilter;

/*! Create  NB_SearchFilter object
@param context NAVBuilder Context Instance
@param filter On success, a newly created NB_SearchFilter object; NULL otherwise.  A valid object must be destroyed using NB_SearchFilterDestroy()

@return NB_Error
*/
NB_DEC NB_Error NB_SearchFilterCreate( NB_Context* context, NB_SearchFilter** filter );

/*! Add new rule into search filter

@param filter A NB_SearchFilter object
@param key A rule
@param value A value

@return NB_Error
*/
NB_DEC NB_Error NB_SearchFilterAdd( NB_SearchFilter* filter, const char* key, const char* value );


/*! This function removes rule from filer

@param filter A NB_SearchFilter object
@param key A rule

@return NB_Error
*/
NB_DEC NB_Error NB_SearchFilterDelete( NB_SearchFilter* filter, const char* key );


/*! Destroy a previously created NB_SearchFilter object

@param filter A NB_SearchFilter object created with NB_SearchFilterCreate()
@return NB_Error
*/
NB_DEC NB_Error NB_SearchFilterDestroy(NB_SearchFilter* filter);


/*! @} */

#endif
