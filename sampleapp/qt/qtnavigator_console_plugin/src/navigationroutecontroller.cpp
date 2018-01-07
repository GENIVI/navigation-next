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

#include "navigationroutecontroller.h"

// navigationRouteController Constructor with default initializations
navigationRouteController::navigationRouteController(QQuickItem *parent):
    QQuickItem(parent),
    mNavRouteController(NULL)
{

}
navigationRouteController::~navigationRouteController()
{
    if(mNavRouteController != NULL) {
        delete mNavRouteController;
        mNavRouteController = NULL;
    }

}

/*#########################################################################################

    SetUp percentage dimensions for UI to make the designs modular and scalable

 *#########################################################################################*/
void navigationRouteController::setUp() {

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    QString appDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    qDebug()<<"engine::"<<engine;
    QQmlComponent component(engine,QUrl(appDirPath+"/resource/NavigationRouteController.qml"));
    mNavRouteController  = qobject_cast<QQuickItem*>(component.create());
    qDebug()<<"mNavRouteController::"<<mNavRouteController;
    QQmlEngine::setObjectOwnership(mNavRouteController, QQmlEngine::CppOwnership);
    QQmlEngine::setContextForObject(mNavRouteController, QQmlEngine::contextForObject(this));
    mNavRouteController->setParentItem(this);
    //mNavRouteController->setParent(this);


    int routeWidth = this->width()*0.3;
    int routeHeight = this->height() - this->height()/5 - (this->height()/5 - this->height()/8.5);
    qDebug() << "Route: " << routeWidth << routeHeight;
    //    mNavRouteController->setPosition(QPointF(40, this->height()/6));
    mNavRouteController->setPosition(QPointF(40, 30 + this->height()/6));
    mNavRouteController->setWidth(routeWidth);
    mNavRouteController->setHeight(routeHeight);

    connect( mNavRouteController, SIGNAL(routeFirstClicked()), this, SLOT(onRouteFirstClicked()) );
    connect( mNavRouteController, SIGNAL(routeSecondClicked()), this, SLOT(onRouteSecondClicked()) );
    connect( mNavRouteController, SIGNAL(routeThirdClicked()), this, SLOT(onRouteThirdClicked()) );
    connect( mNavRouteController, SIGNAL(routeFirstGoClicked()), this, SLOT(onRouteFirstGoClicked()) );
    connect( mNavRouteController, SIGNAL(routeSecondGoClicked()), this, SLOT(onRouteSecondGoClicked()) );
    connect( mNavRouteController, SIGNAL(routeThirdGoClicked()), this, SLOT(onRouteThirdGoClicked()) );

    mNavRouteController->setVisible(false);
    //  Loading specific fonts from resource files
    //    QString fontPath_m = LTKSampleUtil::GetResourceFolder() + "font/SF-UI-Display-Medium.otf";
    //    int id_m = QFontDatabase::addApplicationFont(fontPath_m);
    //    QString fontFamily_m = QFontDatabase::applicationFontFamilies(id_m).at(0);
    //    QFont sfDisplayMedium(fontFamily_m);

    //    QString fontPath_r = LTKSampleUtil::GetResourceFolder() + "font/SF-UI-Display-Regular.otf";
    //    int id_r = QFontDatabase::addApplicationFont(fontPath_r);
    //    QString fontFamily_r = QFontDatabase::applicationFontFamilies(id_r).at(0);
    //    QFont sfDisplayRegular(fontFamily_r);

    //    this->setAttribute(Qt::WA_StyledBackground,true);
    //    QString bgStyle = QString("#navigationRouteController{background-color: rgba(20,40,49,255);border: 1px solid white;}");
    //    this->setStyleSheet(bgStyle);

    //    ui->mainWidget->setFixedSize(this->width()*0.4, this->height() - this->height()*2/7);
    //    ui->mainWidget->setStyleSheet("background-color: transparent;");

    //    int mainHeight = this->height() - this->height()/5 - (this->height()/5 - this->height()/7);
    //    ui->route1->setFixedHeight(mainHeight/5);
    //    ui->route1->setStyleSheet("background-color: rgba(0,0,0,180); border: 1px solid white; border-radius: 50px;");
    //    ui->route1->setContentsMargins(0,0,10,0);
    //    ui->go1->setFixedSize(ui->route1->height() - 30, ui->route1->height() - 30);
    //    ui->go1->setStyleSheet("background: none; border: 3px solid #FF7C00; border-radius: 40px; color: white;");
    //    ui->go1->setText("GO");
    //    ui->distance1->setStyleSheet("background: none; border: none; color: white; padding-left: 30px; padding-top: 9px");
    //    ui->routeType1->setStyleSheet("background: none; border: none; color: #FF7C00; padding-left: 30px; padding-bottom: 9px");
    //    ui->routeType1->setText("Route 1");

    //    ui->route2->setFixedHeight(mainHeight/5);
    //    ui->route2->setStyleSheet("background-color: rgba(0,0,0,180); border: 1px solid white; border-radius: 50px;");
    //    ui->route2->setContentsMargins(0,0,10,0);
    //    ui->go2->setFixedSize(ui->route2->height() - 30, ui->route2->height() - 30);
    //    ui->go2->setStyleSheet("background: none; border: 3px solid #00DEFF; border-radius: 40px; color: white;");
    //    ui->go2->setText("GO");
    //    ui->distance2->setStyleSheet("background: none; border: none; color: white; padding-left: 30px; padding-top: 9px");
    //    ui->routeType2->setStyleSheet("background: none; border: none; color: #00DEFF; padding-left: 30px; padding-bottom: 9px");
    //    ui->routeType2->setText("Route 2");

    //    ui->route3->setFixedHeight(mainHeight/5);
    //    ui->route3->setStyleSheet("background-color: rgba(0,0,0,180); border: 1px solid white; border-radius: 50px;");
    //    ui->route3->setContentsMargins(0,0,10,0);
    //    ui->go3->setFixedSize(ui->route3->height() - 30, ui->route3->height() - 30);
    //    ui->go3->setStyleSheet("background: none; border: 3px solid #00F938; border-radius: 40px; color: white;");
    //    ui->go3->setText("GO");
    //    ui->distance3->setStyleSheet("background: none; border: none; color: white; padding-left: 30px; padding-top: 9px");
    //    ui->routeType3->setStyleSheet("background: none; border: none; color: #00F938; padding-left: 30px; padding-bottom: 9px");
    //    ui->routeType3->setText("Route 3");

    //    sfDisplayMedium.setPixelSize(this->height()*0.0533);
    //    ui->distance1->setFont(sfDisplayMedium);
    //    ui->distance2->setFont(sfDisplayMedium);
    //    ui->distance3->setFont(sfDisplayMedium);

    //    ui->go1->setFont(sfDisplayMedium);
    //    ui->go2->setFont(sfDisplayMedium);
    //    ui->go3->setFont(sfDisplayMedium);

    //    sfDisplayRegular.setPixelSize(this->height()*0.0467);
    //    ui->routeType1->setFont(sfDisplayRegular);
    //    ui->routeType2->setFont(sfDisplayRegular);
    //    ui->routeType3->setFont(sfDisplayRegular);

    //    ui->routeType1->installEventFilter(this);
    //    ui->route1->installEventFilter(this);
    //    ui->distance1->installEventFilter(this);
    //    ui->go1->installEventFilter(this);

    //    ui->routeType2->installEventFilter(this);
    //    ui->route2->installEventFilter(this);
    //    ui->distance2->installEventFilter(this);
    //    ui->go2->installEventFilter(this);

    //    ui->routeType3->installEventFilter(this);
    //    ui->route3->installEventFilter(this);
    //    ui->distance3->installEventFilter(this);
    //    ui->go3->installEventFilter(this);
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxx END ... SetUp percentage dimensions xxxxxxxxxxxxxxxxxxxxxxxxxxx

// update labels as per routes retrieved from navigation session
void navigationRouteController::updateLabels(qreal* distances, qreal* totalTimes) {

    //    ui->route1->setHidden(true);
    //    ui->route2->setHidden(true);
    //    ui->route3->setHidden(true);

    QObject *object = mNavRouteController;

    QQuickItem *route1 = object->findChild<QQuickItem*>("route1");
    if (route1){
        //        route1->setProperty("color", QColor(Qt::yellow));
        route1->setVisible(false);
    }
    QQuickItem *route2 = object->findChild<QQuickItem*>("route2");
    if (route2){
        route2->setVisible(false);
    }
    QQuickItem *route3 = object->findChild<QQuickItem*>("route3");
    if (route3){
        route3->setVisible(false);
    }

    if (distances[0] != -1 && totalTimes[0] != -1) {
        route1->setVisible(true);
        QQuickItem *distance1 = object->findChild<QQuickItem*>("distance1");
        QString dist = LTKSampleUtil::FormatDistance(distances[0]);
        distance1->setProperty("text",dist);
    }

    if (distances[1] != -1 && totalTimes[1] != -1) {
        route2->setVisible(true);
        QQuickItem *distance2 = object->findChild<QQuickItem*>("distance2");
        QString dist = LTKSampleUtil::FormatDistance(distances[1]);
        distance2->setProperty("text",dist);
    }

    if (distances[2] != -1 && totalTimes[2] != -1) {
        route3->setVisible(true);
        QQuickItem *distance3 = object->findChild<QQuickItem*>("distance3");
        QString dist = LTKSampleUtil::FormatDistance(distances[2]);
        distance3->setProperty("text",dist);
    }

}

void navigationRouteController::onRouteFirstClicked() {
    //    emit highlightRoute(0);
    emit startNavigationOnRoute(0);
}

void navigationRouteController::onRouteSecondClicked() {
    //    emit highlightRoute(1);
    emit startNavigationOnRoute(1);
}

void navigationRouteController::onRouteThirdClicked() {
    //    emit highlightRoute(2);
    emit startNavigationOnRoute(2);
}

void navigationRouteController::onRouteFirstGoClicked() {
    emit startNavigationOnRoute(0);

}

void navigationRouteController::onRouteSecondGoClicked() {
    emit startNavigationOnRoute(1);

}

void navigationRouteController::onRouteThirdGoClicked() {
    emit startNavigationOnRoute(2);

}


// event filters to respond to button click events of routes
//bool navigationRouteController::eventFilter(QObject* object, QEvent* event) {

//    if (event->type() == QEvent::MouseButtonPress) {

//        if (object == ui->routeType1 || object == ui->route1 || object == ui->distance1) {
//            emit highlightRoute(0);
//        }
//        else if (object == ui->routeType2 || object == ui->route2 || object == ui->distance2) {
//            emit highlightRoute(1);
//        }
//        else if (object == ui->routeType3 || object == ui->route3 || object == ui->distance3) {
//            emit highlightRoute(2);
//        }

//        else {

//            if (object == ui->go1) {
//                emit startNavigationOnRoute(0);
//            }
//            else if (object == ui->go2) {
//                emit startNavigationOnRoute(1);
//            }
//            else if (object == ui->go3) {
//                emit startNavigationOnRoute(2);
//            }
//            else {

//            }

//        }

//    }

//    return QWidget::eventFilter(object, event);

//}
void navigationRouteController::show()
{
    if(!mNavRouteController->isVisible())
        mNavRouteController->setVisible(true);
}
void navigationRouteController::hide()
{
    if(mNavRouteController->isVisible())
        mNavRouteController->setVisible(false);
}
