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

#include "util.h"
#include "inifile.h"
#include "globalsetting.h"
#include "servertoken.h"
#include "mapwidget.h"
#include "ltkhybridmanager.h"
#include "ltkcontext.h"

#include <QtGlobal>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QDebug>

static locationtoolkit::LTKContext* ltkContext = NULL;
static locationtoolkit::LTKHybridManager* ltkHybridManager = NULL;

locationtoolkit::LTKContext* GetLTKContext()
{
    if( ltkContext == NULL )
    {
        float dpi = GlobalSetting::GetInstance().GetDpi();
        QString token = GlobalSetting::GetInstance().token();
        QString language = GlobalSetting::GetInstance().language();
        qDebug()<<"GetLTKContext "<<language;
        ltkContext = locationtoolkit::LTKContext::CreateInstance( token, language, dpi );
    }

    return ltkContext;
}

void ReleaseLTKContext( locationtoolkit::LTKContext* context )
{
    if( context != NULL )
    {
        delete context;
    }
}

locationtoolkit::LTKHybridManager* GetLTKHybridManager()
{
    if( ltkHybridManager == NULL)
    {
        ltkHybridManager = locationtoolkit::LTKHybridManager::CreateInstance(*GetLTKContext());

    }
    return ltkHybridManager;
}

void ReleaseLTKHybridManager()
{
    if( ltkHybridManager != NULL )
    {
        delete ltkHybridManager;
    }
}

locationtoolkit::MapWidget* GetMapWidget()
{
    locationtoolkit::LTKContext* context = GetLTKContext();
    if( context == NULL )
    {
        qDebug( "create ltk context failed!!!\n" );
        return NULL;
    }

    locationtoolkit::MapOptions mapOption;
    mapOption.mEnableFullScreenAntiAliasing = GlobalSetting::GetInstance().IsAntiAliasingEnabled();
    mapOption.mEnableAnisotropicFiltering = GlobalSetting::GetInstance().IsAnisotropicEnabled();
    mapOption.mMapviewOnly = GlobalSetting::GetInstance().MapviewOnly();
    mapOption.mEnableSSAO = GlobalSetting::GetInstance().EnableSSAO();
    mapOption.mEnableGlowRoute = GlobalSetting::GetInstance().EnableGlowRoute();
    mapOption.mResourceFolder = LTKSampleUtil::GetResourceFolder();
    mapOption.mWorkFolder = LTKSampleUtil::GetWorkFolder();

    QDir workFolder(mapOption.mWorkFolder);
    if(!workFolder.exists())
    {
        workFolder.mkpath(mapOption.mWorkFolder);
    }
    QFile::copy(mapOption.mResourceFolder + "on_board_metadata_source", mapOption.mWorkFolder + "on_board_metadata_source");


    locationtoolkit::MapWidget* mapWidget = new locationtoolkit::MapWidget();
    mapWidget->Initialize( mapOption,  *context );

    return mapWidget;
}


void InitLTKHybridManager(bool enable, const QString& path)
{
    locationtoolkit::LTKHybridManager* hybridManager = GetLTKHybridManager();
    hybridManager->EnableOnboard(enable);
    QDir rootDir(path);
    QStringList subDir = rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    hybridManager->BeginLoadMapData();
    for(int i=0; i<subDir.size(); ++i){
        QString subFolder = path + "/" + subDir[i];
        hybridManager->AddMapDataPath(subFolder);
    }
    hybridManager->EndLoadMapData();
}

QString LTKSampleUtil::workFolder = ".";

QString LTKSampleUtil::GetResourceFolder()
{
    //QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    return workFolder + "/resource/";
}

void LTKSampleUtil::SetWorkFolder(QString path)
{
    workFolder = path;
}

QString LTKSampleUtil::GetWorkFolder()
{
    //QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    return workFolder + "/";
}

QJsonDocument LTKSampleUtil::loadJsonDocumentFromFile(QString fileName)
{
    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    return QJsonDocument().fromJson(jsonFile.readAll());
}

void LTKSampleUtil::saveJsonDocumentToFile(QJsonDocument document, QString fileName)
{
    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(document.toJson());
}

QString getMapDataPath()
{
    QString qs = QDir::homePath();
    qs += "/navdata/mapdata";
    qDebug() << qs;
    return qs;
}
QString LTKSampleUtil::FormatDistance(double dist)
{
    QString buffer;
    bool yard = false;
    bool metric;
    QSettings settings(GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    settings.beginGroup("AppSettings");
    QString distanceType = settings.value(gDistanceType).toString();

    if(distanceType == "kms")
        metric = true;
    else
        metric = false;
    settings.endGroup();

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
