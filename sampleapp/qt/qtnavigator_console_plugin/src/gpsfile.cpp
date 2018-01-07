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

#include "gpsfile.h"
#include <qglobal.h>
#include <QString>
#include <QDateTime>
#include <QDebug>

typedef struct
{
    quint32             dwTimeStamp;          // Time, seconds since 1/6/1980
    quint32             status;               // Response status;
    qint32              dwLat;                // Lat, 180/2^25 degrees, WGS-84 ellipsoid
    qint32              dwLon;                // Lon, 360/2^26 degrees, WGS-84 ellipsoid
    qint16              wAltitude;            // Alt, meters, WGS-84 ellipsoid
    quint16             wHeading;             // Heading, 360/2^10 degrees
    quint16             wVelocityHor;         // Horizontal velocity, 0.25 meters/second
    qint8               bVelocityVer;         // Vertical velocity, 0.5 meters/second
    qint8               accuracy;             // Accuracy of the data
    quint16             fValid;               // Flags indicating valid fields in the struct.
    quint8              bHorUnc;              // Horizontal uncertainty
    quint8              bHorUncAngle;         // Horizontal Uncertainty at angle
    quint8              bHorUncPerp;          // Horizontal uncertainty perpendicular
    quint8              bUTCofset;            // UTC ofset
    quint16             wVerUnc;              // Vertical uncertainty.
} GPSFileRecord;

const static double Gps_Latitude_Constant                 = 0.00000536441803;
const static double Gps_Longitude_Constant                = 0.00000536441803;
const static double Gps_Heading_Constant                  = 0.3515625;
const static double Gps_Horizontal_Vel_Constant           = 0.25;
const static double Gps_Vertical_Vel_Constant             = 0.5;
const static double Gps_Horizontal_Unc_Angle_Of_Axis_Constant = 5.625;
const static double Gps_Altitude_Constant                 = 500;
const static int    Gps_Fake_Num_Of_Satellites            = 4;
const static qint64 UNIX_FILE_TIME_GAP = 315964800;

// According TIA/EIA IS-801 standard, Standard Deviation for Position Uncertainty (meters)
static const double unctbl[] = {0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 8.0,
                                12.0, 16.0, 24.0, 32.0, 48.0, 64.0, 96.0, 128.0,
                                192.0, 256.0, 384.0, 512.0, 768.0, 1024.0, 1536.0,
                                2048.0, 3072.0, 4096.0, 6144.0, 8192.0, 12288.0};

static double
uncdecode(quint8 unc)
{
    if (unc < (sizeof(unctbl) / sizeof(*unctbl)))
    {
        return (unctbl[unc]);
    }
    else
    {
        return (0.0);
    }
}

GpsFile::GpsFile()
    : m_MaxFixesCount(0)
    , m_CurrentFix(0)
{

}

GpsFile::~GpsFile()
{
    if( m_File.isOpen() )
    {
        m_File.close();
    }
}

int GpsFile::maxFixesCount()
{
    return m_MaxFixesCount;
}

bool GpsFile::initialize(const QString &path)
{
    m_File.setFileName( path );
    if( !m_File.open( QIODevice::ReadOnly ) )
    {
        return false;
    }
    m_MaxFixesCount = m_File.size()/sizeof(GPSFileRecord);

    if( m_MaxFixesCount < 1 )
    {
        return false;
    }

    return true;
}

void GpsFile::readPosition(int index, GeoLocation &location)
{
#ifdef DEBUG_GPSFILE
    _T();
#endif
    qint64 offset = index * sizeof(GPSFileRecord);
    m_File.seek( offset );

    GPSFileRecord fileRcd;
    m_File.read( (char *)&fileRcd, sizeof(GPSFileRecord) );

    qint64 timestamp = QDateTime::currentMSecsSinceEpoch() / 1000;

    location.setStatus(fileRcd.status);
    location.setGpsTime(timestamp - UNIX_FILE_TIME_GAP);
    location.setValid(fileRcd.fValid);
    location.setLatitude(fileRcd.dwLat * Gps_Latitude_Constant);
    location.setLongitude(fileRcd.dwLon * Gps_Longitude_Constant);
    location.setHeading(fileRcd.wHeading * Gps_Heading_Constant);
    location.setHorizontalVelocity(fileRcd.wVelocityHor * Gps_Horizontal_Vel_Constant);
    location.setAltitude(fileRcd.wAltitude - Gps_Altitude_Constant);
    location.setVerticalVelocity(fileRcd.bVelocityVer * Gps_Vertical_Vel_Constant);
    location.setHorizontalUncertaintyAngleOfAxis(fileRcd.bHorUncAngle * Gps_Horizontal_Unc_Angle_Of_Axis_Constant);
    location.setHorizontalUncertaintyAlongAxis(uncdecode(fileRcd.bHorUnc));
    location.setHorizontalUncertaintyAlongPerpendicular(uncdecode(fileRcd.bHorUncPerp));
    location.setVerticalUncertainty(uncdecode(fileRcd.wVerUnc));
    location.setUtcOffset(0);
    location.setNumberOfSatellites(Gps_Fake_Num_Of_Satellites);
    location.setGpsHeading(0.0f);
    location.setCompassHeading(0.0f);
    location.setCompassAccuracy(0.0f);
}

void GpsFile::reset()
{
    m_CurrentFix = 0;
}

QString GpsFile::path() const
{
    return m_File.fileName();
}

void GpsFile::nextGeoLocation(GeoLocation &location)
{
    if (m_CurrentFix >= m_MaxFixesCount)
    {
        m_CurrentFix = 0;
    }

    readPosition(m_CurrentFix, location);

    m_CurrentFix++;
}

void GpsFile::readFirstGeoLocation(GeoLocation &loc)
{
    readPosition(0, loc);
}

void GpsFile::readLastGeoLocation(GeoLocation &loc)
{
    readPosition(m_MaxFixesCount - 1, loc);
}
