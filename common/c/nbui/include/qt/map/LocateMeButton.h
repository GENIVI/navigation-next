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

    @file locatemebutton.h
    @date 08/26/2014
    @defgroup nbui
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

#ifndef _LOCATE_ME_BUTTON_H_
#define _LOCATE_ME_BUTTON_H_

#include <QPushButton>

class LocateMeButton : public QPushButton
{
    Q_OBJECT

public:
    enum ShowMode
    {
        SM_INVALID = 0,
        SM_LOCATE_ME,
        SM_FOLLOW_ME_ANYHEADING,
        SM_FOLLOW_ME
    };

public:
    explicit LocateMeButton(QWidget *parent = 0);
    ~LocateMeButton();

public:
    void SetEnabled(bool enabled);
    void SetShowMode(ShowMode mode);

protected:
    virtual void paintEvent(QPaintEvent *paintEvent);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
private:
    void Repaint();
    QString GetCurrentPath();

private:
    bool      mEnabled;
    QPixmap*  mPixmapFollowmeOff;
    QPixmap*  mPixmapFollowmeOn;
    QPixmap*  mPixmapFollowmeCompass;
    QPixmap*  mCurrentPixmap;
    QPixmap*  mBackgroudPixmap;
};

#endif // _LOCATE_ME_BUTTON_H_
