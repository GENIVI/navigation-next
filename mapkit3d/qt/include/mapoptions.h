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

    @file mapoptions.h
    @date 08/06/2014
    @addtogroup mapkit3d
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */


#ifndef __MAP_OPTIONS_H__
#define __MAP_OPTIONS_H__

#include <QString>

namespace locationtoolkit
{

class MapOptions
{
public:
    /**
     * MapOptions constructor.
     * Default constructor to use when creating a MapOptions object.
     */
     MapOptions();
     MapOptions(double defaultLatitude,
                double defaultLongitude,
                double defaultTiltAngle,
                int maximumCachingTileCount,
                int maximumTileRequestCountPerLayer,
                int zoomLevel);

public:
    /** Default latitude of the map center*/
    double mDefaultLatitude;
    /** Default longitude of the map center*/
    double mDefaultLongitude;
    /** Default tilt angle in degrees.*/
    double mDefaultTiltAngle;
    /** Maximum count of caching tiles.*/
    int mMaximumCachingTileCount;
    /** Maximum count of tile requests per layer.*/
    int mMaximumTileRequestCountPerLayer;
    /** Cache folder to store downloading data.*/
    QString mCacheFolder;
    /** Resource folder to store pre-loading data.*/
    QString mResourceFolder;
    /** Work folder.*/
    QString mWorkFolder;
    /** Default zoom level of the map. Range(2, 22].*/
    int mZoomLevel;
    /** Language code. */
    QString mLanguageCode;
    /** Full screen anti-aliasing settings, the default is off. */
    bool mEnableFullScreenAntiAliasing;
    /** Anisotropic Filtering settings, the default is off. */
    bool mEnableAnisotropicFiltering;
    /** SSAO settings, the default is off. */
    bool mEnableSSAO;
    /** Glow avatar and route settings, the default is off. */
    bool mEnableGlowRoute;
    /** Only show mapview, or shoud mapview as well as other widgets, the default is off. */
    bool mMapviewOnly;
    /**
    Materials shall be customized to look good for different classes of products and devices.
    For example, products designed for hand held devices require a different
    treatment of fonts and graphics than products designed for in-dash automotive devices.
    To support this, the metadata source query shall support an optional product-class attribute,
    which allows the client to specify the type of product and device on which the map will be displayed.

    The value rule:
    'default' : Standard hand held device.
    'att': Low resolution graphics platform, such as AT&T Drive.
    'nvidia': High end graphics automotive platform, such as Nvidia.
    */
    QString mProductClass;
private:
     /** Default value is 1000. */
     const static int MAXIMUM_CACHING_TILE_COUNT = 1000;
     /** Default value is 128. */
     const static int MAXIMUM_TILE_REQUEST_COUNT_PERLAYER = 128;
     /** Location of Aliso Viejo. */
     const static double DEFAULT_LATITUDE/* = 33.604*/;
     /** Location of Aliso Viejo. */
     const static double DEFAULT_LONGITUDE/* = -117.689*/;
     /** Default value is 90. */
     const static double DEFAULT_TILT_ANGLE/* = 90.0*/;
};
}  //namespace locationtoolkit
#endif // __MAP_OPTIONS_H__

/*! @} */
