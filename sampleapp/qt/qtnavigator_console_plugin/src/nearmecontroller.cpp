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

#include <QtCore>
#include <QGuiApplication>
#include <QDebug>
#include<QQmlEngine>
#include<QQmlContext>
#include "util.h"
#include "nearmecontroller.h"
#include "nearmedataobject.h"
#include "categorysearchcontroller.h"
NearMeController::NearMeController(QQuickItem* parent):
    QQuickItem(parent),
    mNearMeView(NULL){


}
NearMeController::~NearMeController()
{
    if(mNearMeView != NULL) {
        delete mNearMeView;
        mNearMeView = NULL;
    }
}
void NearMeController::setUp()
{
    qDebug() << "NearMeController::setUp()...................";
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    QString appDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    QQmlComponent component(engine,QUrl(appDirPath+"/resource/NearMeSubMenu.qml"));

    mNearMeView  = qobject_cast<QQuickItem*>(component.create());
    QQmlEngine::setObjectOwnership(mNearMeView, QQmlEngine::CppOwnership);
    mNearMeView->setParentItem(this);

    mNearMeView->setFlag(ItemHasContents);
    mNearMeView->setVisible(true);

    isNearByPinLocation = false;
    engine->rootContext()->setContextProperty("isNearByPinLocation", false);

    connect( mNearMeView, SIGNAL(nearMeItemsTriggered(QString, QString)), this, SLOT(onNearMeItemsTriggered(QString, QString)) );
    connect( mNearMeView, SIGNAL(closeNearMeMenu()), this, SLOT(oncloseNearMeMenu()) );

    QString categoryListFilePath = appDirPath + "/resource/jsons/category_list.json";
    qDebug() << " categoryListFilePath: " << categoryListFilePath;

    QFileInfo checkCategoryFile(categoryListFilePath );

    QList<QObject*> dataList;

    if (checkCategoryFile.exists()) {
        QJsonDocument doc = LTKSampleUtil::loadJsonDocumentFromFile(categoryListFilePath);
        //updateCategoryGridView(doc);

        // check validity of the document
        if (!doc.isNull()) {

            if (doc.isObject()) {

                QJsonObject obj = doc.object();
                QJsonArray categoryArray = obj["category_list"].toArray();

                for (int itemIndex = 0; itemIndex < categoryArray.count(); ++itemIndex) {
                    QJsonValue value = categoryArray.at(itemIndex);
                    QJsonObject categoryObject = value.toObject();

                    QString code, icon, catName;

                    code = categoryObject["code"].toString();
                    icon = "";

                    //categoryObject["code"] = categoryObject["code"].toString();
                    //categoryObject["icon"] = "";
                    QJsonArray nameArray = categoryObject["name"].toArray();

                    foreach (const QJsonValue name, nameArray) {
                        QJsonObject categoryName = name.toObject();
                        catName = categoryName["full_name"].toString();
                        //categoryObject["name"] = categoryName["full_name"].toString();
                    }

                    //dataList.append(new NearMeDataObject(categoryObject["name"].toString(), categoryObject["code"].toString()));
                    dataList.append(new NearMeDataObject(catName, code));

                }
            }
            else {
                qDebug() << "Document is not an object" << endl;
            }

        }
        else {
            qDebug() << "Invalid JSON...\n" << endl;
        }
    }
    else {
        //  perform search using Search REST API
        //  categoryAPIRequest();
    }
    engine->rootContext()->setContextProperty("nearMeModel", QVariant::fromValue(dataList));
}
void NearMeController::onNearMeItemsTriggered(QString name, QString code) {

    //  Set category to Theatres and enable search for Theatres near by
    QJsonObject categoryParameter;
    categoryParameter["code"] = code;
    categoryParameter["name"] = name;
    QVariant parameter = categoryParameter;

    emit nearMeItemsClicked(parameter,isNearByPinLocation);
}
void NearMeController::oncloseNearMeMenu() {
    mNearMeView->setVisible(false);
}
