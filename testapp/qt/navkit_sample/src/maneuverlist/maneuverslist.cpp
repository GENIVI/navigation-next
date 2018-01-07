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

#include "maneuverlist/maneuverslist.h"
#include "ui_maneuverlist.h"
#include "maneuver.h"
#include "private/manlistdelegate.h"
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>

#define     ManeuverIDRole     Qt::UserRole + 1//custom role

using namespace locationtoolkit;

ManeuversList::ManeuversList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManeuverList),
    mIsMetric(false),
    mIsYard(false)
{
    ui->setupUi(this);

    QString fontFilePath = QFileInfo( QCoreApplication::applicationFilePath() ).dir().absolutePath() + "/resource/guidancefont.ttf";
    int appFontId = QFontDatabase::addApplicationFont(fontFilePath);
    if(appFontId < 0)
    {
        mManeuverFontName = "MS Shell Dlg2";
    }
    else
    {
        mManeuverFontName = QFontDatabase::applicationFontFamilies(appFontId).at(0);
    }


    MainWindow* mw = (MainWindow *)(parent);
    QObject::connect( this, SIGNAL(ShowMessageBox(QString, MessageType)), mw, SLOT(OnShowMessageBox(QString, MessageType)) );
    QObject::connect( this, SIGNAL(ShowWidget(int)), mw, SLOT(OnShowWidget(int)) );
    QObject::connect( this, SIGNAL(PlayAnounce(int)), mw, SIGNAL(PlayAnounce(int)) );
    QObject::connect( this, SIGNAL(StopNavigation()), mw, SLOT(OnStopNavigation()) );

    QObject::connect( mw, SIGNAL(UpdateMeasurmentOption(Preferences::Measurement)), this, SLOT(OnUpdateMeasurmentOption(Preferences::Measurement)) );
}

ManeuversList::~ManeuversList()
{
    delete ui;
}

void ManeuversList::on_Backbutton_clicked()
{
   emit ShowMessageBox("Stop navigation session?", MT_CancelNavigation);
}

void ManeuversList::on_listWidget_pressed(const QModelIndex &index)
{
    QListWidgetItem* curItem = ui->listWidget->currentItem();
    qint32 maneuverID = curItem->data(ManeuverIDRole).toInt();
    emit PlayAnounce(maneuverID);
}

void ManeuversList::OnRouteReceived()
{
}

void ManeuversList::OnTripRemainingTime(quint32 time)
{
    QString convertedTime = " remaining time: ";
    convertedTime.append(FormatNavTime(time));
    ui->label_RemainTime->setText(convertedTime);
}

void ManeuversList::OnTripRemainingDistance(qreal distance)
{
    QString dist = " Distance: ";
    dist.append(FormatDistance(distance, mIsMetric, mIsYard));
    ui->label_Distance->setText(dist);
}

void ManeuversList::OnManeuverRemainingDistance(qreal distance)
{
     QListWidgetItem *item = ui->listWidget->item(0);
     item->setData(Qt::DecorationRole,FormatDistance(distance, mIsMetric, mIsYard));
}

 void ManeuversList::OnUpdateManeuverList(const ManeuverList& maneuverlist)
 {
    ui->listWidget->clear();
    quint32 maneuverCount = maneuverlist.GetNumberOfManeuvers();
    for(int i = 0; i < (int)maneuverCount; ++i)
    {
        QListWidgetItem* item = new QListWidgetItem();
        const Maneuver* maneuver = maneuverlist.GetManeuver(i);

        if(i!=(maneuverCount - 1))
        {
            item->setData(Qt::DisplayRole, maneuver->GetPrimaryStreet());
        }
        else
        {
            item->setData(Qt::DisplayRole, "You have arrived your destination");
        }
        item->setData(ManeuverIDRole, maneuver->GetManeuverID());
        QString cmd = maneuver->GetRoutingTTF();
        item->setData(Qt::UserRole, cmd);

        item->setData(Qt::DecorationRole, FormatDistance(maneuver->GetDistance(), mIsMetric, mIsYard));
        ui->listWidget->addItem(item);
    }
    ManListDelegate* delegate = new ManListDelegate(ui->listWidget);
    delegate->SetFontName(mManeuverFontName);
    ui->listWidget->setItemDelegate(delegate);
 }

void ManeuversList::OnUpdateMeasurmentOption(Preferences::Measurement measurement)
{
    switch (measurement)
    {
        case Preferences::Metric:
            mIsMetric = true;
            mIsYard = false;
            break;
        case Preferences::NonMetric:
            mIsMetric = false;
            mIsYard = false;
            break;
        case Preferences::NonMetricYards:
            mIsMetric = false;
            mIsYard = true;
            break;
        default:
            break;
    }
}

QString ManeuversList::FormatNavTime(quint32 time)
{
        QString buffer;

        quint32 h = 0;
        quint32 m = 0;
        quint32 s = 0;

        if (time < 3600)
        {
            h = 0;
            m = time / 60;
        }
        else
        {
            h = time / 3600;
            m = (time % 3600)/60;
            //m = (time / 60) - (h * 60);
        }

        s = time % 60;
        if (s > 30 && m > 0)
        {
            m++;
            s = 0;
            //We should avoid "% h 60 min", so adding additional check
            if( m == 60)
            {
                m = 0;
                h++;
            }
        }

        QString tmp;
        if(h>0)
        {
            buffer.append(tmp.setNum(h) + " h ");
            if(m >0)
            {
                buffer.append(tmp.setNum(m) + " m ");
            }
            if(s>0)
            {
                buffer.append(tmp.setNum(s) + " s");
            }
        }
        else if(m >0)
        {
            buffer.append(tmp.setNum(m) + " m ");
            if(s>0)
            {
                buffer.append(tmp.setNum(s) + " s");
            }
        }
        else if(s > 0)
        {
            buffer.append(tmp.setNum(s) + " s");
        }
        else
        {
            buffer.append("0 s");
        }
        return buffer;
    }


QString ManeuversList::FormatDistance(double dist, bool metric, bool yard)
{
    QString buffer;
    double distance = dist, roundIncrement = 0.0;

    if(metric)
    {
        /*! KM */
        distance = dist/1000.0;
        if(distance >= 1)
        {
            buffer.setNum(distance,'f',1);
            buffer.append(" km ");
        }
        else
        {
            roundIncrement = 20;
            distance = roundIncrement * (int) ((dist + roundIncrement/2) / roundIncrement );
            buffer.setNum(distance);
            buffer.append(" m ");
        }
    }
    else
    {
        /*! Mile */
        distance = dist/1609.0;
        if(distance < 0.189204545)
        {
            roundIncrement = 50;
            if(yard)
            {
                distance = dist * 1.0936; // yards
            }
            else
            {
                distance = dist * 3.2808; // feet
            }
            distance = roundIncrement * (int) ((distance + roundIncrement/2) / roundIncrement );
            if(yard)
            {
                buffer.setNum(distance);
                buffer.append(" yd ");
            }
            else
            {
                buffer.setNum(distance);
                buffer.append(" ft ");
            }
        }
        else
        {
            buffer.setNum(distance,'f',1);
            buffer.append(" mi ");
        }
    }
    return buffer;
}
