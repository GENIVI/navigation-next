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

    @file nbgmnbmconfig.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBGM_NBM_CONFIG_H_
#define _NBGM_NBM_CONFIG_H_
#include "nbretypes.h"
#include "nbgmnbmcommon.h"
#include "nbrecommon.h"

/*! \addtogroup NBRE_Mapdata
*  @{
*/
/*! \addtogroup NBGM_NBMFile
*  @{
*/
enum NBGM_NBMConfigType
{
    NBGM_NBM_CT_ATTRIBUTE,
    NBGM_NBM_CT_FIELD,
    NBGM_NBM_CT_GROUP
};

struct NBGM_NBMConfig;

struct NBGM_NBMAttributeConfig
{
    NBGM_NBMValueType   valueType;
    NBRE_String*        description;

    NBGM_NBMAttributeConfig();
    ~NBGM_NBMAttributeConfig();
};

struct NBGM_NBMFieldConfig
{
    void AppendConfig(NBGM_NBMConfig* config);

    NBRE_Vector<NBGM_NBMConfig*>*  subConfig;

    NBGM_NBMFieldConfig();
    ~NBGM_NBMFieldConfig();

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_NBMFieldConfig);
};

struct NBGM_NBMGroupConfig
{
    NBGM_NBMValueType       countValueType;
    NBGM_NBMFieldConfig     fieldConfig;

    NBGM_NBMGroupConfig();
    ~NBGM_NBMGroupConfig();
};


struct NBGM_NBMConfig
{
    NBRE_String                     mName;
    NBGM_NBMConfigType              mType;
    union
    {
        NBGM_NBMFieldConfig*        mFieldConfig;
        NBGM_NBMGroupConfig*        mGroupConfig;
        NBGM_NBMAttributeConfig*    mAttributeConfig;
    };

    NBGM_NBMConfig();
    ~NBGM_NBMConfig();
private:
    DISABLE_COPY_AND_ASSIGN(NBGM_NBMConfig);
};

/** @} */
/** @} */
#endif
