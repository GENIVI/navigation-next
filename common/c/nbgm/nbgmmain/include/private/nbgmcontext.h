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

    @file nbgmcontext.h
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
#include "nbresharedptr.h"
#include "nbrecontext.h"

#ifndef _NBGM_CONTEXT_H_
#define _NBGM_CONTEXT_H_
#include "paltypes.h"

class NBRE_MapMaterialManager;
class NBGM_BuildModelContext;
class NBGM_LayoutManager;
class NBRE_RenderEngine;
class NBGM_RenderContext;
class NBRE_TransformUtil;
class NBRE_FileLogger;
class NBRE_TextureManager;

struct NBGM_Context
{
    NBGM_Context():
        buildModelContext(NULL),
        layoutManager(NULL),
        renderingEngine(NULL),
        needMoreRenderCycle(FALSE),
        transUtil(NULL),
        fileLogger(NULL),
        maxOverlayId(0),
        scaleFactor(1)
    {

    }
    NBGM_BuildModelContext*         buildModelContext;
    NBGM_LayoutManager*             layoutManager;
    NBRE_RenderEngine*              renderingEngine;
    nb_boolean                      needMoreRenderCycle;
    shared_ptr<NBGM_RenderContext>  renderContext;
    NBRE_TransformUtil*             transUtil;
    NBRE_FileLogger*                fileLogger;
    int32                           maxOverlayId;

    inline float ModelToWorld(float v) const
    {
        return v/scaleFactor;
    }
    inline float WorldToModel(float v) const
    {
        return v*scaleFactor;
    }
    inline double ModelToWorld(double v) const
    {
        return v/scaleFactor;
    }
    inline double WorldToModel(double v) const
    {
        return v*scaleFactor;
    }

    inline void SetModelScaleFactor(float factor)
    {
        scaleFactor = factor;
    }

    inline float GetModelScaleFactor()
    {
        return scaleFactor;
    }

private:
    float                           scaleFactor;
};


struct NBGM_ResourceContext
{
    NBGM_ResourceContext():
        palInstance(NULL),
        renderPal(NULL),
        mapMaterialManager(NULL),
        textureManager(NULL),
        buildModelContext(NULL),
        fileLogger(NULL)
    {

    }

    PAL_Instance*                   palInstance;
    NBRE_IRenderPal*                renderPal;
    NBRE_MapMaterialManager*        mapMaterialManager;
    NBRE_TextureManager*            textureManager;
    NBGM_BuildModelContext*         buildModelContext;
    NBRE_FileLogger*                fileLogger;

    inline float ModelToWorld(float v) const
    {
        return v/scaleFactor;
    }
    inline float WorldToModel(float v) const
    {
        return v*scaleFactor;
    }
    inline double ModelToWorld(double v) const
    {
        return v/scaleFactor;
    }
    inline double WorldToModel(double v) const
    {
        return v*scaleFactor;
    }

    inline void SetModelScaleFactor(float factor)
    {
        scaleFactor = factor;
    }

    inline float GetModelScaleFactor()
    {
        return scaleFactor;
    }

private:
    float                           scaleFactor;
};

/*! @} */
#endif
