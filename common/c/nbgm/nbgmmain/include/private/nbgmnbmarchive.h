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

    @file nbgmnbmarchive.h
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
#ifndef _NBGM_NBM_ARCHIVE_H_
#define _NBGM_NBM_ARCHIVE_H_
#include "nbretypes.h"
#include "nbrecommon.h"
#include "nbreiostream.h"

struct NBGM_NBMChunkInfo;
struct NBGM_NBMConfig;
struct NBGM_NBMField;
struct NBGM_NBMAttributeValue;
struct NBGM_NBMAttribute;
struct NBGM_NBMGroup;

/*! \addtogroup NBRE_Mapdata
*  @{
*/
/*! \addtogroup NBGM_NBMFile
*  @{
*/
class NBGM_NBMArchive
{
public:
    NBGM_NBMArchive(NBRE_IOStream& ios, nb_boolean storing);
    ~NBGM_NBMArchive();

public:
    const NBGM_NBMArchive& operator>>(NBGM_NBMField& field) const;

    nb_boolean Storing() {return mStoring;}
    void SetCurrentChunkConfig(const NBGM_NBMConfig& config);

    nb_boolean Eof() const;
    uint32 GetPos() const;
    void Seek(PAL_FileSetOrigin type, int32 offset);

private:
    const NBGM_NBMArchive& operator>>(NBGM_NBMAttributeValue& val) const;
    const NBGM_NBMArchive& operator>>(NBGM_NBMAttribute& attr) const;
    const NBGM_NBMArchive& operator>>(NBGM_NBMGroup& group) const;

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_NBMArchive);

private:
    NBRE_IOStream&                          mStream;
    mutable const NBGM_NBMConfig*      mCurrentConfig;
    nb_boolean                              mStoring;
};

/** @} */
/** @} */
#endif
