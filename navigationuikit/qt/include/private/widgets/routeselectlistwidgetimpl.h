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
    @file routeselectlistwidgetimpl.h
    @date 10/31/2014
    @addtogroup navigationuikit
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary to
    TeleCommunication Systems, Inc., and considered a trade secret as defined
    in section 499C of the penal code of the State of California. Use of this
    information by anyone other than authorized employees of TeleCommunication
    Systems is granted only under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
    --------------------------------------------------------------------------
*/

/*! @{ */

#ifndef _ROUTESELECTLISTWIDGET_H_
#define _ROUTESELECTLISTWIDGET_H_

#include "widgetbase.h"
#include "RouteSelectListWidget.h"
#include "maneuverdataobject.h"
#include <QtDeclarative/QDeclarativeView>
#include <vector>

namespace locationtoolkit
{
class RouteSelectListWidgetImpl : public WidgetBase
        , public nkui::RouteSelectListWidget
        ,public nkui::RouteSelectHeaderWidget
{
Q_OBJECT
    Q_PROPERTY(QString TripDistance READ GetTripDistance WRITE SetTripDistance NOTIFY TripDistanceChanged)
    Q_PROPERTY(QString TripTime READ GetTripTime WRITE SetTripTime NOTIFY TripTimeChanged)
    Q_PROPERTY(QString Traffic READ GetTraffic WRITE SetTraffic NOTIFY TrafficChanged)
    Q_PROPERTY(QString ViaRoadName READ GetViaRoadName WRITE SetViaRoadName NOTIFY ViaRoadNameChanged)
    Q_PROPERTY(QString RouteTip READ GetRouteTip WRITE SetRouteTip NOTIFY RouteTipChanged)
    Q_PROPERTY(QString StartOn READ GetStartOn WRITE SetStartOn NOTIFY StartOnChanged)
    Q_PROPERTY(QString CurrentRoad READ GetCurrentRoad WRITE SetCurrentRoad NOTIFY CurrentRoadChanged)
public:
    RouteSelectListWidgetImpl();
    virtual ~RouteSelectListWidgetImpl();
    Q_INVOKABLE int IsRouteTipsEmpty();
private:
    QString GetTripDistance() const;
    QString GetTripTime() const;
    QString GetTraffic() const;
    QString GetViaRoadName() const;
    QString GetRouteTip() const;
    QString GetStartOn() const;
    QString GetCurrentRoad() const;
    void SetTripDistance(const QString& TripDistance);
    void SetTripTime(const QString& TripTime);
    void SetTraffic(const QString& traffic);
    void SetViaRoadName(const QString& ViaRoadName);
    void SetRouteTip(const QString& RouteTip);
    void SetStartOn(const QString& StartOn);
    void SetCurrentRoad(const QString& CurrentRoad);
signals:
    void TripDistanceChanged();
    void TripTimeChanged();
    void TrafficChanged();
    void ViaRoadNameChanged();
    void RouteTipChanged();
    void StartOnChanged();
    void CurrentRoadChanged();
public slots:
    void OnSwipRoute(int offset);
private:
    /*! hide a widget. */
    virtual void Hide();
    /*! get widget id . */
    virtual nkui::WidgetID GetWidgetID();
    /*! set presenter to widget. */
    virtual void SetPresenter(nkui::NKUIPresenter* presenter);
    /*! show a widget */
    virtual void Show();

    // override WidgetBase
    virtual QWidget* GetNativeView();

    // implement RouteSelectListWidget
    virtual void UpdateManeuvers(const std::vector<nkui::ManeuverListData>& maneuvers) ;

    /** Update all the traffic informations on a route.**/
    virtual void UpdateTrafficEvents(const std::vector<nkui::TrafficListData>& trafficEvents) ;

    /** Update the list title. **/
    virtual void UpdateTitle(const std::string& hint, const std::string& roadName) ;

    /** Update the distance for top maneuver**/
    virtual void UpdateDistance(const char* distance, const char* unit) ;
    virtual void SetTransportationMode(nbnav::TransportationMode mode);
    /*! set route information.*/
    virtual void SetRouteInfomation(std::vector<nkui::RouteInfoData>& routesData,int selectedIndex);
    /*! select a route. */
    virtual void RouteSelected(uint32 index) ;

    // implement RouteSelectHeaderWidget
    virtual void OnScreenModeChanged();
    virtual void CreateNativeView(QWidget* parentView);
    virtual void OnSetPresenter();

private:
    QDeclarativeView* mRouteSelectlistViewer;
    QObject* mRouteSelectlistObject;
    QDeclarativeContext* mRouteSelectlistContext;
    ManeuverDataModel mManeuverDataModel;
    QString mTripDistance;
    std::vector<nkui::RouteInfoData> mRoutesData;
    QString mTripTime;
    QString mViaRoad;
    QString mRouteTips;
    QString mStartOn;
    QString mCurrentRoad;
    QString mTraffic;
    size_t mSelectedRoute;
};
}

#endif /* _ROUTESELECTLISTWIDGET_H_ */


/*! @} */
