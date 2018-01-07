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

#ifndef GLOBALSETTING_H
#define GLOBALSETTING_H

#include <QStringList>

class GlobalSetting
{
public:
    static void InitInstance();
    static GlobalSetting& GetInstance();

    const float GetDpi() const;
    const bool IsAntiAliasingEnabled() const;
    const bool IsAnisotropicEnabled() const;

    const QString GetCurrentServer() const;
    void SetCurrentServer( QString server );
    QStringList GetServerList() const;
    bool GetServerProperty( const QString& server, QString& credential ) const;

private:
    GlobalSetting();

private:
    static GlobalSetting mInstance;
    float mDpi;
    bool mAntiAliasingEnabled;
    bool mAnisotropicEnabled;
    QString mServer;
    QStringList mServerList;

    static const QString mSettingsOrganizationName;
    static const QString mSettingsApplicationName;
    static const QString mSettingsServerKey;
};

#endif // GLOBALSETTING_H
