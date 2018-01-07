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
    @file navigationuikitimpl.h
    @date 10/15/2014
    @addtogroup navigationuikit
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
#ifndef LOCATIONTOOLKIT_NAVIGATIONUIKITIMPL_H
#define LOCATIONTOOLKIT_NAVIGATIONUIKITIMPL_H


#include "navigationuikit.h"
#include "private/navigationuikitwrapper.h"

namespace locationtoolkit
{

class NavigationUIKitImpl:public NavigationUIKit
{
public:
    NavigationUIKitImpl(const UserPreferences& prefs, const Place& destination, const Place& origin);
    virtual ~NavigationUIKitImpl();

    /**
    * Get NavigationUIsignals for user bind to NavigationUI slot.
    * @return NavigationUI signals.
    */
    virtual const NavigationUISignals& GetNavigationUISignals() const;

    /**
    *  recalculate with route option
    *
    *  @param routeOption the route opthion, it can be nil if not change current option.
    */
    virtual void Recalculate(const NavUIRouteOptions& routeOption);

    /**
    *  Get navigation share information
    *
    *  @return return share information see LTKNUShareInformation
    */
    virtual ShareInformation GetNavShareInformation()const;

    /**
     * Stops the navigation session and releases all the resources.
     */
    virtual void Start();

    /**
    * Pauses the navigation UI.
    *
    * This could be called when the NKUI is running in the background to stop
    * announcements during a phone call for example.
    */
    virtual void Pause();

    /**
    * Resumes the paused Navigation UI.
    */
    virtual void Resume();

   /**
    * Delete this NavgationUI.
    *
    * All the data associated with the NKUI controller will be released then
    * closes the navigation UI view to return control to the host app.
    */
    virtual void Delete();

    /**
    * Add ErrorHandler
    * @param errorHandler     object of NavigationUIErrorHandler
    */
    virtual void AddErrorHandler(NavigationUIErrorHandler* errorHandler);

    /**
    * Remove ErrorHandler
    *@param errorHandler     object of NavigationUIErrorHandler
    */
    virtual void RemoveErrorHandler(NavigationUIErrorHandler* errorHandler);

private:
    NavigaitonUIKitWrapper* mUIKitWrapper;
};
}  // namespace locationtoolkit
#endif
/*! @} */