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

#include<QQmlEngine>
#include<QQmlContext>
#include <QDir>
#include "titlebarcontroller.h"

TitleBarController::TitleBarController(QQuickItem *parent):
    QQuickItem(parent)
{

}

void TitleBarController::setUp()
{
    qDebug()<<"******************* TitleBarController::setUp *****************";

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    QString appDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();

    QQmlComponent component(engine, QUrl(appDirPath + "/resource/TitleBar.qml"));
    QQuickItem *titleView  = qobject_cast<QQuickItem*>(component.create());
    QQmlEngine::setObjectOwnership(titleView, QQmlEngine::CppOwnership);

    titleView->setParentItem(this);
    titleView->setVisible(true);

    qDebug()<<"TitleView Width : "<<titleView->width()<<"Height : "<<titleView->height();

    connect(titleView, SIGNAL(settingsButtonClicked()), this, SIGNAL(onSettingsClicked()));
    connect(titleView, SIGNAL(hamburgerMenuButtonClicked()), this, SIGNAL(onMenuClicked()));
    connect(titleView, SIGNAL(showSuggestedSearch(QString)), this, SIGNAL(onSearchButtonClicked(QString)));
    connect(titleView,SIGNAL(hideAllPopUp()),this,SIGNAL(hideAllPopUp()));
}
void TitleBarController::onToggleTitleBarVisibility(bool val)
{
    qDebug()<<"TitleBarController::onToggleTitleBarVisibility";

    this->setVisible(val);


}
