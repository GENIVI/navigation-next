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

    @file     realgpsprovider.cpp
    @date     03/27/2012
    @defgroup REALGPSPROVIDER_H Real Gps

    @brief    Real Gps

*/
/*
    See file description in header file.

    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "realgpsprovider.h"
#include "palclock.h"
#include "palstdlib.h"
#include "paltestlog.h"
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "pal.h"
#include "abpalgpsutils.h"

enum
{
    DESTROY_REQUEST = 0,
    START_REQUEST,
    CANCEL_REQUEST,
    UPDATE_REQUEST,
    BACKGROUND_GEOLOCATION_ON,
    BACKGROUND_GEOLOCATION_OFF
};

#define INVALID_LATLON          (-999.0)
#define GPS_TIMEOUT        5000 // ms
#define ALL_VALID_VALUE (PGV_Latitude | PGV_Longitude | PGV_Altitude | PGV_Heading \
    | PGV_HorizontalVelocity | PGV_HorizontalUncertainty | PGV_AxisUncertainty \
    | PGV_PerpendicularUncertainty | PGV_VerticalUncertainty | PGV_UTCOffset | PGV_SatelliteCount)

#define GPS_VALID_BITS_LOG_INTERVAL (30 * 1000)

RealGpsProvider* RealGpsProvider::m_pRealGpsProvider = NULL;
uint32 RealGpsProvider::m_Ref = 0;
pthread_mutex_t RealGpsProvider::m_lock = PTHREAD_MUTEX_INITIALIZER;

RealGpsProvider::RealGpsProvider(PAL_Instance* pal):
    GpsProvider(pal),
    m_pPal(pal),
    m_backgroundEventsEnabled(false),
    m_logLocationAllowed(false),
    m_bThreadWorking(false),
    m_threadId(0),
    m_serialHandle(-1)
{
    m_gpsdevicefd = -1;
    m_gpsinfonotifyflag = 0;
    m_gpscmdvalidcount = 0;

    memset(&m_gpsdatabuf, 0, gpsdatabufsize);
    memset(&m_gpsLocation, 0, sizeof(m_gpsLocation));
    memset(&m_oldserialoptions, 0, sizeof(struct termios));
}

RealGpsProvider* RealGpsProvider::GetInstantce(PAL_Instance* pal)
{
    if (m_pRealGpsProvider == NULL)
    {
        pthread_mutex_lock(&m_lock);
        if (m_pRealGpsProvider == NULL)
        {
            m_pRealGpsProvider = new RealGpsProvider(pal);
            if (m_pRealGpsProvider)
            {
                m_Ref = 1;
            }
        }
        pthread_mutex_unlock(&m_lock);
    }
    else
    {
        m_Ref += 1;
    }

    return m_pRealGpsProvider;
}

void RealGpsProvider::RemoveInstance()
{
    if (--m_Ref == 0)
    {
        delete this;
        m_pRealGpsProvider = NULL;
    }
}

RealGpsProvider::~RealGpsProvider()
{

}

PAL_Error RealGpsProvider::Initialize(const ABPAL_GpsConfig* gpsConfig, uint32 configCount)
{
    int ret = -1;
    int speed = 0;
    int inputbps = 0;
    char *serialbps = NULL;
    char *gpsdevicefile = NULL;
    struct termios newserialoptions;

    //open serial port and set up the parameters for the serial port.
    //assert(gpsConfig);

    if (gpsConfig == NULL){
        return PAL_ErrBadParam;
    }

    memset(&newserialoptions, 0, sizeof(struct termios));
    
    //serialPort = GetGpsConfigValue(gpsConfig, configCount, "ComPort");
    gpsdevicefile = GetGpsConfigValue(gpsConfig,configCount, "ComPort");
    serialbps = GetGpsConfigValue(gpsConfig,configCount, "serialbps");

    if ((gpsdevicefile == NULL) || (serialbps == NULL)){
        return PAL_ErrNoInit;
    }

    inputbps = atoi(serialbps);

    m_gpsdevicefd = open(gpsdevicefile,O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_gpsdevicefd == -1){
        return PAL_ErrNoInit;
    }

    fcntl(m_gpsdevicefd, F_SETFL, 0);

    ret = tcgetattr(m_gpsdevicefd, &m_oldserialoptions);
    if (ret== -1){
        close(m_gpsdevicefd);
        m_gpsdevicefd = -1;
        return PAL_ErrNoInit;
    }

    tcflush(m_gpsdevicefd,TCIOFLUSH);

    newserialoptions = m_oldserialoptions;

    cfmakeraw(&newserialoptions);
    newserialoptions.c_cflag &= ~CSIZE;

    switch (inputbps)
    {
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 115200:
        speed = B115200;
        break;
    default:
        speed = B115200;
        break;

    }

    cfsetispeed(&newserialoptions, speed);
    cfsetospeed(&newserialoptions, speed);

    newserialoptions.c_cflag |= CS8;
    newserialoptions.c_cflag &= ~CSTOPB;

    newserialoptions.c_cflag &= ~PARENB;
    newserialoptions.c_iflag &= ~INPCK;

    newserialoptions.c_cc[VMIN] = 10;
    newserialoptions.c_cc[VTIME] = 5;

    tcflush(m_gpsdevicefd,TCIFLUSH);
    ret = tcsetattr(m_gpsdevicefd, TCSANOW,&newserialoptions);
    if (ret == -1){
        close(m_gpsdevicefd);
        m_gpsdevicefd = -1;
        return PAL_ErrNoInit;
    }

    return PAL_Ok;
}

bool RealGpsProvider::logLocationAllowed(const ABPAL_GpsConfig* config, uint32 configCount)
{
    for (uint32 i = 0; i < configCount; i++)
    {
        if (nsl_strcmp(config[i].name, "logLocationAllowed") != 0)
        {
            continue;
        }

        if ( nsl_strcmp(config[i].value, "true") == 0 )
            return true;
        else
            return false;
    }

    return false;
}

void RealGpsProvider::RecordLocation(ABPAL_GpsLocation &location, ABPAL_GpsLocation &lastLocation, bool coarse)
{
    static bool lastCoarse = true;
    char buffer[1024] = "GPS:";
    int stringLength = nsl_strlen("GPS:");

    if (m_logLocationAllowed && ((location.latitude != lastLocation.latitude) || (location.longitude != lastLocation.longitude)))
    {
        stringLength += sprintf(buffer + stringLength, "(lat,lon)=%lf,%lf ", location.latitude, location.longitude);
    }

    if (location.heading != lastLocation.heading)
    {
        stringLength += sprintf(buffer + stringLength, "hd=%.0f ", location.heading);
    }

    if (location.horizontalVelocity != lastLocation.horizontalVelocity)
    {
        stringLength += sprintf(buffer + stringLength, "spd=%.0f ", location.horizontalVelocity);
    }

    if (location.altitude != lastLocation.altitude)
    {
        stringLength += sprintf(buffer + stringLength, "alt=%.1f ", location.altitude);
    }

    if (location.horizontalUncertaintyAlongAxis != lastLocation.horizontalUncertaintyAlongAxis)
    {
        stringLength += sprintf(buffer + stringLength, "accuracy=%.0f ", location.horizontalUncertaintyAlongAxis);
    }

    if (location.numberOfSatellites != lastLocation.numberOfSatellites)
    {
        stringLength += sprintf(buffer + stringLength, "sat=%d ", location.numberOfSatellites);
    }

    if (lastCoarse != coarse)
    {
        stringLength += sprintf(buffer + stringLength, "coarse=%d", coarse);
        lastCoarse = coarse;
    }

    if (stringLength > nsl_strlen("GPS:"))
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, buffer);
    }
}

void RealGpsProvider::start()
{
    if(0 == pthread_create(&m_threadId, NULL, RealGpsProvider::GpsWorkerThreadFunc, this))
    {
        m_bEnabled = true;
    }
}

void RealGpsProvider::updateCriteria(const ABPAL_GpsCriteria* criteria)
{
    //printf("call updateCriteria\n");
    SetGpsNotifyFlag(1);
}

void RealGpsProvider::stop()
{
    m_bThreadWorking = false;
    pthread_join(m_threadId, 0);

    if (m_gpsdevicefd > 0){
        tcflush(m_gpsdevicefd,TCIFLUSH);
        tcsetattr(m_gpsdevicefd, TCSANOW,&m_oldserialoptions);
        close(m_gpsdevicefd);
        m_gpsdevicefd = -1;
    }
}

PAL_Error RealGpsProvider::enableBackgroundGeolocation(nb_boolean isEnabled)
{
    return PAL_Ok;
}

void* RealGpsProvider::GpsWorkerThreadFunc(void* user_data)
{
    RealGpsProvider* gpsProvider = reinterpret_cast<RealGpsProvider*>(user_data);
    assert(gpsProvider);
    gpsProvider->m_bThreadWorking = true;

    while(gpsProvider->m_bThreadWorking)
    {
        //read data from serial port
        //parse data and fill the ABPAL_GpsLocation structure.
        //gpsProvider->NotifyListeners(PAL_Error error, const ABPAL_GpsLocation& location, //bool isGpsFix);

        gpsProvider->getgpsinformation();
    }
}

void RealGpsProvider::getgpsinformation(){
    int i = 0;
    int curnotifygpsflag = 0;
    PAL_Error ret = PAL_Ok;
    char c = '0';
    char buf[gpsdatabufsize] = {0};
    char tmpgpsdata[gpsdatabufsize] = {0};
    char *pgpscmd = NULL;
    volatile bool breadflag = true;
    const char *gprmckeyword = "$GPRMC";
    const char *gpggakeyword = "$GPGGA";

    if (m_gpsdevicefd == -1){
        return;
    }

    /*! read serial and parse gps data buf. */
    while ((breadflag) && (i<gpsdatabufsize)){
        usleep(10);
        if(read(m_gpsdevicefd,&c,1) < 0 ){
            return;
        }

        if (i==0){
            /*! gps data buf must start with $. */
            if (c != '$'){
                continue;
            }
        }

        buf[i++] = c;
        if (c == '\n'){
            strncpy(m_gpsdatabuf,buf,i);
            breadflag = false;
        }
    }

    strcpy(tmpgpsdata, m_gpsdatabuf);
    pgpscmd = strtok(tmpgpsdata,",");

    //curnotifygpsflag = GetGpsNotifyFlag();
    //if (curnotifygpsflag == 1){
    if (strcmp(pgpscmd,gprmckeyword) == 0){
        m_gpscmdvalidcount++;
        ret = parsegpsinfobygprmc((char *)m_gpsdatabuf, (ABPAL_GpsLocation &)m_gpsLocation);
    }else if (strcmp(pgpscmd,gpggakeyword) == 0){
        m_gpscmdvalidcount++;
        ret = parsegpsinfobygpgga((char *)m_gpsdatabuf, (ABPAL_GpsLocation &)m_gpsLocation);
    }else{
        //other gps command parse
    }

    if (m_gpscmdvalidcount == gpscmdrequiredtotal){
        NotifyListeners(ret, (ABPAL_GpsLocation &)m_gpsLocation, true);
        //curnotifygpsflag = 0;
        m_gpscmdvalidcount = 0;
        //SetGpsNotifyFlag(0);
    }
    //}
}

PAL_Error RealGpsProvider::parsegpsinfobygprmc(char* csnmea,ABPAL_GpsLocation &pgpsinfo){
    int i = 0;
    const char *pdelimc = ",";
    char *p = NULL;
    char *ptime = NULL;
    char *pdate = NULL;
    char *csgpsepoch = "19800106000000";
    uint32 ugpsepochtime = 0;
    char csgpstimefromdevice[gpsdatabufsize] = {0};
    uint32 ugpstimefromdevice = 0;
    char gpsdataarray[15][gpsdatabufsize] = {{0}};
    double curlatitude = 0.0;
    double curlongitude = 0.0;
    uint32 curvalid = 0;

    if (csnmea == NULL){

        return PAL_ErrBadParam;
    }

    p = strtok(csnmea,pdelimc);
    while ((p != NULL) && (i<15)){
        strcpy(gpsdataarray[i], p);
        i++;

        p = strtok(NULL,pdelimc);
    }

    if (strcmp(gpsdataarray[2],"A") == 0){
        pgpsinfo.status = 0;

        csgpstimefromdevice[0] = '2';
        csgpstimefromdevice[1] = '0';
        csgpstimefromdevice[2] = gpsdataarray[9][4];
        csgpstimefromdevice[3] = gpsdataarray[9][5];
        csgpstimefromdevice[4] = gpsdataarray[9][2];
        csgpstimefromdevice[5] = gpsdataarray[9][3];
        csgpstimefromdevice[6] = gpsdataarray[9][0];
        csgpstimefromdevice[7] = gpsdataarray[9][1];

        ptime = strtok(gpsdataarray[1],".");
        if (ptime != NULL){
            strcat(csgpstimefromdevice,ptime);
        }

        ConvertGpsTimeToInt((const char*)csgpsepoch, (uint32 &)ugpsepochtime);
        ConvertGpsTimeToInt((const char*)csgpstimefromdevice, (uint32 &)ugpstimefromdevice);

        //printf("csgpsepoch is %s, after convert is: %d\n", csgpsepoch,ugpsepochtime);
        //printf("csgpstimefromdevice is %s, after convert is: %d\n", csgpstimefromdevice,ugpstimefromdevice);

        pgpsinfo.gpsTime = ugpstimefromdevice - ugpsepochtime;

        curvalid |= PGV_Latitude;
        curvalid |= PGV_Latitude;
        curvalid |= PGV_Heading;
        pgpsinfo.valid = curvalid;

        curlatitude = CovertGpsLatLonToDouble((const char*)gpsdataarray[3]);
        if (strcmp(gpsdataarray[4],"N") == 0){
            pgpsinfo.latitude = curlatitude;
        }else{
            pgpsinfo.latitude = 0 - curlatitude;
        }
        printf("latitude is %s, after convert: %f\n", gpsdataarray[3],curlatitude);

        curlongitude = CovertGpsLatLonToDouble((const char*)gpsdataarray[5]);
        if (strcmp(gpsdataarray[6],"E") == 0){
            pgpsinfo.longitude = curlongitude;
        }else{
            pgpsinfo.longitude = 0 - curlongitude;
        }
        printf("longitude is %s, after convert: %f\n", gpsdataarray[5], curlongitude);

        pgpsinfo.heading = atof(gpsdataarray[8]);
        pgpsinfo.horizontalVelocity = atof(gpsdataarray[7]);

        pgpsinfo.gpsHeading = atof(gpsdataarray[8]);
    }else{
        //printf("[parsegpsinfobygprmc]PAL_ErrGpsInvalidState\n");
        pgpsinfo.status = PAL_ErrGpsInvalidState;
        pgpsinfo.valid = 0;
        return PAL_ErrGpsInvalidState;
    }

    return PAL_Ok;
}

PAL_Error RealGpsProvider::parsegpsinfobygpgga(char* csnmea,ABPAL_GpsLocation &pgpsinfo){
    int i = 0;
    const char *pdelimc = ",";
    char *p = NULL;
    char gpsdataarray[15][gpsdatabufsize] = {{0}};

    if (csnmea == NULL){

        return PAL_ErrBadParam;
    }

    p = strtok(csnmea,pdelimc);
    while ((p != NULL) && (i<15)){
        strcpy(gpsdataarray[i], p);
        i++;

        p = strtok(NULL,pdelimc);
    }

    if (strcmp(gpsdataarray[6],"0") == 0){
        pgpsinfo.status = PAL_ErrGpsInvalidState;
        pgpsinfo.valid = 0;
        return PAL_ErrGpsInvalidState;
    }else{
        pgpsinfo.valid |= PGV_Altitude;
        pgpsinfo.valid |= PGV_SatelliteCount;

        pgpsinfo.numberOfSatellites = atoi(gpsdataarray[7]);
        pgpsinfo.altitude = atof(gpsdataarray[9]);
    }

    return PAL_Ok;
}

int	RealGpsProvider::GetGpsNotifyFlag(){
    int flag = 0;

    pthread_mutex_lock(&m_lock);
    flag = m_gpsinfonotifyflag;
    pthread_mutex_unlock(&m_lock);

    return flag;
}

void RealGpsProvider::SetGpsNotifyFlag(int flag){
    pthread_mutex_lock(&m_lock);
    m_gpsinfonotifyflag = flag;
    pthread_mutex_unlock(&m_lock);
}

/*! timestring: yyyymmddhhmmss. */
PAL_Error RealGpsProvider::ConvertGpsTimeToInt(const char *timestring, uint32 &utime){
    uint32 gpstime = 0;
    int		i = 0;
    int		yearindex = 0;
    int		monthindex = 0;
    int		dayindex = 0;
    int		hourindex = 0;
    int		minuteindex = 0;
    int		secondindex = 0;
    int		len = 0;
    char	cyear[8] = {0};
    char	cmonth[8] = {0};
    char	cday[8] = {0};
    char	chour[8] = {0};
    char	cminute[8] = {0};
    char	csecond[8] = {0};

    struct tm tmGPS;

    if (timestring == NULL){
        return PAL_ErrBadParam;
    }

    len = strlen(timestring);
    for (i=0; i<len; i++){
        if (i<4){
            cyear[yearindex] = timestring[i];
            yearindex++;
        }else if (i<6){
            cmonth[monthindex] = timestring[i];
            monthindex++;
        }else if (i<8){
            cday[dayindex] = timestring[i];
            dayindex++;
        }else if (i<10){
            chour[hourindex] = timestring[i];
            hourindex++;
        }else if (i<12){
            cminute[minuteindex] = timestring[i];
            minuteindex++;
        }else if (i<14){
            csecond[secondindex] = timestring[i];
            secondindex++;
        }
    }

    memset(&tmGPS, 0, sizeof(struct tm));

    tmGPS.tm_year = atoi(cyear) - 1900;
    tmGPS.tm_mon = atoi(cmonth);
    tmGPS.tm_mday = atoi(cday);
    tmGPS.tm_hour = atoi(chour);
    tmGPS.tm_min = atoi(cminute);
    tmGPS.tm_sec = atoi(csecond);

    utime = mktime(&tmGPS);
    return PAL_Ok;
}

double RealGpsProvider::CovertGpsLatLonToDouble(const char *latlongitute){
    double retval = 0.0;
    int i = 0;
    int len = 0;
    int dotpos = 0;
    int angledegreeindex = 0;
    int angleotherindex = 0;
    char	csangledegreeval[8] = {0};
    char	csangleother[8] = {0};

    if (latlongitute == NULL){
        return retval;
    }

    len = strlen(latlongitute);
    while ((latlongitute[i] != '.') && (i<len)){
        dotpos++;
        i++;
    }

    len = strlen(latlongitute);
    for (i=0; i<dotpos-2; i++){
        csangledegreeval[angledegreeindex] = latlongitute[i];
        angledegreeindex++;
    }

    for (i=dotpos-2; i<len; i++){
        csangleother[angleotherindex] = latlongitute[i];
        angleotherindex++;
    }

    retval = atoi(csangledegreeval) + atof(csangleother)/60;
    return retval;
}






/*! @} */

