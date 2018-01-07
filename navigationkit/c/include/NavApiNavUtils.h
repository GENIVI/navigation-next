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
    @file     NavApiNavUtils.h
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */


#ifndef __NAVAPINAVUTIL_H__
#define __NAVAPINAVUTIL_H__

#include "NavApiTypes.h"
extern "C"
{
#include "nbcontext.h"
}
#include <map>
#include <string>

namespace nbnav
{

/*! Non thread safe classes*/

/*! @todo;LOW PRIORITY;OTHER;use C++ Context instead of NB_Context* nbContext*/

/*! map contains supported proununce voice style informations
 * for example: < en_US, <female, female-5-aac-v3 > >
 */
typedef std::map<std::string,               /*! Supported Language */
                 std::pair<std::string,     /*! voice name (prefix of the voiceStyle) */
                           std::string      /*! voice style */
           > > NKProununceDateBase;

/*! NavUtils

    Class provides navigation utility functions
*/
class NavUtils
{
public:
    /*! Get the supports proununce voice styles

        @param  workpath - the path will be searched.
        @return map contains supported proununce voice styles
     */
    static NKProununceDateBase GetAvailableAudioStyle(const std::string& workpath);

    /*! Calculate curved Earth distance between two points.

       @param localLatitude   latitude of starting point
       @param localLongitude  longitude of starting point
       @param remoteLatitude  latitude of end point
       @param remoteLongitude longitude of end point
       @param heading         return the heading in degree from the start to the end point.
                              Optional, set to NULL if not needed
       @return distance between starting point and destination point (meters)
     */
    static double CalculateDistance(double startLatitude,
                                    double startLongitude,
                                    double endLatitude,
                                    double endLongitude,
                                    double* heading);

    /*! Covert geographic coordinate to marcator coordinate.

       @param latitude      geographic latitude
       @param longitude     geographic longitude
       @param xMercator     marcator coordinate x
       @param yMercator     marcator coordinate y
       @return none
     */
    static void MercatorForward(double latitude, double longitude, double* xMercator, double* yMercator);

    /*! Covert marcator coordinate to geographic coordinate.

       @param xMercator     marcator coordinate x
       @param yMercator     marcator coordinate y
       @param latitude      geographic latitude
       @param longitude     geographic longitude
       @return none
     */
    static void MercatorReverse(double xMercator, double yMercator,double* latitude, double* longitude);

    /*! Gets an image bitmap

        @param context NB_Context instance
        @param imageName
        @return image object
    */
    static Image GetImage(NB_Context* nbContext, std::string imageName);

    /*! Get the navigation voice file

        @param context NB_Context instance
        @param voiceName
        @return voice buffer
    */
    static BinaryBuffer GetNavigationVoice(NB_Context* nbContext, std::string voiceName);

    /*! Get a maneuver icon image

        @param context NB_Context instance
        @param routingIconName
        @return image object
    */
    static Image GetRoutingIconImage(NB_Context* nbContext, std::string routingIconName);
    
    /*! remove marked text. 
        the function will remove <...> substring from gvie text. if the origin string is 
        "and make a hard <span class=\"direction\">left</span> turn" the return string is
        "and make a hard left turn"
        @param originText;
        @return strip string.
     */
    static std::string ConvertFormatStringToPlainString(std::string oriString);
};

}

#endif

/*! @} */
