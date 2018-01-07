/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include <math.h>
#include <stdio.h>

/*
 * dummy wgtochina_lb function that simply returns WGS84 latitude/longitude
 */
unsigned int wgtochina_lb(
        int wg_flag,            /* input: initial flag for variables in SM,
                                   0=need, 1=un-need,
                                   the initialization is needed while the GPS module restart */
        unsigned int wg_lng,    /* input: WGS84 longitude from GPS (LSB=1/1024 s) */
        unsigned int wg_lat,    /* input: WGS84 latitude from GPS (LSB=1/1024 s) */
        int wg_heit,            /* input: present altitude (LSB=1m) */
        int wg_week,            /* input: week number for time from GPS
                                   (from 0 o'clock 6th Jan. 1980) */
        unsigned int wg_time,   /* input: time (not enough for one week) based at week
                                   system from GPS (LSB=1/1000 s) */
        unsigned int* china_lng, /* output: shifted longitude (LSB=1/1024 s) */
        unsigned int* china_lat  /* output: shifted latitude (LSB=1/1024 s) */
)
{
    /* test sqrt, sin, cos function */
    double x = 1.0;
    double root = 0;
    double sinx = 0;
    double cosx = 0.0;

    root = sqrt(x);
    sinx = sin(x);
    cosx = cos(x);

    printf("wg_lng=%u\n", wg_lng);
    printf("wg_lat=%u\n", wg_lat);
    printf("wg_heit=%d\n", wg_heit);
    printf("wg_week=%d\n", wg_week);
    printf("wg_time=%u\n", wg_time);
    printf("\n");

    if (china_lng)
    {
        *china_lng = wg_lng;
    }

    if (china_lat)
    {
        *china_lat = wg_lat;
    }

    return 0;
}

