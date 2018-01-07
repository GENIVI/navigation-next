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
#include <QDebug>

#include "maneuverslistcontroller.h"
#include "maneuverdataobject.h"

ManeuversListController::ManeuversListController(QQuickItem* parent):
    QQuickItem(parent),
    mManeuversListView(NULL){


}
ManeuversListController::~ManeuversListController()
{
    //    if(mManeuversListView != NULL) {
    //        delete mManeuversListView;
    //        mManeuversListView = NULL;
    //    }
    mManeuversListView = NULL;
}
void ManeuversListController::setUp()
{
    qDebug()<<"******************* ManeuversListController::setUp *****************";

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    mAppDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();

    QQmlComponent component(engine,QUrl(mAppDirPath+"/resource/ManeuverListView.qml"));

    mManeuversListView  = qobject_cast<QQuickItem*>(component.create());
    mManeuversListView->setParentItem(this->parentItem()/*->parentItem()*/);

    connect( mManeuversListView, SIGNAL(backButtonClicked()), this, SLOT(onBackButtonClicked()) );
    connect( mManeuversListView, SIGNAL(mapToggleButtonClicked()), this, SLOT(onMapToggleButtonClicked()) );

    //mManeuversListView->setVisible(false);
}
void ManeuversListController::setDestinationName(QString destName) {

    QQmlEngine *engine = QQmlEngine::contextForObject(mManeuversListView)->engine();
    if(!destName.isEmpty())
        engine->rootContext()->setContextProperty("placeName",destName.left(12) + "...");
}
void ManeuversListController::OnTripRemainingTime(quint32 time) {
    QString convertedTime = FormatNavTime(time);//= "   |   ";
    //convertedTime.append(FormatNavTime(time));
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    engine->rootContext()->setContextProperty("pETA",convertedTime);
}

void ManeuversListController::OnTripRemainingDistance(qreal distance) {

    QString dist = LTKSampleUtil::FormatDistance(distance);
    QQmlEngine *engine = QQmlEngine::contextForObject(mManeuversListView)->engine();
    engine->rootContext()->setContextProperty("pDistance",dist);
    engine->rootContext()->setContextProperty("separatorVisible",true);
}

void ManeuversListController::setManeuverLabels(QString imageId,QString streetName,QString distance/*,QString warning*/){

    //QString arrowImageString = "image://pixmap/" + imageId;
    mManeuverDataList.append(new ManeuverDataObject(imageId,streetName,distance));

}
void ManeuversListController::loadManeuverList(){


    qDebug()<<"ManeuversListController::loadManeuverList :: " << mManeuverDataList.count();
    QQmlEngine *engine = QQmlEngine::contextForObject(this/*mManeuversListView*/)->engine();
    engine->rootContext()->setContextProperty("maneuverListModel", QVariant::fromValue(mManeuverDataList));
}

void ManeuversListController::onBackButtonClicked(){
    mManeuversListView->setVisible(false);
    this->setVisible(false);
    emit destroyManeuverList();
}
void ManeuversListController::onMapToggleButtonClicked(){
    mManeuversListView->setVisible(false);
    this->setVisible(false);
    emit destroyManeuverList();
}

void ManeuversListController::clearFields() {
    //mManeuversListView->setVisible(false);
    QQmlEngine *engine = QQmlEngine::contextForObject(mManeuversListView)->engine();

    //engine->rootContext()->setContextProperty("placeName"," ");
    engine->rootContext()->setContextProperty("pDistance"," ");
    engine->rootContext()->setContextProperty("pETA"," ");
    engine->rootContext()->setContextProperty("separatorVisible",false);

}
QString ManeuversListController::FormatNavTime(quint32 time) {

    QString buffer;
    int hours = time / 3600;
    int remainder = time - hours * 3600;
    int mins = remainder / 60;
    remainder = remainder - mins * 60;
    int secs = remainder;

    if (hours > 0 /*&& mins >= 0 && secs >= 0*/) {
        buffer = QString("%1:%2 h").arg(hours < 10 ? "0"+QString::number(hours) : QString::number(hours), mins < 10 ? "0"+QString::number(mins) : QString::number(mins));
    }
    else if(mins > 0 /*&& secs >= 0*/) {
        buffer = QString("%1:%2 m").arg(mins < 10 ? "0"+QString::number(mins) : QString::number(mins) , secs < 10 ? "0"+QString::number(secs) : QString::number(secs));
    }
    else if(secs > 0) {
        buffer = QString("%1 s").arg(secs < 10 ? "0"+QString::number(secs) : QString::number(secs));
    }
    else {
        buffer = "";
    }

    return buffer;

}
//void ManeuversListController::OnUpdateManeuverList(const ManeuverList& maneuverlist) {
//    qDebug() << "**************************** ManeuversListController::OnUpdateManeuverList******************************";
//    clearFields();
//    if(mManeuverDataList.length() > 0)
//    {
//        mManeuverDataList.clear();
//    }
//    quint32 maneuverCount = maneuverlist.GetNumberOfManeuvers();
//    for(int i = 0; i < (int)maneuverCount; ++i) {
//        const Maneuver* maneuver = maneuverlist.GetManeuver(i);

//        // Add Search Result Row to List View Widget
//        //maneuverItemWidget* maneuverWidget = new maneuverItemWidget();
//        //maneuverWidget->setGeometry(0,0,mManeuverWidgetItemWidth,mManeuverWidgetItemHeight);
//        QString streetName;
//        if(i!=(maneuverCount - 1)) {
//            streetName = maneuver->GetPrimaryStreet();
//        }
//        else {
//            streetName = "You have arrived at your destination";
//        }
//        setManeuverLabels(maneuver->GetRoutingTTF(), streetName, FormatDistance(maneuver->GetDistance())/*,"No Warning"*/);
//    }

//    loadManeuverList();
//}

void ManeuversListController::OnManeuverRemainingDistance(qreal distance) {

    //check model list count, if greater than zero, reset list index, and update distance.
    /*
     if(ui->listWidget->count() > 0)
     {
         QListWidgetItem *item = ui->listWidget->item(0);
         if(item)
         {
//            item->setData(Qt::DecorationRole,FormatDistance(distance, mIsMetric, mIsYard));
             maneuverItemWidget *widget = qobject_cast<maneuverItemWidget*>(ui->listWidget->itemWidget(item));
             widget->setManeuverLabels("", "", FormatDistance(distance, mIsMetric, mIsYard));
         }
     }*/
    if(mManeuverDataList.length() > 0)
    {
        //get item at index = 0
        //for that item set distance
    }

}

void ManeuversListController::OnRouteReceived() {
    int a = 1;
}

void ManeuversListController::setManeuverList(QList<QObject *> pManeuverDataList)
{
    if(mManeuverDataList.length() > 0)
    {
        mManeuverDataList.clear();
    }
    mManeuverDataList = pManeuverDataList;
}
void ManeuversListController::show(){
    if(!mManeuversListView->isVisible())
        mManeuversListView->setVisible(true);

    this->setVisible(true);
    clearFields();
    loadManeuverList();
}
void ManeuversListController::hide(){
    if(mManeuversListView->isVisible())
        mManeuversListView->setVisible(false);

    this->setVisible(false);

}
