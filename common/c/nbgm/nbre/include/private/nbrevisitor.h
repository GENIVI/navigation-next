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

    @file nbrevisitor.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_VISITABLE_H_
#define _NBRE_VISITABLE_H_
#include "nbretypes.h"
#include "nbrelight.h"
#include "nbreaxisalignedbox3.h"
#include "nbrevector3.h"
#include "nbreray3.h"

class NBRE_SurfaceManager;
/** \addtogroup NBRE_Scene
*  @{
*/
class NBRE_Visitor;

class NBRE_IVisitable
{
public:
    virtual ~NBRE_IVisitable(){};
    virtual void Accept(NBRE_Visitor& visitor) = 0;
};

class NBRE_IRenderOperationProvider;
class NBRE_ILightProvider;
class NBRE_ISpatialObject;
class NBRE_IPickedUpObject;
class NBRE_IMovableObject;

class NBRE_Visitor
{
public:
    virtual ~NBRE_Visitor(){}
    virtual void Visit(NBRE_IRenderOperationProvider* /*renderQueue*/, NBRE_SurfaceManager* /*surfaceManager*/){};
    virtual void Visit(NBRE_ILightProvider* /*updateLight*/){};
    virtual void Visit(NBRE_ISpatialObject* /*spatialObject*/){};
    virtual void Visit(NBRE_IPickedUpObject* /*pickObject*/){};
    virtual void Visit(NBRE_IMovableObject* /*movableObject*/){};

};

class NBRE_Overlay;
class NBRE_Camera;

class NBRE_IRenderOperationProvider
{
public:
    virtual ~NBRE_IRenderOperationProvider(){};
    virtual void NotifyCamera(const NBRE_Camera& camera) = 0;
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay) = 0;
    /// provider a overlay Id
    virtual int32 OverlayId() const = 0;
    virtual nb_boolean Visible() const = 0;
    ///update surface list
    virtual void UpdateSurfaceList() = 0;
    virtual uint32 GetPriority() const = 0;
};


class NBRE_ILightProvider
{
public:
    virtual ~NBRE_ILightProvider(){}
    virtual void UpdateLights(NBRE_LightList& lights) = 0;
};

/** Any object want to be picked, implement this interface
*/
class NBRE_IPickedUpObject
{
public:
    virtual ~NBRE_IPickedUpObject(){}
    /// respond choose event
    virtual void OnPickedUp() = 0;
    virtual nb_boolean IsIntersect(const NBRE_Camera& camera, const NBRE_Ray3f& ray, NBRE_Point3f* pIntersectPoint) = 0;
};

/*! @} */
#endif
