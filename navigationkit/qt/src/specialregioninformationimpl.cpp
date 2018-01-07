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

 @file specialregioninformationimpl.cpp
 @date 09/22/2014
 @addtogroup navigationkit

 */
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
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

#include "NavApiSpecialRegionInformation.h"
#include "internal/specialregioninformationimpl.h"

namespace locationtoolkit
{
SpecialRegionInformationImpl::SpecialRegionInformationImpl(const nbnav::SpecialRegionInformation& specialRegionInformation)
{
    mRegionType = QString::fromStdString(specialRegionInformation.GetType());
    mRegionDescription = QString::fromStdString(specialRegionInformation.GetDescription());
    mStartManeuverIndex = qint32(specialRegionInformation.GetStartManeuverIndex());
    mStartManeuverOffset = qreal(specialRegionInformation.GetStartManeuverOffset());
    mEndManeuverIndex = qint32(specialRegionInformation.GetEndManeuverIndex());
    mEndManeuverOffset = qreal(specialRegionInformation.GetEndManuverOffset());
    mNextOccurStartTime = qint32(specialRegionInformation.GetNextOccurStartTime());
    mNextOccurEndTime =  qint32(specialRegionInformation.GetNextOccurEndTime());
    const std::vector<unsigned char>& data = specialRegionInformation.GetImage().GetData();
    mImage.loadFromData(&data[0], data.size());
}

SpecialRegionInformationImpl::SpecialRegionInformationImpl()
{
}

SpecialRegionInformationImpl::~SpecialRegionInformationImpl()
{
}

const QString& SpecialRegionInformationImpl::GetType() const
{
    return mRegionType;
}

const QString& SpecialRegionInformationImpl::GetDescription() const
{
    return mRegionDescription;
}

const QImage& SpecialRegionInformationImpl::GetImage() const
{
    return mImage;
}

qint32 SpecialRegionInformationImpl::GetStartManeuverIndex() const
{
    return mStartManeuverIndex;
}

qreal SpecialRegionInformationImpl::GetStartManeuverOffset() const
{
    return mStartManeuverOffset;
}

qint32 SpecialRegionInformationImpl::GetEndManeuverIndex() const
{
    return mEndManeuverIndex;
}

qreal SpecialRegionInformationImpl::GetEndManuverOffset() const
{
    return mEndManeuverOffset;
}

qint32 SpecialRegionInformationImpl::GetNextOccurStartTime() const
{
    return mNextOccurStartTime;
}

qint32 SpecialRegionInformationImpl::GetNextOccurEndTime() const
{
    return mNextOccurEndTime;
}
}
