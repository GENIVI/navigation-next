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
    @file specialregioninformationimpl.h
    @date 10/08/2014
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
---------------------------------------------------------------------------*/

/*! @{ */
#ifndef LOCATIONTOOLKIT_SPECIALREGION_INFORMATION_IMPL_H
#define LOCATIONTOOLKIT_SPECIALREGION_INFORMATION_IMPL_H

#include <QImage>
#include "data/specialregioninformation.h"
#include "NavApiSpecialRegionInformation.h"

namespace locationtoolkit
{
class SpecialRegionInformationImpl: public SpecialRegionInformation
{
public:
    SpecialRegionInformationImpl(const nbnav::SpecialRegionInformation& specialRegionInformation);

    SpecialRegionInformationImpl();

    virtual ~SpecialRegionInformationImpl();

    virtual const QString& GetType() const;

    virtual const QString& GetDescription() const;

    virtual const QImage& GetImage() const ;

    virtual qint32 GetStartManeuverIndex() const;

    virtual qreal GetStartManeuverOffset() const;

    virtual qint32 GetEndManeuverIndex() const;

    virtual qreal GetEndManuverOffset() const;

    virtual qint32 GetNextOccurStartTime() const;

    virtual qint32 GetNextOccurEndTime() const;

private:
    QString mRegionType;

    QString mRegionDescription;

    QImage mImage;

    qint32  mStartManeuverIndex;

    qreal   mStartManeuverOffset;

    qint32  mEndManeuverIndex;

    qreal   mEndManeuverOffset;

    qint32  mNextOccurStartTime;

    qint32  mNextOccurEndTime;

};
} // namespace locationtoolkit
#endif
