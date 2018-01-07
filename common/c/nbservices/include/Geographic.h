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
 @file     geographic.h
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

#ifndef GEOGRAPHIC_H_
#define GEOGRAPHIC_H_

#include <string>

namespace nbmap
{

/*! Type of GeoGraphicSelectedListener.

    GeoGraphic is selected listener interface, once you registered your own listener,
    you should be responsible to do something when object selected or unselected.
*/
class GeographicSelectedListener
{
public:
    virtual ~GeographicSelectedListener(){}
    /**
     * @name OnSelected - callback to respond select event.
     * @param isSelected - if selected
     * @return None.
     */
    virtual bool OnSelected(bool isSelected) = 0;

};

class GeographicPrivate;

/*! Base class of Graphic object
 */
class Geographic
{
public:
    virtual ~Geographic(){};

    /**
     * @name Select - select or unselect current Graphic object.
     * @param isSelected - if select it
     * @return None.
     */
    virtual void SetSelected(bool isSelected) = 0;

    /**
     * @name IsSelected - Get selected state of current Graphic object.
     * @return is object selected.
     */
    virtual bool IsSelected() = 0;

    /**
     * @name SetSelectedListner - Set Selected Listner.
     * @param l
     * @return None.
     */
    virtual void SetSelectedListner(GeographicSelectedListener* l);

    /**
     * @name SetDrawOrder - Set draw order.
     * @param order
     * @return None.
     */
    virtual void SetDrawOrder (int order);

    /*! return an identifier of this GeoGraphic Object.

        @return identifier of this Object.
    */
    virtual const std::string& GetId() const;

    /**
     * @name Submit - convert user data to graphic data.
     * @return None.
     */
    virtual void Submit() = 0;

    /**
     * @name Submit - show or hide current Graphic object.
     * @param bShow
     * @return None.
     */
    virtual void Show(bool bShow)   = 0;

    /**
     * @name SetParentGeographic - Some polylines are be combined. so we need to know this sub polyline belongs to.
     * @param parent
     * @return None.
     */
    virtual void SetParentGeographic(Geographic* parent) = 0;
    virtual Geographic* GetParentGeographic() = 0;

protected:
    Geographic(const std::string& id);
    GeographicPrivate* m_private;
    Geographic* m_parentGeographic;
};
}

#endif /* GEOGRAPHIC_H_ */
