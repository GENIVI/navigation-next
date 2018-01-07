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

#include "confirmationboxcontroller.h"
ConfirmationBoxController::ConfirmationBoxController(QQuickItem* parent):
    QQuickItem(parent),
    mConfirmBoxView(NULL),bNavMode(false){


}
void ConfirmationBoxController::setUp() {
    // qDebug()<<"******************* ConfirmBoxController::setUp *****************";
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    QString appDirPath = engine->rootContext()->contextProperty("applicationDirPath1").toString();

    QQmlComponent component(engine,QUrl(appDirPath+"/resource/ConfirmationBox.qml"));
    if(component.isError())
    {
        qDebug()<<"******************* ConfirmBoxController::setUp *****************" << component.errors();
        return;
    }else{

        mConfirmBoxView  = qobject_cast<QQuickItem*>(component.create());
        qDebug()<<"******************* ConfirmBoxController::setUp *****************" << mConfirmBoxView;

        QQmlEngine::setObjectOwnership(mConfirmBoxView, QQmlEngine::CppOwnership);
        mConfirmBoxView->setParentItem(this->parentItem()->parentItem());
        QQmlEngine::contextForObject(mConfirmBoxView)->engine();

        connect(mConfirmBoxView, SIGNAL(confirmBoxValueClicked(bool)),this,SLOT(onConfirmBoxClicked(bool)));
        connect(mConfirmBoxView, SIGNAL(lowFuelLimit(bool)),this,SLOT(onLowFuelLimit(bool)));
        mConfirmBoxView->setFlag(ItemHasContents);
        setVisibility(false);
    }
}

//void ConfirmationBoxController::onConfirmBoxClicked(bool val)
//{
//    qDebug()<<"ConfirmBoxController::onConfirmBoxClicked";
//    this->setVisible(false);
//    mConfirmBoxView->setVisible(false);
//    emit confirmBoxNotification(val);
//}
void ConfirmationBoxController::onLowFuelLimit(bool visible)
//void ConfirmationBoxController::onLowFuelLimit(int flag)

{

    if(visible){
        setVisibility(visible);
    }else{
        setVisibility(visible);
    }
}

void ConfirmationBoxController::setVisibility(bool visible)
{
    qDebug()<<"ConfirmationBoxController::setVisibility" << bNavMode;
    if(mConfirmBoxView){
        if(!bNavMode){
            mConfirmBoxView->setVisible(visible);
            emit lowFuelLimit(visible);
        }else{
            mConfirmBoxView->setVisible(false);
        }
    }
}

void ConfirmationBoxController::isNavMode(bool mode){
    qDebug()<<"ConfirmationBoxController::isNavMode" << mode;
    bNavMode = mode;
}
