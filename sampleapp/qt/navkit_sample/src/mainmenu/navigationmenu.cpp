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

#include "mainmenu/navigationmenu.h"
#include <QDir>
#include <QMessageBox>
#include "locationconfiguration.h"
#include "locationprovider.h"
#include "ui_navigationmenu.h"
#include "globalsetting.h"
#include "util.h"

using namespace locationtoolkit;

NavigationMenu::NavigationMenu(QWidget *parent)
    :QWidget(parent),
     mCurrentAvoid(0),
     mSelectedServerIndex(0),
     ui(new Ui::NavigationMenu)
{
    ui->setupUi(this);
    MainWindow* mw = static_cast<MainWindow *>(parent);
    connect( this, SIGNAL(StartNavigation(const Place&, const RouteOptions&, const Preferences&)), mw, SLOT(OnStartNavigation(const Place&, const RouteOptions&, const Preferences&)) );
    connect( this, SIGNAL(UpdateGpsFile(QString)), mw, SIGNAL(UpdateGpsFile(QString)) );
    connect( this, SIGNAL(UpdateMeasurmentOption(Preferences::Measurement)), mw, SIGNAL(UpdateMeasurmentOption(Preferences::Measurement)) );
    connect( this, SIGNAL(EnableTTS(bool)), mw, SIGNAL(EnableTTS(bool)) );
    connect( this, SIGNAL(ShowMessageBox(QString, MessageType)), mw, SLOT(OnShowMessageBox(QString, MessageType)) );

    Initialize();
}

NavigationMenu::~NavigationMenu()
{
    delete ui;
}

void NavigationMenu::LocationUpdated( const Location& location )
{
    Coordinates coord;
    coord.latitude = location.latitude;
    coord.longitude = location.longitude;
    mPolyPoints.append(coord);
}

void NavigationMenu::ProviderStateChanged( LocationProviderState /*state*/ )
{

}

void NavigationMenu::OnLocationError(int /*errorCode*/)
{
    mPolylineReady = true;
}

void NavigationMenu::OnTimer()
{
    on_StartButton_clicked();
}

void NavigationMenu::on_StartButton_clicked()
{
    if(ui->comboBox_GPSFile->count() == 0)
    {
        emit ShowMessageBox("Gps file is NULL! ", MT_Normal);
    }
    else if((!ui->lineEdit_destcoord_lat->text().isEmpty())
            &&(!ui->lineEdit_destcoord_lon->text().isEmpty())
            &&(!ui->lineEdit_origincoord_lat->text().isEmpty())
            &&(!ui->lineEdit_origincoord_lon->text().isEmpty()))
    {
        SetStartButtonDisabled(true);
        RouteOptions* routeOption = new RouteOptions(mCurrentRouteType,mCurrentVehicleType,mCurrentAvoid);

        MapLocation maplocation;
        Place place;
        maplocation.center.latitude  = ui->lineEdit_destcoord_lat->text().toDouble();
        maplocation.center.longitude = ui->lineEdit_destcoord_lon->text().toDouble();
        place.SetLocation(maplocation);

        Preferences preference;
        preference.SetMeasurement(mCurrentMeasurmentOption);
        preference.SetOffRouteIgnoreCount(mCurrentOffRouteSensitivity);

        if(mCurrentVoiceGuide == "Regular")
        {
            preference.SetLaneGuidance(false);
            preference.SetNaturalGuidance(false);
        }
        else if(mCurrentVoiceGuide == "Natural Guidance")
        {
            preference.SetNaturalGuidance(true);
        }
        else if(mCurrentVoiceGuide == "Lane Guidance")
        {
            preference.SetLaneGuidance(true);
        }
        else if(mCurrentVoiceGuide == "Natural and Lane Guidance")
        {
            preference.SetLaneGuidance(true);
            preference.SetNaturalGuidance(true);
        }
        else
        {
            preference.SetLaneGuidance(false);
            preference.SetNaturalGuidance(false);
        }

        emit StartNavigation(place, *routeOption, preference);
    }
    else
    {
        emit ShowMessageBox("Please Check the settings\n of origin or destination.", MT_Normal);
    }
}

void NavigationMenu::on_comboBox_GPSFile_currentIndexChanged(const QString &arg1)
{
    mCurrentGPSFile = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + arg1;

    LocationConfiguration locationConfiguration;
    locationConfiguration.emulationMode = true;
    locationConfiguration.locationFilename = mCurrentGPSFile;
    LocationProvider& locProvider = LocationProvider::GetInstance( locationConfiguration );

    if(mLastGPGFile != mCurrentGPSFile)
    {
        mLastGPGFile = mCurrentGPSFile;
        mPolylineReady = false;
        mPolyPoints.clear();
        while(!mPolylineReady)
        {
            locProvider.GetOneFix( static_cast<LocationListener&>(*this), Location::LFT_Normal);
        }
        QString tmp;
        ui->lineEdit_origincoord_lat->setText(tmp.setNum(mPolyPoints.first().latitude,'g',10));
        ui->lineEdit_origincoord_lon->setText(tmp.setNum(mPolyPoints.first().longitude,'g',10));
        ui->lineEdit_destcoord_lat->setText(tmp.setNum(mPolyPoints.last().latitude,'g',10));
        ui->lineEdit_destcoord_lon->setText(tmp.setNum(mPolyPoints.last().longitude,'g',10));
    }

    emit UpdateGpsFile(mCurrentGPSFile);
}

void NavigationMenu::on_comboBox_voiceGuid_currentIndexChanged(const QString &arg1)
{
    mCurrentVoiceGuide = arg1;
}

void NavigationMenu::on_comboBox_vehicleType_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "Car")
    {
        mCurrentVehicleType = TM_Car;
    }
    else if(arg1 == "Truck")
    {
        mCurrentVehicleType = TM_Truck;
    }
    else if(arg1 == "Bicycle")
    {
        mCurrentVehicleType = TM_Bicycle;
    }
    else if(arg1 == "Pedestrian")
    {
        mCurrentVehicleType = TM_Pedestrian;
    }
    else
    {
        mCurrentVehicleType = TM_Car;
    }
}

void NavigationMenu::on_comboBox_measurmentOption_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "metric")
    {
        mCurrentMeasurmentOption = Preferences::Metric;
    }
    else if(arg1 == "non_metric")
    {
        mCurrentMeasurmentOption = Preferences::NonMetric;
    }
    else if(arg1 == "non_metric_yard")
    {
        mCurrentMeasurmentOption = Preferences::NonMetricYards;
    }
    else
    {
        mCurrentMeasurmentOption = Preferences::NonMetric;
    }
    emit UpdateMeasurmentOption(mCurrentMeasurmentOption);
}

void NavigationMenu::on_comboBox_routeType_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "Fastest")
    {
        mCurrentRouteType = RT_Fastest;
    }
    else if(arg1 == "Shortest")
    {
        mCurrentRouteType = RT_Shortest;
    }
    else if(arg1 == "Easiest")
    {
        mCurrentRouteType = RT_Easiest;
    }
    else
    {
        mCurrentRouteType = RT_Fastest;
    }
}

void NavigationMenu::on_comboBox_offRouteSens_currentIndexChanged(const QString &arg1)
{
    mCurrentOffRouteSensitivity = arg1.toUInt();
}


void NavigationMenu::on_checkBox_tollroads_clicked(bool checked)
{
    if(checked)
    {
        mCurrentAvoid = mCurrentAvoid | RA_Toll;
    }
    else
    {
        mCurrentAvoid = mCurrentAvoid & (~RA_Toll);
    }
}

void NavigationMenu::on_checkBox_highways_clicked(bool checked)
{
    if(checked)
    {
        mCurrentAvoid = mCurrentAvoid | RA_Highway;
    }
    else
    {
        mCurrentAvoid = mCurrentAvoid & (~RA_Highway);
    }
}

void NavigationMenu::on_checkBox_hovlanes_clicked(bool checked)
{
    if(checked)
    {
        mCurrentAvoid = mCurrentAvoid | RA_HOV;
    }
    else
    {
        mCurrentAvoid = mCurrentAvoid & (~RA_HOV);
    }
}

void NavigationMenu::on_checkBox_ferry_clicked(bool checked)
{
    if(checked)
    {
        mCurrentAvoid = mCurrentAvoid | RA_Ferry;
    }
    else
    {
        mCurrentAvoid = mCurrentAvoid & (~RA_Ferry);
    }
}


void NavigationMenu::on_checkBox_TTS_clicked(bool checked)
{
    emit EnableTTS(checked);
}


/*******************************************************************************************************/
void NavigationMenu::Initialize()
{
    ui->comboBox_routeType->addItem("Fastest");
    ui->comboBox_routeType->addItem("Shortest");
    ui->comboBox_routeType->addItem("Easiest");

    ui->comboBox_vehicleType->addItem("Car");
    //ui->comboBox_vehicleType->addItem("Truck");
    //ui->comboBox_vehicleType->addItem("Bicycle");
    //ui->comboBox_vehicleType->addItem("Pedestrian");

    ui->checkBox_tollroads->setChecked(false);
    ui->checkBox_highways->setChecked(false);
    ui->checkBox_hovlanes->setChecked(false);
    ui->checkBox_ferry->setChecked(false);

    ui->comboBox_voiceGuid->addItem("Regular");

    ui->comboBox_offRouteSens->addItem("1");
    ui->comboBox_offRouteSens->addItem("2");

    ui->comboBox_measurmentOption->addItem("metric");
    ui->comboBox_measurmentOption->addItem("non_metric");
    ui->comboBox_measurmentOption->addItem("non_metric_yard");
    ui->comboBox_measurmentOption->setCurrentIndex(1);

    // set the limit to the input value
    QRegExp rx1("^-?(90|[1-8]?\\d(\\.\\d+)?)$");
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);

    ui->lineEdit_origincoord_lat->setValidator( pReg1 );
    ui->lineEdit_destcoord_lat->setValidator( pReg1 );

    QRegExp rx2("^-?(180|1?[0-7]?\\d(\\.\\d+)?)$");
    QRegExpValidator *pReg2 = new QRegExpValidator(rx2, this);
    ui->lineEdit_origincoord_lon->setValidator( pReg2 );
    ui->lineEdit_destcoord_lon->setValidator( pReg2 );

    InitGPSFileList();
}

void NavigationMenu::InitGPSFileList()
{
    QStringList filter;
    filter << "*.gps";
    QDir dirGPS( LTKSampleUtil::GetResourceFolder() + "gpsfiles" );
    QStringList names = dirGPS.entryList( filter );

    int defaultIndex = 0;
    for( int i = 0; i < names.size(); i++ )
    {
        if( names[i] == "LA_staples_center.gps" )
        {
            defaultIndex = i;
            break;
        }
    }

    ui->comboBox_GPSFile->addItems(names);
    ui->comboBox_GPSFile->setCurrentIndex( defaultIndex );
    if(!names.empty())
    {
        mCurrentGPSFile = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + names[defaultIndex];
    }
    else
    {
        mCurrentGPSFile = LTKSampleUtil::GetResourceFolder();
    }
}

void NavigationMenu::SetStartButtonDisabled(bool isDisabled)
{
    ui->StartButton->setDisabled(isDisabled);
}
