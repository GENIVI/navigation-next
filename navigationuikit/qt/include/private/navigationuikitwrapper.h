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
    @file navigationuikitwrapper.h
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
#ifndef LOCATIONTOOLKIT_NAVIGATIONUI_WRAPPER_H
#define LOCATIONTOOLKIT_NAVIGATIONUI_WRAPPER_H

#include "NKUIController.h"
#include "navigationuikit.h"
#include "signal/navigationuisignals.h"
#include "data/userpreferences.h"
#include "data/nkuitypes.h"
#include "data/place.h"
#include "private/mapinterfacewrapper.h"
#include "private/localizedstringhelperimpl.h"
#include "private/locationproviderlistener.h"
#include "private/widgets/widgetmanager.h"
#include "private/typetranslater.h"
#include "ltkcontextimpl.h"
#include <QDir>
#include <QCoreApplication>

namespace locationtoolkit
{
using namespace nkui;
class NavigaitonUIKitWrapper:public NKUIListener,NKUIEventListener
{
public:
    NavigaitonUIKitWrapper(const UserPreferences& prefs, const Place& destination, const Place& origin);
    virtual ~NavigaitonUIKitWrapper();
public:
    const NavigationUISignals& GetNavigationUISignals()const;
    void Recalculate(const NavUIRouteOptions& nkuiRouteOption);
    ShareInformation GetNavShareInformation()const;
    void Start();
    void Pause();
    void Resume();
    void Delete();
    void AddErrorHandler(NavigationUIErrorHandler* errorHandler);
    void RemoveErrorHandler(NavigationUIErrorHandler* errorHandler);

    void NotifyEvent(NKEventID event, NKUIEventData data);

    //NKUIListener
    virtual void OnNavigationStart();
    virtual void OnRouteOptionsChanged();
    virtual void OnArrived();
    virtual void OnNavigationEnd();
    virtual void OnNavigationCancel();
    virtual bool OnConfirmAction(NavigationUIAction action);
    virtual void OnTurnByTurnNavigationStart();
    virtual void ShowOverflowMenu(NKUIPublicState state, std::vector<MenuOption>& items, bool show);
    virtual void OnNavigationStateChanged(NKUIPublicState state);
    //NKUIErrorListener
    virtual bool OnError(NKUI_Error errorCode);

    /*! App was sent to background or brought to foreground
     *
     * @param isBackground true means sent to background otherwise brought to foreground
     * @return none.
     */
    void SetBackground(bool isBackground);

    /*!
     * set the calling state
     *
     * @param isInCalling true means in calling state, otherwise not in calling state.
     * @return none
     */
    void SetCallingState(bool isInCalling);

    /*! cancel the timer which will end trip
     *
     * @param none.
     * @return none.
     */
    void CancelEndTripTimer();

    /*! end the trip
     *
     * @param none.
     * @return none.
     */
    void EndTrip();

    /*! get NavigaitonUIKitWrapper instance. */
    static NavigaitonUIKitWrapper* GetInstance(const UserPreferences& prefs, const Place& destination, const Place& origin);
    /*! get resource path. */
    static QString& GetResourcePath();
private:
    NavigaitonUIKitWrapper();

    /*! get the call state, in calling state or not
     *
     * @param none.
     * @return yes in calling ,else not.
     */
    bool IsInCallingState();

    bool LoadNexGenFont();

    bool LoadNexGenFont(QString fontFilePath);

    static NavigaitonUIKitWrapper* uikitWrapperInstance;
private:
    NKUIController*                         mNkuiController;
    bool                                    mIsInBackground;
    QString                                 mProductName;
    NavigationUISignals                     mNavigationUISignals;
    MapInterfaceWrapper*                    mMapInterfaceWrapper;
    LocalizedStringHelperImpl*              mStringHelper;
    LocationProviderListener*               mLocationListener;
    std::set<NavigationUIErrorHandler*>     mUIErrorHandlerSet;
    WidgetLayoutConfig*                     mLayoutConfig;
    nbnav::Place                            mDestination;
    nbnav::Place                            mOrigin;
    typedef shared_ptr<const nbnav::Place> PlacePtr;
    PlacePtr                                mNavDestination;
    PlacePtr                                mNavOrigin;
    QWidget*                             mParentView;
    QString                                 mResourcePath;
};
}  // namespace locationtoolkit
#endif
/*! @} */
