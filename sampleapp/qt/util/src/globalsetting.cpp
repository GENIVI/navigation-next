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

#include "globalsetting.h"
#include "util.h"
#include "servertoken.h"
#include <QSettings>
#include <string>

//#include <QMessageBox>
#include <QDebug>
#include <QSettings>

const static QString Organization = "TCS";
const static QString Application = "Sample";
const static QString MapDataPath = "onBoardDataPath";

GlobalSetting GlobalSetting::mInstance;

GlobalSetting::GlobalSetting() :
    mDpi( 200.0 ),
    mAntiAliasingEnabled( false ),
    mAnisotropicEnabled( false ),
    mMapviewOnly(false),
    mEnableSSAO(false),
    mToken(""),
    mLanguage("en-US")
{
}

void GlobalSetting::InitInstance()
{
    // read ini file
    QString resourceFolder = LTKSampleUtil::GetResourceFolder();
    QString strConfig = resourceFolder + "sampleapp.ini";
    qDebug()<<"Load configuration file:"<<strConfig;

    QSettings *configFile = new QSettings(strConfig, QSettings::IniFormat);
    if(configFile)
    {
        mInstance.mDpi = configFile->value("display/dpi", "258").toInt();
        mInstance.mAntiAliasingEnabled = configFile->value("display/enableAntiAliasing", "true").toBool();
        mInstance.mAnisotropicEnabled = configFile->value("display/enableAnisotropicFiltering", "true").toBool();
        mInstance.mMapviewOnly =configFile->value("display/mapviewOnly", "true").toBool();
        mInstance.mEnableSSAO = configFile->value("display/enableSSAO", "false").toBool();
        mInstance.mEnableGlowRoute = configFile->value("display/enableGlowRoute", "false").toBool();
        mInstance.SetMapDataPath(configFile->value("hybrid/onBoardDataPath", "").toString());
        mInstance.setToken(configFile->value("hybrid/token", TOKEN).toString());
        mInstance.setLanguage(configFile->value("AppSettings/language", "en-US").toString());
        mInstance.setNightMode(configFile->value("display/nightMode", "auto").toString());

        delete configFile;
    }
    else
    {
//        QMessageBox box( QMessageBox::Critical, "system error", "open sampleapp.ini failed" );
//        box.exec();
    }
}

GlobalSetting& GlobalSetting::GetInstance()
{
    return mInstance;
}

float GlobalSetting::GetDpi() const
{
    return mDpi;
}

bool GlobalSetting::IsAntiAliasingEnabled() const
{
    return mAntiAliasingEnabled;
}

bool GlobalSetting::IsAnisotropicEnabled() const
{
    return mAnisotropicEnabled;
}

bool GlobalSetting::MapviewOnly() const
{
    return mMapviewOnly;
}

bool GlobalSetting::EnableSSAO() const
{
    return mEnableSSAO;
}

bool GlobalSetting::EnableGlowRoute() const
{
    return mEnableGlowRoute;
}
QString GlobalSetting::GetMapDataPath() const
{
    QSettings settings(Organization, Application);
    QString str = settings.value(MapDataPath).toString();
    return str;
}

void GlobalSetting::SetMapDataPath(const QString& str) const
{
    QSettings settings(Organization, Application);
    settings.setValue(MapDataPath, str);
    settings.sync();
}

QString GlobalSetting::token() const
{
    return mToken;
}

void GlobalSetting::setToken(const QString& str)
{
    mToken = str;
}

QString GlobalSetting::language() const
{
    return mLanguage;
}

void GlobalSetting::setLanguage(const QString& str)
{
    mLanguage = str;
}

locationtoolkit::MapWidget::NightMode GlobalSetting::nightMode() const
{
    return mNightMode;
}

void GlobalSetting::setNightMode(const QString& str)
{
    if(QString::compare(str, "night", Qt::CaseInsensitive) == 0)
    {
        mNightMode = locationtoolkit::MapWidget::NM_NIGHT;
    }
    else if(QString::compare(str, "day", Qt::CaseInsensitive) == 0)
    {
        mNightMode = locationtoolkit::MapWidget::NM_DAY;
    }
    else
    {
        mNightMode = locationtoolkit::MapWidget::NM_AUTO;
    }
}
