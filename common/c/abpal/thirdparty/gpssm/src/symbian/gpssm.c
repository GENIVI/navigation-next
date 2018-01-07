/*!--------------------------------------------------------------------------

@file gpssm.c

*/
/*
(C) Copyright 2010 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "gpssm.h"
#include "palstdlib.h"
#include "sm.h"

struct GpsSecurityModule
{
	void* vp;
};

#define LAT_LON_TO_INT_CONSTANT (3600 * 1024.0) /* convert degree to 1/1024 seconds) */

#define SECONDS_PER_WEEK (3600 * 24 * 7)        /* seconds in a weeek */

static unsigned int
lat_lon_to_int(double d)
{
	int result = (int) (d * LAT_LON_TO_INT_CONSTANT);

	return (unsigned int)result;
}

static double int_to_lat_lon(unsigned int i)
{
	double result = ((int)i) / LAT_LON_TO_INT_CONSTANT;
	return result;
}

ABPAL_DEF PAL_Error
GpsSecurityModuleCreate(GpsSecurityModule** securityModule)
{
	GpsSecurityModule* result = NULL;

	if (securityModule == NULL)
	{
		return PAL_ErrBadParam;
	}

	result = (GpsSecurityModule*) nsl_malloc(sizeof(GpsSecurityModule));
	if (result == NULL)
	{
		return PAL_ErrNoMem;
	}

	result->vp = NULL;

	*securityModule = result;
	return PAL_Ok;
}

ABPAL_DEF PAL_Error
GpsSecurityModuleDestroy(GpsSecurityModule* securityModule)
{
	if (securityModule == NULL)
	{
		return PAL_ErrBadParam;
	}

	if (securityModule->vp != NULL)
	{
		nsl_free (securityModule->vp);
	}

	nsl_free(securityModule);
	return PAL_Ok;
}

ABPAL_DEF PAL_Error
GpsSecurityModuleEncrypt(GpsSecurityModule* securityModule,
        ABPAL_GpsLocation* gpsLocation,
        ABPAL_GpsLocation* result)
{
	unsigned int wgsLon = 0;
	unsigned int wgsLat = 0;
	int wgsHeight = 0;
	int wgsWeek = 0;
	int wgsSeconds = 0;

	unsigned int chinaLat = 0;
	unsigned int chinaLon = 0;

	if ((securityModule == NULL) || (gpsLocation == NULL) || (result == NULL))
	{
		return PAL_ErrBadParam;
	}

	wgsLat = lat_lon_to_int(gpsLocation->latitude);
	wgsLon = lat_lon_to_int(gpsLocation->longitude);
	wgsHeight = (int)gpsLocation->altitude;

	wgsWeek = gpsLocation->gpsTime / SECONDS_PER_WEEK;
	wgsSeconds = gpsLocation->gpsTime % SECONDS_PER_WEEK;

	if (wgtochina_lb(&securityModule->vp, wgsLon, wgsLat, wgsHeight,
				     wgsWeek, wgsSeconds, &chinaLon, &chinaLat))
	{
		/*  non-zero, failed */
		return PAL_Failed;
	}

	*result = *gpsLocation;
	result->latitude = int_to_lat_lon(chinaLat);
	result->longitude = int_to_lat_lon(chinaLon);

	return PAL_Ok;
}

/*! @} */
