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
 
 @file     nblocationserviceinterface.h
 @date     03/15/2012
 @defgroup LOCATIONSERVICES_GROUP
 
 */
/*
 CellIDInformation class declaration
 This class retrieves cellular network data
 (primarily, cell ID and signal strength).
 
 (C) Copyright 2012 by TeleCommunication Systems, Inc.
 
 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef NBLOCATIONSERVICEINTERFACE_H
#define NBLOCATIONSERVICEINTERFACE_H

extern "C"
{
#include "nberror.h"
#include "paltypes.h"
#include "abpalgps.h"
#include "nbcontext.h"
#include "nbnavigation.h"
}

#include "nblocationlistenerinterface.h"
#include "base.h"

#define NAVIGATION_ACCURACY 100
#define NONNAVIGATION_ACCURACY 250
#define NARROWTOACCURATE_ACCURACY 250

namespace nbcommon
{

/*!
This contains LocationKit types which could be used in LocationService
*/
typedef enum {
    NB_LKT_Default = 0, 
     
    NB_LKT_Gps,        /*!<Location kit which provides gps fixes only*/
    NB_LKT_Combined    /*!<Location kit which provides fixes from all available sources*/
} NB_LocationKitType;

typedef enum {
    NB_TT_None = 0,

    NB_TT_Tracking,
    NB_TT_NarrowToAccurate,
    NB_TT_Cancel
} NB_TrackingType;

    
/*!
LocationService provides location fixes using desired LocationKit
*/

class LocationServiceInterface
{
public:

    /*! Create LocationServiceInterface instance 

        @return None
     */
    static LocationServiceInterface* CreateLocationService(PAL_Instance* pal,           /*!< PAL instance */
                                                           NB_LocationKitType kitType); /*!< LocationKit Type */

    /*! Destroys Location Service instance
     
        @return None
     */ 
    virtual void Release() = 0;

    /*! Starts navigation Tracking
        
        @return NE_OK if Success
     */
    virtual NB_Error StartNavigationTracking(ABPAL_GpsConfig* config,                       /*!< Gps Config */
                                             uint32 configSize,                             /*!< Config Size */
                                             LocationServiceListenerInterface* listener,    /*!< Listener which would recieve fixes */
                                             NB_GpsCriteria criteria                        /*!< Criteria */
                                             ) = 0;

    /*! Starts non navigation Tracking
     
        @return NE_OK if Success
     */
    virtual NB_Error StartNonNavigationTracking(ABPAL_GpsConfig* config,                            /*!< Gps Config */
                                                uint32 configSize,                                  /*!< Config Size */
                                                LocationServiceListenerInterface* listener,         /*!< Listener which would recieve fixes */
                                                NB_GpsCriteria criteria                             /*!< Criteria */
                                                ) = 0;
    
    /*! Narrows location to defined accuracy minimum
     
        Provides all fixed which accuracy less then defined one.
        Once defined accuracy is reached tracking stops
        
        @return NE_OK if success
     */
    virtual NB_Error NarrowToAccurate(ABPAL_GpsConfig* config,                              /*!< Gps Config */
                                      uint32 configSize,                                    /*!< Config Size */
                                      LocationServiceListenerInterface* listener,           /*!< Listener which would recieve fixes */
                                      NB_GpsCriteria criteria                               /*!< Criteria */
                                      ) = 0;
    
    /* Stop any tracking for defined listener
     
     @return NE_OK if success
     */
    virtual NB_Error StopTracking(LocationServiceListenerInterface* listener                /*!< Listener which recieves fixes */
                                  ) = 0;

    /* Get last stored location
     
     @return location
    */
    virtual NB_GpsLocation& GetLastLocation() = 0;

    /* Set last location

     @return None
     */
    virtual void SetLastLocation (NB_GpsLocation location                                 /*!< Last stored location */
                                  ) = 0;

    /* Update gps criteria
     
     @return location
     */
    virtual NB_Error UpdateCriteria(LocationServiceListenerInterface* listener,             /*!< Listener which recieves fixes */
                                    NB_GpsCriteria criteria) = 0;                           /*!< Criteria */


    /* Get current tracking state for the specified listener

     @return Tracking type
     */

    virtual NB_TrackingType CurrentTrackingType(LocationServiceListenerInterface* listener  /*!< Listener which recieves fixes */
                                                ) = 0;

};

}

#endif
/*! @} */
