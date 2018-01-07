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

    @file mapdecoration.h
    @date 08/06/2014
    @addtogroup mapkit3d
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

#ifndef __MAP_DECORATION_H__
#define __MAP_DECORATION_H__

#include <QObject>
#include <QString>

namespace locationtoolkit
{

class MapDecoration : public QObject
{
    Q_OBJECT
public:
    MapDecoration();
    ~MapDecoration();

    void SetLocateMeButtonEnabled( bool enabled );
    bool IsLocateMeButtonEnabled() const { return mIsLocateMeButtonEnabled; }

    void SetZoomButtonEnabled( bool enabled );
    bool IsZoomButtonEnabled() const { return mIsZoomButtonEnabled; }

    void SetLayerOptionButtonEnabled( bool enabled );
    bool IslayerOptionButtonEnabled() const { return mIsLayerOptionEnabled; }

    void SetCompassPath(const QString& dayModeIconPath, const QString& nightModeIconPath);
    const QString& GetCompassDayModeIconPath() const { return mCompassDayModeIconPath; }
    const QString& GetCompassNightModeIconPath() const { return mCompassNightModeIconPath; }

    void SetCompassPosition(float positionX, float positionY);
    float GetCompassPositionX() const { return mCompassPositionX;}
    float GetCompassPositionY() const { return mCompassPositionY;}

    void SetCompassEnabled( bool enabled);
    bool IsCompassEnabled() const { return mCompassEnabled; }
Q_SIGNALS:
    void CompassEnabledChanged(bool enabled);
    void LocateMeButtonEnabledChanged(bool enabled);
    void ZoomButtonEnabledChanged(bool enabled);
    void LayerOptionButtonEnabledChanged(bool enabled);
    void CompassPathChanged(const QString& dayModeIconPath, const QString& nightModeIconPath);
    void CompassPositionChanged(float positionX, float positionY);

private:
    bool mIsLocateMeButtonEnabled;
    bool mIsZoomButtonEnabled;
    bool mIsLayerOptionEnabled;
    QString mCompassDayModeIconPath;
    QString mCompassNightModeIconPath;
    bool mCompassEnabled;
    float mCompassPositionX;
    float mCompassPositionY;
};

}  //namespace locationtoolkit

#endif // __MAP_DECORATION_H__

/*! @} */
