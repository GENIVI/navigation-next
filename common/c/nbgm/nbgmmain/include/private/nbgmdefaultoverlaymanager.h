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

    @file nbgmdefaultoverlaymanager.h
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
#ifndef _NBRE_DEFAULT_OVERLAY_MANAGER_H_
#define _NBRE_DEFAULT_OVERLAY_MANAGER_H_
#include "nbretypes.h"
#include "nbreoverlay.h"
#include "nbreoverlaymanager.h"


/** \addtogroup NBRE_Scene
*  @{
*/
/** 
a default Overlay Manager, user can define draw orders by themselves
*/
template<typename DrawOrder>
class NBRE_DefaultOverlayManager: public NBRE_IOverlayManager
{
public:
    static const int32 Invalidate_Id = -1;

public:
    NBRE_DefaultOverlayManager(NBRE_Context& context, int32* maxId): mContext(context), mMaxId(maxId)
    {
    }

    virtual ~NBRE_DefaultOverlayManager()
    {
        for(IdOverlayMap::iterator pOverlay = mIdOverlayMap.begin(); pOverlay != mIdOverlayMap.end(); ++pOverlay)
        {
            NBRE_DELETE pOverlay->second;
        }
    }

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_DefaultOverlayManager);

public:
    virtual void Begin()
    {
        mCursor = mDrawOrderIdMap.begin();
    }

    virtual void Next()
    {
        if(mCursor == mDrawOrderIdMap.end())
        {
            return;
        }
        ++mCursor;
    }

    virtual NBRE_Overlay* GetOverlay()
    {
        if(mCursor == mDrawOrderIdMap.end())
        {
            return NULL;
        }
        return mIdOverlayMap[mCursor->second];
    }

    virtual NBRE_Overlay* FindOverlay(int32 id) 
    {
        IdOverlayMap::iterator iter = mIdOverlayMap.find(id);
        if(iter == mIdOverlayMap.end())
        {
            return NULL;
        }
        return iter->second;
    }

    void RemoveOverlay(int32 id)
    {
        typename IdOverlayMap::iterator iter = mIdOverlayMap.find(id);
        if(iter == mIdOverlayMap.end())
        {
            return;
        }
        NBRE_DELETE iter->second;
        mIdOverlayMap.erase(iter);

        for(typename DrawOrderIdMap::iterator pOverlay = mDrawOrderIdMap.begin(); pOverlay != mDrawOrderIdMap.end(); ++pOverlay)
        {
            if(pOverlay->second == id)
            {
                mDrawOrderIdMap.erase(pOverlay);
                break;
            }
        }
    }

    int32 FindId(DrawOrder order)
    {
        typename DrawOrderIdMap::iterator iter = mDrawOrderIdMap.find(order);
        if(iter == mDrawOrderIdMap.end())
        {
            return Invalidate_Id;
        }
        return iter->second;
    }

    int32 AssignOverlayId(DrawOrder order)
    {
        int32 id = FindId(order);
        if(id == Invalidate_Id)
        {
            mDrawOrderIdMap[order] = *mMaxId;
            mIdOverlayMap[*mMaxId] = NBRE_NEW NBRE_Overlay(mContext);
            return (*mMaxId)++;
        }
        return id;
    }



private:
    typedef NBRE_Map<int32, NBRE_Overlay*> IdOverlayMap; 
    typedef NBRE_Map<DrawOrder, int32> DrawOrderIdMap; 

private:
    NBRE_Context& mContext;
    IdOverlayMap mIdOverlayMap;
    DrawOrderIdMap mDrawOrderIdMap;
    typename DrawOrderIdMap::iterator mCursor;
    int32* mMaxId;
};

/** @} */
#endif
