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

#include "messageboxcontroller.h"
#include<QGuiApplication>
#include<QQmlEngine>
#include<QQmlContext>
Messageboxcontroller::Messageboxcontroller(QQuickItem* parent):
    QQuickItem(parent),
    mMessageBoxView(NULL){


}
Messageboxcontroller::~Messageboxcontroller()
{
    //    if(mMessageBoxView != NULL) {
    //        delete mMessageBoxView;
    //        mMessageBoxView = NULL;
    //    }
}
void Messageboxcontroller::setUp()
{
    qDebug() << "Messageboxcontroller::setUp()...................";

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    QString appDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    QQmlComponent component(engine,QUrl(appDirPath+"/resource/MessageBox.qml"));

    mMessageBoxView  = qobject_cast<QQuickItem*>(component.create());
    QQmlEngine::setObjectOwnership(mMessageBoxView, QQmlEngine::CppOwnership);
    mMessageBoxView->setParentItem(this);
    engine->rootContext()->setContextProperty("messageText1",mVar1);
    engine->rootContext()->setContextProperty("messageText2",mVar2);

    mMessageBoxView->setFlag(ItemHasContents);
    mMessageBoxView->setVisible(true);

    connect( mMessageBoxView, SIGNAL(okButtonClicked()), this, SLOT(onOkButtonClicked()) );

}
void Messageboxcontroller::messageStrings(QString var1,QString var2){
    mVar1 = var1;
    mVar2 = var2;
}

void Messageboxcontroller::onOkButtonClicked(){

    mMessageBoxView->setVisible(false);
    this->setVisible(false);
    emit okClicked();
}
