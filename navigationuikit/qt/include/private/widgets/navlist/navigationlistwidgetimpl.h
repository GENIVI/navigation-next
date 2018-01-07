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
    @file navigationlistwidgetimpl.h
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

#ifndef _NAVIGATION_LISTWIDGET_H_
#define _NAVIGATION_LISTWIDGET_H_

#include "NavigationListWidget.h"
#include "maneuverdataobject.h"
#include "widgetbase.h"
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeContext>

namespace locationtoolkit
{
class NavigationListWidgetImpl :public WidgetBase, public nkui::NavigationListWidget
{
    Q_OBJECT
    Q_PROPERTY(QString TripDistance READ GetTripDistance WRITE SetTripDistance NOTIFY TripDistanceChanged)
    Q_PROPERTY(QString ArrivalTime READ GetArrivalTime WRITE SetArrivalTime NOTIFY ArrivalTimeChanged)
    Q_PROPERTY(QString Traffic READ GetTraffic WRITE SetTraffic NOTIFY TrafficChanged)
    Q_PROPERTY(bool IsArriving READ GetIsArriving WRITE SetIsArriving NOTIFY IsArrivingChanged)
    Q_PROPERTY(QString DestinationChar READ GetDestinationChar WRITE SetDestinationChar NOTIFY DestinationCharChanged)
    Q_PROPERTY(QString ArrivingTip READ GetArrivingTip WRITE SetArrivingTip NOTIFY ArrivingTipChanged)
    Q_PROPERTY(QString DestinationText READ GetDestinationText WRITE SetDestinationText NOTIFY DestinationTextChanged)
public:
    NavigationListWidgetImpl();
    virtual ~NavigationListWidgetImpl();

private:
    // override Widget interface
    void Hide();
    nkui::WidgetID GetWidgetID();
    void SetPresenter(nkui::NKUIPresenter *presenter);
    void Show();

    // override WidgetBase
    virtual QWidget* GetNativeView();

    // implement NavigationListWidget
    virtual void UpdateManeuvers(const std::vector<nkui::ManeuverListData>& maneuvers);
    virtual void UpdateTrafficEvents(const std::vector<nkui::TrafficListData>& trafficEvents);
    virtual void UpdateTitle(const std::string& hint, const std::string& roadName);
    virtual void PopManeuver();
    virtual void UpdateTripDistance(std::string distance);
    virtual void UpdateArrivalTime(std::string time);
    virtual void UpdateTraffic(char traffic);
    virtual void UpdateDistance(const char* distance, const char* unit);
    virtual void CreateNativeView(QWidget* parentView);
    virtual void OnSetPresenter();
    virtual void SetDestinationText(const std::string& destinationSide, const std::string& destinationStreet);

    QString GetTripDistance() const;
    QString GetArrivalTime() const;
    QString GetTraffic() const;
    bool GetIsArriving() const;
    QString GetDestinationChar() const;
    QString GetArrivingTip() const;
    QString GetDestinationText() const;

    void SetTripDistance(const QString& TripDistance);
    void SetArrivalTime(const QString& ArrivalTime);
    void SetTraffic(const QString& Traffic);


    QString TranslateTrafficEevent(nbnav::TrafficEvent::Type type, nbnav::TrafficEvent::TrafficSeverity severity);
    QString TranslateDelayTime(std::string delayText, std::string unit,
                               std::string delayText2, std::string unit2);
    QString TranslateTrafficSeverity(nbnav::TrafficEvent::TrafficSeverity severity);

    void SetIsArriving(const bool isArriving);
    void SetDestinationChar(const QString& destinationChar);
    void SetArrivingTip(const QString& arrivingTip);
    void SetDestinationText(const QString& destinationText);
signals:
    void TripDistanceChanged();
    void ArrivalTimeChanged();
    void TrafficChanged();
    void IsArrivingChanged();
    void DestinationCharChanged();
    void ArrivingTipChanged();
    void DestinationTextChanged();
private:
    QDeclarativeView* mNavlistViewer;
    QObject* mNavlistObject;
    QDeclarativeContext* mNavlistContext;

    ManeuverDataModel mManeuverDataModel;
    QString mTripDistance;
    QString mArrivalTime;
    QString mTraffic;
    QString mDestinationChar;
    QString mArrivingTip;
    QString mDestinationText;
    bool    mIsArriving;


    // NavigationListWidget interface
public:
    void SetListener(nkui::NavigationListWidgetListener *listener);
};
}

#endif /* _NAVIGATION_LISTWIDGET_H_ */


/*! @} */
