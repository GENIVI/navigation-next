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
    @file         FooterWidget.h
    @defgroup     nkui
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary to
    TeleCommunication Systems, Inc., and considered a trade secret as defined
    in section 499C of the penal code of the State of California. Use of this
    information by anyone other than authorized employees of TeleCommunication
    Systems is granted only under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
    --------------------------------------------------------------------------
*/

#ifndef _NKUI_FOOTERWIDGET_H_
#define _NKUI_FOOTERWIDGET_H_

#include <string>

namespace nkui
{
class FooterWidget
{
public:
    /** Show progress bar
     */
    virtual void ShowProgress(float progress) = 0;

    /** Hide progress bar
     */
    virtual void HideProgress() = 0;

    /** Show indicator view
     */
    virtual void ShowIndicator() = 0;

    /** Hide indicator view
     */
    virtual void HideIndicator() = 0;

    /*! set messge show at footer. 
     */
    virtual void ShowMessage(const std::string& message) = 0;

    /*! hide message. 
     */
    virtual void HideMessage() = 0;

    /*! show menu button
     */
    virtual void ShowMenuButton() = 0;

    /*! Hide menu button 
     */
    virtual void HideMenuButton() = 0;

    /*! show list button. 
     */
    virtual void ShowListButton() = 0;

    /*! Hide list button. 
     */
    virtual void HideListButton() = 0;

    /*! show cancel button.
     */
    virtual void ShowcancelButton() = 0;
    /*! Hide cancel button.
     */
    virtual void HidecancelButton() = 0;
    /*! Set list button status
     *
     *  @param isOpen: true if opened.
     *  @return: None
     */
    virtual void SetListButtonStatus(bool isOpen) = 0;
    /*! set title. */
    virtual void SetTitle(std::string title) = 0;
};
}

#endif
