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

    @file nbgmnbmcommon.h
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
#ifndef _NBGM_NBM_COMMON_H_
#define _NBGM_NBM_COMMON_H_
#include "nbretypes.h"
#include "nbrecommon.h"

/*! \addtogroup NBGM_Mapdata
*  @{
*/
/*! \addtogroup NBGM_NBMFile
*  @{
*/

struct float16
{
public:
    void Set(uint16 /*buf*/)
    {
    }
    float Get() const
    {
        return v;
    }
private:
    float v;
};

typedef struct cstr
{
    char*       s;
    uint32      l;
}cstr;

typedef struct float32_3
{
    float f0;
    float f1;
    float f2;
}float32_3;

typedef struct float32_2
{
    float f0;
    float f1;
}float32_2;

typedef enum NBGM_NBMValueType
{
    NBGM_DVT_UINT16,
    NBGM_DVT_UINT32,
    NBGM_DVT_INT16,
    NBGM_DVT_INT32,
    NBGM_DVT_FLOAT16,
    NBGM_DVT_FLOAT32,
    NBGM_DVT_STRING,
    NBGM_DVT_FLOAT32_2,
    NBGM_DVT_FLOAT32_3,
}NBGM_NBMValueType;

typedef union NBGM_NBMValue
{
    uint16              u16;
    uint32              u32;
    int16               i16;
    int32               i32;
    float16             f16;
    float               f32;
    cstr                str;
    float32_2           f32_2;
    float32_3           f32_3;
}NBGM_NBMValue;

struct NBGM_NBMAttributeValue
{
    uint16 GetU16() const
    {
        return v.u16;
    }
    uint32 GetU32() const
    {
        return v.u32;
    }
    int16 GetI16() const
    {
        return v.i16;
    }
    int32 GetI32() const
    {
        return v.i32;
    }
    float GetF32() const
    {
        return v.f32;
    }
    cstr GetStr() const
    {
        return v.str;
    }
    float32_2 GetF32_2() const
    {
        return v.f32_2;
    }
    float32_3 GetF32_3() const
    {
        return v.f32_3;
    }

    NBGM_NBMValue v;
};

struct NBGM_NBMAttribute;
struct NBGM_NBMGroup;
struct NBGM_NBMField;

typedef NBRE_Map<NBRE_String, NBGM_NBMAttribute*>       NBGM_NBMAttributeMap;
typedef NBRE_Vector<NBGM_NBMField*>                     NBGM_NBMFieldArray;
typedef NBRE_Map<NBRE_String, NBGM_NBMField*>           NBGM_NBMFieldMap;
typedef NBRE_Map<NBRE_String, NBGM_NBMGroup*>           NBGM_NBMFieldGroupMap;

struct NBGM_NBMAttribute
{
    NBRE_String                 mName;
    NBGM_NBMAttributeValue      mValue;
    NBRE_String                 mDescription;
};

struct NBGM_NBMGroup
{
    NBRE_String                 mName;
    NBGM_NBMFieldArray          mFieldArray;

    inline ~NBGM_NBMGroup();
};

struct NBGM_NBMField
{
    NBRE_String                 mName;
    NBGM_NBMAttributeMap        mAttributes;
    NBGM_NBMFieldMap            mSubFields;
    NBGM_NBMFieldGroupMap       mGroups;

    inline ~NBGM_NBMField();
};

#include "nbgmnbmcommon.inl"

/** @} */
/** @} */
#endif
