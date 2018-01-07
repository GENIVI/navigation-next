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

    @file     useroptions.h
    @date     08/15/2014
    @defgroup USEROPTIONS_H

    user options widget definition
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef USEROPTIONS_H
#define USEROPTIONS_H

#include <QWidget>
#include <QColor>

namespace Ui {
class UserOptions;
}

class UserOptions : public QWidget
{
    Q_OBJECT

public:
    enum CAMERA_MOVE_TYPE
    {
        CMT_UP = 1,
        CMT_DOWN,
        CMT_LEFT,
        CMT_RIGHT,
    };

    enum CAMERA_ZOOM_TYPE
    {
        CZT_IN = 1,
        CZT_OUT,
    };

    enum CAMERA_TILT_TYPE
    {
        CTT_UP = 1,
        CTT_DOWN,
    };

    explicit UserOptions(QWidget *parent = 0);
    ~UserOptions();

    void SetMainWindow( QWidget* mainwin );

signals:
    void SigMoveCamera(UserOptions::CAMERA_MOVE_TYPE moveType);
    void SigShowNight( bool bNight );
    void SigShowCompass( bool bShow );
    void SigShowDoppler( bool bShow );
    void SigShowTraffic( bool bShow );

private slots:
    void on_camaraBtn_clicked();
    void on_backButton_clicked();
    void on_generalButton_clicked();
    void on_switchDayNightButton_clicked();
    void on_showCompassButton_clicked();
    void on_showDopplerButton_clicked();
    void on_showTrafficButton_clicked();
    void on_polylineButton_clicked();
    void on_circleFillColorButton_clicked();
    void on_circleStrokeColorButton_clicked();
    void on_upButton_clicked();
    void on_leftButton_clicked();
    void on_rightButton_clicked();
    void on_downButton_clicked();

private:
    Ui::UserOptions *ui;

    bool mIsNightMode;
    bool mIsCompassShown;
    bool mIsDopplerShown;
    bool mIsTrafficShown;

    QColor mCircleFillColor;
    QColor mCircleStrokeColor;
};

#endif // USEROPTIONS_H

/*! @} */
