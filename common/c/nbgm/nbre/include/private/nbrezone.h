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

    @file nbrezone.h
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
#ifndef _NBRE_BinaryZone_H_
#define _NBRE_BinaryZone_H_
#include "nbretypes.h"
#include "nbreaxisalignedbox3.h"
#include "nbrespatialobject.h"
#include "nbrevisitor.h"
#include "nbrematrix4x4.h"
#include "nbredebugboundingbox.h"


typedef NBRE_Set<NBRE_ISpatialObject*> NBRE_SpatialObjectList;
/*! \addtogroup NBRE_Scene
*  @{
*/
/** this class is a binary space node, it contains two children node to divide parent half.
 */
class NBRE_BinaryZone
{

public:
    NBRE_BinaryZone();
    virtual ~NBRE_BinaryZone();

public:
    NBRE_BinaryZone* CreateLeftChild();
    NBRE_BinaryZone* CreateRightChild();

    void SetLeftChild(NBRE_BinaryZone* child);
    void SetRightChild(NBRE_BinaryZone* child);

    NBRE_BinaryZone* LeftChild();
    NBRE_BinaryZone* RightChild();

    NBRE_BinaryZone* RemoveLeftChild();
    NBRE_BinaryZone* RemoveRightChild();

    void AddSpatialObject(NBRE_ISpatialObject* obj);
    void AddSubZoneSpatialObject(NBRE_ISpatialObject* obj);
    void RemoveSpatialObject(NBRE_ISpatialObject* obj);

    const NBRE_SpatialObjectList& SpatialObjects() const; 
    NBRE_SpatialObjectList& SpatialObjects(); 
    const NBRE_SpatialObjectList& SubZoneSpatialObjects() const; 

    const NBRE_AxisAlignedBox3f& AxisAlignedBox() const; 
    NBRE_AxisAlignedBox3f& AxisAlignedBox(); 
    void SetBoundingbox(const NBRE_AxisAlignedBox3f& box);



protected:

    NBRE_BinaryZone* mLeftChild;
    NBRE_BinaryZone* mRightChild;

    NBRE_SpatialObjectList mSpatialObjects;
    NBRE_SpatialObjectList mSubZoneSpatialObjects;
    NBRE_AxisAlignedBox3f mBoundingBox;         /*!< Include children bounding box*/
    
};

/*! @} */
#endif
