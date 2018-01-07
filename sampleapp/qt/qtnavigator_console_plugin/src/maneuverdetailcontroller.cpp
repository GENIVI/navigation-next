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

#include "maneuverdetailcontroller.h"
#include "pixmapprovider.h"
//  ManeuverDetailController Constructor with default initializations
ManeuverDetailController::ManeuverDetailController(QQuickItem* parent):
    QQuickItem(parent),
    mManeuverView(NULL){


}

/*#########################################################################################

    SetUp percentage dimensions for UI to make the designs modular and scalable

 *#########################################################################################*/
void ManeuverDetailController::setUp() {

    qDebug()<<"******************* ManeuverDetailController::setUp *****************";

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    QString mAppDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    QQmlComponent component(engine,QUrl(mAppDirPath+"/resource/ManeuverDetail.qml"));
    mManeuverView  = qobject_cast<QQuickItem*>(component.create());
    QQmlEngine::setObjectOwnership(mManeuverView, QQmlEngine::CppOwnership);
    mManeuverView->setParentItem(this);

    QQmlEngine *Maneuverengine = QQmlEngine::contextForObject(mManeuverView)->engine();
    Maneuverengine->addImageProvider("pixmap", new PixmapProvider);
    //.engine()->addImageProvider(QLatin1String("arrowPixMap"), new PixmapProvider);
    // previous width-height
    //    int maneuverWidth = this->width()*0.3;
    //    int maneuverHeight = this->height() - this->height()/5 - (this->height()/5 - this->height()/8.5);
    //    qDebug() << "Route: " << maneuverWidth << maneuverHeight;
    //    //mManeuverView->setPosition(QPointF(40, this->height()/6));
    //    mManeuverView->setPosition(QPointF(40, 30 + this->height()/6));
    //    mManeuverView->setWidth(maneuverWidth);
    //    mManeuverView->setHeight(maneuverHeight);

    mManeuverView->setVisible(false);


}

//xxxxxxxxxxxxxxxxxxxxxxxxxxx END ... SetUp percentage dimensions xxxxxxxxxxxxxxxxxxxxxxxxxxx

//  set Maneuver details as per details fetched from navigation session
void ManeuverDetailController::updateManeuverInfoLabels(QString nextArrow, QString nextStreetName, QString streetArrow,
                                                        QString streetName, QString streetDistance, QString currentStreet) {

}

//  close button click slot
void ManeuverDetailController::on_closeButton_clicked() {

    hide();

}

/*#########################################################################################

    Navigation related slots

 *#########################################################################################*/

//  slot executed when trip remaining time changes
void ManeuverDetailController::OnTripRemainingTime(quint32 time) {

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

    //    ui->timeLabel->setText(buffer);
    rootContext()->setContextProperty("timeText", buffer);

    QTime timeNow = QTime::currentTime();
    //    //qDebug() << "Now: "<< timeNow.toString("hh:mm A") << "-------" << QString::number(h) << " "<< QString::number(m)<<" "<<QString::number(s);
    QTime timeArrival = timeNow.addSecs(((int)hours*60*60) + ((int)mins*60) + (int)secs);
    rootContext()->setContextProperty("arrivalText", timeArrival.toString("hh:mm A"));
    //    //qDebug() << "Arrival: "<<timeArrival.toString("hh:mm A");

}

//  slot executed when trip remaining distance changes
void ManeuverDetailController::OnTripRemainingDistance(qreal dist) {

    QString distance = LTKSampleUtil::FormatDistance(dist);
    rootContext()->setContextProperty("distanceText", distance);
}

//  slot executed when current road name changes
void ManeuverDetailController::OnCurrentRoadName(const QString& primaryName, const QString& secondaryName) {

    //    //qDebug()<<"=====================OnCurrentRoadName=====================";
    //    //qDebug() << primaryName + ", " + secondaryName;

    QString streetName = secondaryName.length() > 0 ? primaryName + ", " + secondaryName : primaryName;
    if(streetName.length() > 16) {
        rootContext()->setContextProperty("currentStreetName", streetName/*.left(18) + "..."*/);
    }
    else {
        rootContext()->setContextProperty("currentStreetName", streetName/*.left(18) + "..."*/);
    }
}

//  slot executed when next road name changes
void ManeuverDetailController::OnNextRoadName(const QString& primaryName, const QString& secondaryName) {

    //    //qDebug()<<"=====================OnNextRoadName=====================";
    //    //qDebug()<<primaryName << ", " << secondaryName;
    //    ui->upNextStreetName1->setText( primaryName );
    rootContext()->setContextProperty("upNextStreetName1", primaryName);
    if(secondaryName.length() > 0) {
        //        ui->upNextStreetName2->show();
        //        ui->upNextStreetName2->setText( secondaryName );
        rootContext()->setContextProperty("upNextStreetName2", secondaryName);
    }
    else {
        //        ui->upNextStreetName2->hide();
    }


}

//  slot executed when stack turn image changes
void ManeuverDetailController::OnStackTurnImageTTF(const QString& stackImageTTF) {

    //qDebug()<<"=====================OnStackTurnImageTTF=====================";
    //qDebug()<<stackImageTTF;

}

//  slot executed when maneuver turn image changes
void ManeuverDetailController::OnManeuverImageId(const QString& imageId) {
    qDebug() << "=====================OnManeuverImageId=====================";
    qDebug() << "OnManeuverImageId: " << imageId;

    /*//  Loading specific fonts from resource files
    QString directionFontPath = LTKSampleUtil::GetResourceFolder() + "font/LG_NexGen.ttf";
    int id = QFontDatabase::addApplicationFont(directionFontPath);
    QString directionFontFamily = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont LG_NextGen(directionFontFamily);

    //  Loading specific fonts from resource files
    QString fontPath_m = LTKSampleUtil::GetResourceFolder() + "font/SF-UI-Display-Medium.otf";
    int id_m = QFontDatabase::addApplicationFont(fontPath_m);
    QString fontFamily_m = QFontDatabase::applicationFontFamilies(id_m).at(0);
    QFont sfDisplayMedium(fontFamily_m);

    QString backgroundImg = LTKSampleUtil::GetResourceFolder() + "/images/RouteNavigation_images/maneuverBackground.png";
    QPixmap *pix = new QPixmap(backgroundImg);
    QPainter *paint = new QPainter(pix);

    QLinearGradient gradient;
//    gradient.setStart(ui->upNextDirectionArrow->x(), 0);
//    gradient.setFinalStop(ui->upNextDirectionArrow->x(), ui->upNextDirectionArrow->y() + ui->upNextDirectionArrow->height()/2);
    gradient.setColorAt(0, "#DDF8FF");
    gradient.setColorAt(1, "#0ED9FD");

    QBrush brush(gradient);
    QPen pen;
    pen.setBrush(brush);
    paint->setPen(pen);

//    paint->setPen(*(new QColor(125, 125, 125, 255)));
//    paint->drawRect(100, 50, 50, 50);

    LG_NextGen.setPointSize(75);
    LG_NextGen.setBold(true);
    paint->setFont( LG_NextGen );
    paint->drawText(5,17,200,100, Qt::AlignCenter,imageId);*/


    //    QString arrowImageString = "image://pixmap/" + imageId;//QString::number((qulonglong)pix)
    //    qDebug() << "arrowImageString" << arrowImageString;
    rootContext()->setContextProperty("upNextDirectionArrow", imageId);

    //      rootContext()->setContextProperty("upNextDirectionArrow", imageId);

    //    QIcon* ButtonIcon = new QIcon(*pix);
    //    ui->upNextDirectionArrow->setIcon(*ButtonIcon);
    //    ui->upNextDirectionArrow->setIconSize(pix->size());

}

//  slot executed when lane information changes
void ManeuverDetailController::OnLaneInformation(const locationtoolkit::LaneInformation& laneInfo) {
    //qDebug() << "=====================OnLaneInfo=====================";
    //qDebug() << "";

}

//  slot executed when trafiic color changes
void ManeuverDetailController::OnTripTrafficColor(char color) {
    //    //qDebug() << "=====================OnTrafficColor=====================";
    //    //qDebug() << "";

}

//  slot executed when road sign changes
void ManeuverDetailController::OnRoadSign(const locationtoolkit::RoadSign& roadSign) {
    //qDebug() << "=====================OnRoadSign=====================";
    //qDebug() << "";

}

//  slot executed when Maneuver Remaining Distance changes
void ManeuverDetailController::OnManeuverRemainingDistance(qreal distance) {

    QString dist = LTKSampleUtil::FormatDistance(distance);
    rootContext()->setContextProperty("upNextStreetDistance", dist);
}

////  slot executed when Maneuver List updates
//void ManeuverDetailController::OnUpdateManeuverList(const ManeuverList& maneuverlist) {

//   // this->mManeuverlist = maneuverlist;

//    emit AddManeuverArrow(maneuverlist.GetManeuver(0));

////    //qDebug()<<"=====================OnUpdateManeuverList=====================";

//    if (maneuverlist.GetNumberOfManeuvers() > 0) {
//        const Maneuver* maneuverCurrent = maneuverlist.GetManeuver(0);
////        //qDebug()<<"0: " << maneuverCurrent->GetPrimaryStreet() << ", " << maneuverCurrent->GetSecondaryStreet() << maneuverCurrent->GetDistance() << "meters";
//    }

//    if (maneuverlist.GetNumberOfManeuvers() > 1) {
//        const Maneuver* maneuver = maneuverlist.GetManeuver(1);
////        //qDebug()<<"1: " << maneuver->GetPrimaryStreet() << ", " << maneuver->GetSecondaryStreet() << maneuver->GetDistance() << "meters";
//    }

//    if (maneuverlist.GetNumberOfManeuvers() > 2) {
//        const Maneuver* maneuverNext = maneuverlist.GetManeuver(2);
////        //qDebug()<<"2: " << maneuverNext->GetPrimaryStreet() << ", " << maneuverNext->GetSecondaryStreet() << maneuverNext->GetDistance() << "meters";
//    }

//}

//xxxxxxxxxxxxxxxxxxxxxxx END ... Navigation related slots xxxxxxxxxxxxxxxxxxxxxxx

void ManeuverDetailController::clearFields() {

    rootContext()->setContextProperty("upNextStreetDistance", "");
    rootContext()->setContextProperty("upNextStreetName1", "");
    rootContext()->setContextProperty("upNextStreetName2", "");
    rootContext()->setContextProperty("currentStreetName", "");
    rootContext()->setContextProperty("timeText", "");
    rootContext()->setContextProperty("distanceText", "");
    rootContext()->setContextProperty("arrivalText", "");

    //  Loading specific fonts from resource files
    QString directionFontPath = LTKSampleUtil::GetResourceFolder() + "font/LG_NexGen.ttf";
    int id = QFontDatabase::addApplicationFont(directionFontPath);
    QString directionFontFamily = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont LG_NextGen(directionFontFamily);

    QString backgroundImg = LTKSampleUtil::GetResourceFolder() + "/images/RouteNavigation_images/maneuverBackground.png";
    QPixmap *pix = new QPixmap(backgroundImg);

    //    QIcon* ButtonIcon = new QIcon(*pix);
    //    ui->upNextDirectionArrow->setIcon(*ButtonIcon);
    //    ui->upNextDirectionArrow->setIconSize(pix->size());

}
void ManeuverDetailController::show()
{
    qDebug()<<"ManeuverDetailController::show::"<<mManeuverView;
    if(!mManeuverView->isVisible())
        mManeuverView->setVisible(true);

    this->setVisible(true);
    
    
}
void ManeuverDetailController::hide()
{
    if(mManeuverView->isVisible())
        mManeuverView->setVisible(false);

    this->setVisible(false);
}
QQmlContext* ManeuverDetailController::rootContext()
{
    QQmlEngine *engine = QQmlEngine::contextForObject(mManeuverView)->engine();
    return engine->rootContext();
}
//const ManeuverList& ManeuverDetailController::getManeuverList()
//{
//    return mManeuverlist;
//}
