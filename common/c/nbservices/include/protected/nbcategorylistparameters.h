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

    @file     nbcategorylistparameters.h
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

#ifndef NBCATEGORYLISTPARAMETERS_H
#define NBCATEGORYLISTPARAMETERS_H


#include "paltypes.h"
#include "nbcontext.h"
#include "nbmobilecoupons.h"

/*!
    @addtogroup nbcategorylistparameters
    @{
*/


/*! @struct NB_CategoryListParameters
Opaque data structure that defines the parameters of category list
*/
typedef struct NB_CategoryListParameters NB_CategoryListParameters;


/*! Create  parameters of category list

@param context NAVBuilder context instance
@param position  Location for the search. Can be NULL. If not present, the server will return all available categories.
@param broadCategoryName A broad category
@param categoryName A category
@param parameters On success, a newly created NB_CategoryListParameters object; NULL otherwise.  A valid object must be destroyed using NB_CategoryListParametersDestroy()
@return NB_Error
*/
NB_DEC NB_Error NB_CategoryListParametersCreate( NB_Context* context,
                                                 const NB_CouponSearchRegion* position,
                                                 const char* broadCategoryName,
                                                 const char* categoryName,
                                                 NB_CategoryListParameters** parameters);


/*! Destroy a previously created NB_CategoryListParameters object

@param parameters A NB_CategoryListParameters object created with a call to one of the NB_CategoryListParametersCreate functions
@return NB_Error
*/
NB_DEC NB_Error NB_CategoryListParametersDestroy(NB_CategoryListParameters* parameters);


/*! @} */

#endif
