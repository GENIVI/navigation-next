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
    @file maneuver.h
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

/*! @{ */
#ifndef LOCATIONTOOLKIT_MANEUVER_H
#define LOCATIONTOOLKIT_MANEUVER_H

#include <QtGlobal>
#include <QString>
#include <QVector>
#include "coordinate.h"

namespace locationtoolkit
{
class Maneuver
{
public:
    virtual ~Maneuver(){}

    /**
     * Gets unique ID identifying the maneuver.
     *
     * @return the maneuverId
     */
    virtual qint32 GetManeuverID() const = 0;

    /**
     * Returns the coordinates on route polyline for this maneuver
     *
     * @return vector of polyline
     */
    virtual const QVector<Coordinates*>& GetPolyline() const = 0;

    /**
     * Gets the string to be used to draw the routing icon using the TT guidance font.
     *
     * @return string
     */
    virtual const QString& GetRoutingTTF() const = 0;

    /**
     * Returns distance.
     *
     * @return distance value in meters
     */
    virtual qreal GetDistance() const = 0;

    /**
     * Returns primary street.
     *
     * @return primary street string
     */
    virtual const QString& GetPrimaryStreet() const = 0;

    /**
     * Returns secondary description.
     *
     * @return the second street name
     */
    virtual const QString& GetSecondaryStreet() const = 0;

    /**
     * Returns the maneuver actual time
     *
     * @return the actual maneuver time
     */
    virtual qreal GetTime() const = 0;

    /**
     * Gets commmand
     *
     * @return commang name
     */
    virtual const QString& GetCommand() const = 0;

    /**
     * Gets point coordinates
     *
     * @return point coordinates
     */
    virtual const Coordinates& GetPoint() const = 0;

    /**
     * Gets formatted maneuver text
     *
     * @param isMetric set to {@code true} to indicate that metric system is used
     * @return {@link FormattedText} object
     */
    //virtual FormattedTextBlock GetManeuverText(bool isMetric) const = 0;

    /**
     * Returns maneuver description.
     *
     * @param isMetric
     * @return maneuver description string
     */
    virtual const QString& GetDescription(bool isMetric) const = 0;

    /**
     * Returns the maneuver delay time
     *
     * @return the total delay
     */
    virtual qreal GetTrafficDelay() const = 0;


    /**
     * Returns true if the next maneuver should be stacked
     *
     * @return the stack advise
     */
    virtual bool GetStackAdvise() const = 0;

    /**
     * Gets exit number
     *
     * @return exit number text
     */
    virtual const QString& GetExitNumber() const = 0;

    /**
     * Get if this maneuver is destination or not
     *
     * @return true if it is a destination maneuver
     */
    virtual bool IsDestination() const = 0;
};
} // namespace locationtoolkit
#endif
/*! @} */
