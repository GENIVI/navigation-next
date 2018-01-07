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
#include "inifile.h"
#include "serverconfig.h"
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <string>

#include <QMessageBox>
#include <QDebug>

const QString GlobalSetting::mSettingsOrganizationName = "Telesyscom";
const QString GlobalSetting::mSettingsApplicationName = "ltk-qt";
const QString GlobalSetting::mSettingsServerKey = "server";
GlobalSetting GlobalSetting::mInstance;

GlobalSetting::GlobalSetting() :
    mDpi( 200.0 ),
    mAntiAliasingEnabled( false ),
    mAnisotropicEnabled( false )
{
}

void GlobalSetting::InitInstance()
{
    // read current server
    QSettings settings( mSettingsOrganizationName, mSettingsApplicationName );
    QVariant var = settings.value( mSettingsServerKey );
    if( !var.isNull() )
    {
        mInstance.mServer = var.toString();
    }

    // read available sever list
    size_t size = sizeof(ServerConfigurations) / sizeof(const char *);
    for( size_t i = 0; i < size; i += 2 )
    {
        mInstance.mServerList.push_back( ServerConfigurations[i] );
    }

    // read ini file
    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    QString strRoot = dir.absolutePath();
    QString strConfig = strRoot + "/resource/sampleapp.ini";
    std::string strConfigIni = strConfig.toStdString();
    qDebug() << strConfigIni.c_str();

    if( IniFile::Instance().open( strConfigIni.c_str() ) )
    {
        std::string sdpi = IniFile::Instance().read("display", "dpi", "200.0");
        QString qdpi = QString::fromStdString(sdpi);
        mInstance.mDpi = qdpi.toFloat();

        std::string sEnableAntiAliasing = IniFile::Instance().read("display", "enableAntiAliasing", "true");
        mInstance.mAntiAliasingEnabled = sEnableAntiAliasing == "true" ? true : false;

        std::string sEnableAnisotropic = IniFile::Instance().read("display", "enableAnisotropicFiltering", "true");
        mInstance.mAnisotropicEnabled = sEnableAnisotropic == "true" ? true : false;
    }
    else
    {
        QMessageBox box( QMessageBox::Critical, "system error", "open sampleapp.ini failed" );
        box.exec();
    }
}

GlobalSetting& GlobalSetting::GetInstance()
{
    return mInstance;
}

const float GlobalSetting::GetDpi() const
{
    return mDpi;
}

const bool GlobalSetting::IsAntiAliasingEnabled() const
{
    return mAntiAliasingEnabled;
}

const bool GlobalSetting::IsAnisotropicEnabled() const
{
    return mAnisotropicEnabled;
}

const QString GlobalSetting::GetCurrentServer() const
{
    return mServer;
}

void GlobalSetting::SetCurrentServer( QString server )
{
    mServer = server;
    QSettings settings( mSettingsOrganizationName, mSettingsApplicationName );
    settings.setValue( mSettingsServerKey, server );
}

QStringList GlobalSetting::GetServerList() const
{
    return mServerList;
}

bool GlobalSetting::GetServerProperty( const QString& server, QString& credential ) const
{
    bool ret = false;

    size_t size = sizeof(ServerConfigurations) / sizeof(const char *);
    for( size_t i = 0; i < size; i += 2 )
    {
        if( server == ServerConfigurations[i] )
        {
            credential = ServerConfigurations[ i+1 ];
            ret = true;
            break;
        }
    }
    return ret;
}
