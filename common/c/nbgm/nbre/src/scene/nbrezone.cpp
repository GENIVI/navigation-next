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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
#include "nbrezone.h"
#include "nbrecommon.h"
#include "nbredefaultpass.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"

NBRE_BinaryZone::NBRE_BinaryZone(): mLeftChild(NULL), mRightChild(NULL)
{
}

NBRE_BinaryZone::~NBRE_BinaryZone()
{
    NBRE_DELETE mLeftChild;
    NBRE_DELETE mRightChild;
}

NBRE_BinaryZone* NBRE_BinaryZone::CreateLeftChild()
{
    if(mLeftChild == NULL)
    {
        mLeftChild = NBRE_NEW NBRE_BinaryZone();
    }
    return mLeftChild;
}

NBRE_BinaryZone* NBRE_BinaryZone::CreateRightChild()
{
    if(mRightChild == NULL)
    {
        mRightChild = NBRE_NEW NBRE_BinaryZone();
    }
    return mRightChild;
}

void NBRE_BinaryZone::SetLeftChild(NBRE_BinaryZone* child)
{
    NBRE_DELETE mLeftChild;
    mLeftChild = child;
}

void NBRE_BinaryZone::SetRightChild(NBRE_BinaryZone* child)
{
    NBRE_DELETE mRightChild;
    mRightChild = child;
}

NBRE_BinaryZone* NBRE_BinaryZone::LeftChild()
{
    return mLeftChild;
}

NBRE_BinaryZone* NBRE_BinaryZone::RightChild()
{
    return mRightChild;
}

NBRE_BinaryZone* NBRE_BinaryZone::RemoveLeftChild()
{
    NBRE_BinaryZone* zone = mLeftChild; 
    mLeftChild = NULL;
    return zone;
}

NBRE_BinaryZone* NBRE_BinaryZone::RemoveRightChild()
{
    NBRE_BinaryZone* zone = mRightChild; 
    mRightChild = NULL;
    return zone;
}

void NBRE_BinaryZone::AddSpatialObject(NBRE_ISpatialObject* obj)
{
    mSpatialObjects.insert(obj);
}

void NBRE_BinaryZone::AddSubZoneSpatialObject(NBRE_ISpatialObject* obj)
{
    mSubZoneSpatialObjects.insert(obj);
}

void NBRE_BinaryZone::RemoveSpatialObject(NBRE_ISpatialObject* obj)
{
    mSpatialObjects.erase(obj);
    mSubZoneSpatialObjects.erase(obj);
}

const NBRE_AxisAlignedBox3f& NBRE_BinaryZone::AxisAlignedBox() const
{
    return mBoundingBox;
}
NBRE_AxisAlignedBox3f& NBRE_BinaryZone::AxisAlignedBox()
{
    return mBoundingBox;
}

void NBRE_BinaryZone::SetBoundingbox(const NBRE_AxisAlignedBox3f& box)
{
    mBoundingBox = box;
}

const NBRE_SpatialObjectList& NBRE_BinaryZone::SpatialObjects() const
{
    return mSpatialObjects;
}

NBRE_SpatialObjectList& NBRE_BinaryZone::SpatialObjects() 
{
    return mSpatialObjects;
}

const NBRE_SpatialObjectList& NBRE_BinaryZone::SubZoneSpatialObjects() const
{
    return mSubZoneSpatialObjects;
}
