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

#ifndef _STOPNAVIGATIONWIDGET_H_
#define _STOPNAVIGATIONWIDGET_H_

#include "Widget.h"

namespace nkui
{

/*! Parameter to fill dialog. */
class StopNavigationParameter
{
public:
    StopNavigationParameter(const string& message,
                            const string& confirmButtonText,
                            const string& cancelButtonText)
        : m_message(message),
          m_confirmButtonText(confirmButtonText),
          m_cancelButtonText(cancelButtonText)
    {
    }

    virtual ~StopNavigationParameter(){}

    std::string         m_message;                    // message
    std::string         m_confirmButtonText;          // text of confirm button
    std::string         m_cancelButtonText;           // text of cancel button
};

/*! This interface is called by presenter to show StopNavigation dialog. */
class StopNavigationWidget
{
public:
    /**
     *  Initialize a dialog with the given parameters
     *
     *  @param parameters: Parameters to initialize the dialog
     */
    virtual void ShowWithParameter(StopNavigationParameter& parameter) = 0;
};

}

#endif /* _STOPNAVIGATIONWIDGET_H_ */
