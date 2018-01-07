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
    @file routeinformationimp.h
    @date 10/15/2014
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
#ifndef LOCATIONTOOLKIT_MANEUVER_IMP_H
#define LOCATIONTOOLKIT_MANEUVER_IMP_H

#include <QtGlobal>
#include <QString>
#include <QVector>

#include "NavApiTypes.h"
#include "coordinate.h"
#include "data/maneuver.h"

namespace locationtoolkit
{
class ManeuverImpl: public Maneuver
{
public:
    ManeuverImpl(const nbnav::Maneuver& maneuver);

    virtual ~ManeuverImpl();

    virtual qint32 GetManeuverID() const;

    virtual const QVector<Coordinates*>& GetPolyline() const;

    virtual const QString& GetRoutingTTF() const;

    virtual qreal GetDistance() const;

    virtual const QString& GetPrimaryStreet() const;

    virtual const QString& GetSecondaryStreet() const;

    virtual qreal GetTime() const;

    virtual const QString& GetCommand() const;

    virtual const Coordinates& GetPoint() const;

    //virtual FormattedTextBlock GetManeuverText(QBool isMetric) const;

    virtual const QString& GetDescription(bool isMetric) const;

    virtual qreal GetTrafficDelay() const;

    virtual bool GetStackAdvise() const;

    virtual const QString& GetExitNumber() const;

    virtual bool IsDestination() const;
private:
    qint32 mManeuverId;
    QVector<Coordinates*> mPolyline;
    QString mRouteTTF;
    qreal   mDistance;
    QString mCommand;
    QString mPrimaryStreet;
    QString mSecondaryStreet;
    qreal   mTime;
    Coordinates mPoint;
    //FormattedTextBlock mMiManeuverText;
    //FormattedTextBlock mKmManeuverText;
    QString mMiDescription;
    QString mKmDescription;
    qreal mTrafficDelay;
    bool mStackAdvise;
    QString mExitNumber;
    bool mIsDestination;
}; // namespace locationtoolkit
}
#endif
