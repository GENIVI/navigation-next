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

    @file     until.h
    @date     10/10/2014
    @defgroup MAINWINDOW_H
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef UTIL_H
#define UTIL_H
#include <QString>
#include <QtCore>

namespace locationtoolkit
{
    class LTKContext;
    class MapWidget;
    class LTKHybridManager;
}

const static QString gOrganization = "TCS";
const static QString gApplication = "NavigationSample";
const static QString gSearchRadius = "searchRadius";
const static QString gCurrentLatitude = "currentLatitude";
const static QString gCurrentLongitude = "currentLongitude";
const static QString gFuelType = "fuelType";
const static QString gShowType = "showType";
const static QString gDefaultViewType = "defaultViewType";
const static QString gRouteType = "routeType";
const static QString gAvoidType = "avoidType";
const static QString gTrafficType = "trafficType";
const static QString gDistanceType = "distanceType";
const static QString gGpsFileName = "gpsFileName";
const static QString gConfigFileName = "sampleapp.ini";
const static QString gOnBoardDataPath = "onBoardDataPath";
const static QString gLanguage = "language";



locationtoolkit::LTKContext* GetLTKContext();
locationtoolkit::MapWidget* GetMapWidget();
void InitLTKHybridManager(bool enable, const QString& path = QString());
locationtoolkit::LTKHybridManager* GetLTKHybridManager();
QString getMapDataPath();

class LTKSampleUtil
{
public:
    static QString GetResourceFolder();
    static QString GetWorkFolder();
    static QJsonDocument loadJsonDocumentFromFile(QString fileName);
    static void saveJsonDocumentToFile(QJsonDocument document, QString fileName);

    static void SetWorkFolder(QString path);
    static QString FormatDistance(double dist);


private:
    static QString workFolder;
};

#endif // UTIL_H

/*! @} */
