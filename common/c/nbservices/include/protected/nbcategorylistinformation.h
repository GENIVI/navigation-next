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

    @file     nbcategorylistinformation.h
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

#ifndef NBCATEGORYLISTINFORMATION_H
#define NBCATEGORYLISTINFORMATION_H

#include "nbexp.h"
#include "paltypes.h"
#include "nberror.h"

/*!
    @addtogroup nbcategorylistinformation
    @{
*/


/*! @struct NB_CategoryListInformation
Information about the results of image load
*/
typedef struct NB_CategoryListInformation NB_CategoryListInformation;


/*! Get specified string from broad category list

@param information Address of a valid NB_LocationInformation object
@param index Result index
@param broadCategory Provides a broad category. DO NOT free it, it's just a pointer that will be deallocated in NB_CategoryListInformationDestroy. broadCategory should point to NULL
@returns NB_Error
*/
NB_DEC NB_Error NB_CategoryListInformationGetBroadCategoryItem(
    NB_CategoryListInformation* information,
    int32 index,
    char const** broadCategory );

/*! Get number of broad categories in the server response

@param information Address of a valid NB_LocationInformation object
@param count Number of results in the server response
@return NB_Error
*/
NB_DEC NB_Error NB_CategoryListInformationGetBroadCategoryCount(
    NB_CategoryListInformation* information, 
    int32* count);

/*! Get specified string from category list

@param information Address of a valid NB_LocationInformation object
@param index Result index
@param category Provides a category. DO NOT free it, it's just a pointer that will be deallocated in NB_CategoryListInformationDestroy. category should point to NULL
@returns NB_Error
*/
NB_DEC NB_Error NB_CategoryListInformationGetCategoryItem(
    NB_CategoryListInformation* information,
    int32 index,
    char const** category );

/*! Get number of categories in the server response

@param information Address of a valid NB_LocationInformation object
@param count Number of results in the server response
@return NB_Error
*/
NB_DEC NB_Error NB_CategoryListInformationGetCategoryCount(
    NB_CategoryListInformation* information, 
    int32* count);

/*! Get specified string from sub category list

@param information Address of a valid NB_LocationInformation object
@param index Result index
@param subCategory Provides the broad category. DO NOT free it, it's just a pointer that will be deallocated in NB_CategoryListInformationDestroy. subCategory should point to NULL
@returns NB_Error
*/
NB_DEC NB_Error NB_CategoryListInformationGetSubCategoryItem(
    NB_CategoryListInformation* information,
    int32 index,
    char const** subCategory );

/*! Get number of sub-categories in the server response

@param information Address of a valid NB_LocationInformation object
@param count Number of results in the server response
@return NB_Error
*/
NB_DEC NB_Error NB_CategoryListInformationGetSubCategoryCount(
    NB_CategoryListInformation* information, 
    int32* count);

/*! Destroy a previously created NB_CategoryListInformation object

@param information A NB_CategoryListHandler object created with NB_CategoryListHandlerGetInformation()
@return NB_Error
*/
NB_DEC NB_Error NB_CategoryListInformationDestroy(NB_CategoryListInformation* information);


/*! @} */

#endif
