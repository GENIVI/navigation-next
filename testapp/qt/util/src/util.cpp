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
#include <QDir>
#include <QCoreApplication>
#include <QSettings>
#include <QDebug>

static locationtoolkit::LTKContext* ltkContext = NULL;

locationtoolkit::LTKContext* GetLTKContext()
{
    if( ltkContext == NULL )
    {
        QString server = GlobalSetting::GetInstance().GetCurrentServer();
        QString credential;
        if( !GlobalSetting::GetInstance().GetServerProperty( server, credential ) )
        {
            return NULL;
        }

        float dpi = GlobalSetting::GetInstance().GetDpi();
        ltkContext = locationtoolkit::LTKContext::CreateInstance( credential, dpi );
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

locationtoolkit::MapWidget* GetMapWidget()
{
    locationtoolkit::LTKContext* context = GetLTKContext();
    if( context == NULL )
    {
        qDebug( "create ltk context failed!!!\n" );
        return NULL;
    }

    QDir dir = QFileInfo( QCoreApplication::applicationFilePath() ).dir();
    QString strRoot = dir.absolutePath();
    locationtoolkit::MapOptions mapOption;
    mapOption.mEnableFullScreenAntiAliasing = GlobalSetting::GetInstance().IsAntiAliasingEnabled();
    mapOption.mEnableAnisotropicFiltering = GlobalSetting::GetInstance().IsAnisotropicEnabled();
    mapOption.mResourceFolder = strRoot + "/resource/";
    mapOption.mWorkFolder = strRoot + "/";
    qDebug() << mapOption.mResourceFolder;
    qDebug() << mapOption.mWorkFolder;

    locationtoolkit::MapWidget* mapWidget = new locationtoolkit::MapWidget();
    mapWidget->Initialize( mapOption,  *context );

    return mapWidget;
}

