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

#ifndef MAINVIEWCONTROLLER_H
#define MAINVIEWCONTROLLER_H

#include <QQuickItem>
#include "util.h"
#include "pin.h"
#include "location.h"
#include "mapviewcontroller.h"
#include "categorysearchcontroller.h"

using namespace locationtoolkit;

class MainViewController : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY(MainViewController)

    Q_PROPERTY(QString workFolder READ workFolder WRITE setWorkFolder)
    //    Q_PROPERTY(QPointF avatarPosition READ avatarPosition WRITE setAvatarPosition)
    //    Q_PROPERTY(float avatarHeading READ avatarHeading WRITE setAvatarHeading)
    Q_PROPERTY(float tilt READ tilt WRITE setTilt)
    Q_PROPERTY(float zoom READ zoom WRITE setZoom)
    Q_PROPERTY(QString token READ token WRITE setToken)
    Q_PROPERTY(QString client READ client WRITE setClient)
    Q_PROPERTY(bool isFollowMe READ isFollowMe WRITE setIsFollowMe)

public:
    MainViewController(QQuickItem *parent = 0);
    ~MainViewController();

    void setWorkFolder(QString path);
    QString workFolder();

    //    QPointF avatarPosition() const;
    //    void setAvatarPosition(QPointF p);

    //    float avatarHeading() const;
    //    void setAvatarHeading(float t);

    float tilt();
    void setTilt(float t);

    float zoom();
    void setZoom(float t);

    QString token();
    void setToken(QString t);

    QString client();
    void setClient(QString t);

    bool isFollowMe();
    void setIsFollowMe(bool v);

    MapWidget* mLTKMapKit;

public slots:
    void init();
    void onMenuClicked();
    void onSettingsButtonClicked();
    void onSearchButtonClicked(const QString &searchString);


protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);

private:
    bool mInitDone;
    //    QPointF mAvatarPosition;
    //    float mAvatarHeading;
    float mTilt;
    float mZoom;
    QString mToken;
    QString mClient;
    bool mIsFollowMe;
    MapViewController *mMapController;
    CategorySearchController *mCategorySearchController;
    void InitializeCategoryController();

Q_SIGNALS:
    void ToggleTitleBarVisibility(bool);
    void HideSideMenu();
    void changedCentreCoordinate();
};

#endif // MAINVIEWCONTROLLER_H
