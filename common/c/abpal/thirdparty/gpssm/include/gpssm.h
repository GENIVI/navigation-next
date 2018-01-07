/*!--------------------------------------------------------------------------

@file gpssm.h

GPS Security Module Interface

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

/*!
    @addtogroup gpssm
    @{
*/


#ifndef GPSSM_H
#define	GPSSM_H

#include "abpalexp.h"
#include "pal.h"
#include "palerror.h"
#include "paltypes.h"
#include "abpalgps.h"

typedef struct GpsSecurityModule GpsSecurityModule;

/*! Create a GPS Security Module instance

This function is called to create a GPS security module instance.
it return PAL_Ok if a security module instance is created and the pointer to
the instance is stored at the location pointed by the parameter securityModule.

Call GpsSecurityModuleDestory when the GPS Security module is not needed.

@param securityModule pointer to the location where the pointer to the created
security module will be stored.

@return PAL_Error PAL_Ok if a GPS security module is created successfullly.

@see GpsSecurityModuleDestory
*/
ABPAL_DEC PAL_Error
GpsSecurityModuleCreate(GpsSecurityModule** securityModule);

/*! Destory a GPS Security Module instance

This function is called to destory a GPS security module instance that 
has been created by GpsSecurityModuleCreate

@param securityModule GPS security module instance that has been created
by GpsSecurityModuleCreate.
@return PAL_Error PAL_Ok if the GPS security module is destoryed successfully .

@see GpsSecurityModuleCreate
*/
ABPAL_DEC PAL_Error
GpsSecurityModuleDestroy(GpsSecurityModule* securityModule);

/*! Convert a WGS84 GPS location to an encrypted GPS Location

This function is called to encrypt a WGS84 GPS Location to an encrypted GPS
location that is used by encrypted map data.

@param securityModule GPS security module instance that has been created
by GpsSecurityModuleCreate.
@param wgsGpsLocation GPS Location with WGS84 lat/lon
@param chinaGpsLocation GPS Location with shifted China lat/lon
@return PAL_Error PAL_Ok if the GPS location is converted successfully.
*/
ABPAL_DEC PAL_Error
GpsSecurityModuleEncrypt(GpsSecurityModule* securityModule,
        ABPAL_GpsLocation* wgsGpsLocation,
        ABPAL_GpsLocation* chinaGpsLocation);


#endif

/*! @} */
