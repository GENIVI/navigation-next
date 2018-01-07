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

    @file nbgmbuildutility.h
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
#ifndef _NBRE_BUILD_UTILITY_H_
#define _NBRE_BUILD_UTILITY_H_

#include "nbgmbuildmodelcontext.h"
#include "nbgmarray.h"
#include "palerror.h"
#include "nbrecolor.h"
#include "nbrecontext.h"
#include "nbretextureptr.h"
#include "nbrevector3.h"
#include "nbgmcontext.h"
#include "nbreshader.h"
#include "nbgmmapmaterial.h"

/*! \addtogroup NBGM_Services
*  @{
*/
class NBRE_Model;
class NBRE_SubModel;

class NBGM_BuildModelUtility
{
public:

    static PAL_Error PatternPolylinePack2Tristripe(NBGM_ResourceContext& resourceContext, NBRE_Array* polyline, float width, float texRatio, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer, 
                                                  NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer, double& roadSegTotalLen);

    static PAL_Error PolylinePack2TristripeWithRoundEndpoint(NBRE_Array* polyline,
                                                             float width,
                                                             float texX,
                                                             float texY,
                                                             NBGM_FloatBuffer& vertexBuffer,
                                                             NBGM_FloatBuffer& normalBuffer, 
                                                             NBGM_FloatBuffer& texcoordBuffer1,
                                                             NBGM_FloatBuffer& texcoordBuffer2,
                                                             NBGM_ShortBuffer& indexBuffer);

    static PAL_Error PolylinePack2TristripeWithFlatEndpoint(NBRE_Array* polyline,
                                                            float width,
                                                            float texX,
                                                            float texY,
                                                            NBGM_FloatBuffer& vertexBuffer,
                                                            NBGM_FloatBuffer& normalBuffer, 
                                                            NBGM_FloatBuffer& texcoordBuffer1,
                                                            NBGM_FloatBuffer& texcoordBuffer2,
                                                            NBGM_ShortBuffer& indexBuffer);

    static PAL_Error PolylinePack2TristripeWithFlatStartCapRoundEndCap(NBRE_Array* polyline,
                                                                       float width,
                                                                       float texX,
                                                                       float texY,
                                                                       NBGM_FloatBuffer& vertexBuffer,
                                                                       NBGM_FloatBuffer& normalBuffer, 
                                                                       NBGM_FloatBuffer& texcoordBuffer1,
                                                                       NBGM_FloatBuffer& texcoordBuffer2,
                                                                       NBGM_ShortBuffer& indexBuffer);

    static PAL_Error PolylinePack2TristripeWithRoundStartCapFlatEndCap(NBRE_Array* polyline,
                                                                       float width,
                                                                       float texX,
                                                                       float texY,
                                                                       NBGM_FloatBuffer& vertexBuffer,
                                                                       NBGM_FloatBuffer& normalBuffer, 
                                                                       NBGM_FloatBuffer& texcoordBuffer1,
                                                                       NBGM_FloatBuffer& texcoordBuffer2,
                                                                       NBGM_ShortBuffer& indexBuffer);

    static PAL_Error NavVectorPolyline2Tristripe(NBRE_Array* polyline, float width, float texX, float texY, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoord1Buffer, NBGM_FloatBuffer& texcoord2Buffer, NBGM_ShortBuffer& indexBuffer);

    static PAL_Error NavEcmPolyline2Tristripe(const NBRE_Vector<NBRE_Point3f>& polyline, uint32 assistIndex, uint32 startIndex, uint32 count, float heightOffset,
                                             float halfRouteWidth, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer);

    static PAL_Error PathArrowPack2Tristripe(const NBRE_Vector<NBRE_Point2f>& polyline, float width, float length, float repeatDistance,
                                             NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer);

    static PAL_Error Track2Tristripe(const NBRE_Vector<NBRE_Point2f>& polyline, float width, NBGM_FloatBuffer& vertexBuffer, NBGM_ShortBuffer& indexBuffer);

    static PAL_Error PolygonPack2FloatArray(NBGM_ResourceContext& resourceContext, const NBRE_Point3d& tileCenter, NBRE_Array* polygon, NBGM_FloatBuffer& vertexBuffer,
                                            NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer);

    static PAL_Error HoleyPolygonPack2FloatArray(NBGM_ResourceContext& resourceContext, const NBRE_Point3d& tileCenter, NBRE_Array* polygon, NBGM_FloatBuffer& vertexBuffer,
        NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer);

    static PAL_Error PolygonWireFramePack2FloatArray(const NBRE_Point3d& tileCenter, NBRE_Array* polygon, NBGM_FloatBuffer& vertexBuffer,
                                                     NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer);

    static PAL_Error ConvexPolygonPack2FloatArray(NBGM_ResourceContext& resourceContext, const NBRE_Point3d& tileCenter, NBRE_Array* polygon, NBGM_FloatBuffer& vertexBuffer,
                                                 NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer);

    static PAL_Error BuildPolylineArrowCap(float width, float length, const NBRE_Point2f& lastPt,
                                           const NBRE_Point2f& prevLastPt, const NBRE_Point2f& texCoord,
                                           NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer,
                                           NBGM_FloatBuffer& texcoordBuffer1, NBGM_FloatBuffer& texcoordBuffer2,
                                           NBGM_ShortBuffer& indexBuffer);

    static PAL_Error BuildPolylineCircleCap(float radius, const NBRE_Point2f& lastPt, const NBRE_Point2f& texCoord,
                                            NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer,
                                            NBGM_FloatBuffer& texcoordBuffer1, NBGM_FloatBuffer& texcoordBuffer2,
                                            NBGM_ShortBuffer& indexBuffer);

    static PAL_Error BitmapPatternPolyline2Tristripe(NBGM_ResourceContext& rc,
                                                     NBRE_Array* polyline,
                                                     float width,
                                                     float length,
                                                     float repeatDistance,
                                                     float texRatio,
                                                     nb_boolean distanceIsInvalid,
                                                     NBGM_FloatBuffer& vertexBuffer,
                                                     NBGM_FloatBuffer& normalBuffer,
                                                     NBGM_FloatBuffer& texcoordBuffer,
                                                     NBGM_ShortBuffer& indexBuffer,
                                                     double& roadSegTotalLen);
};

class NBGM_BuildTextureUtility
{
public:

    static NBRE_TexturePtr ConstructRoadTexture(NBRE_IRenderPal& renderPal, const NBRE_Color& color, const NBRE_String& name);
    static NBRE_TexturePtr ConstructNavEcmRouteTexture(NBRE_IRenderPal& renderPal, const NBRE_Color& color, const NBRE_String& name);
    static NBRE_TexturePtr ConstructPathArrowTexture(NBRE_IRenderPal& renderPal, const NBRE_Color& color, float tailWidth, float headWidth, float tailLength, float headLength, const NBRE_String& name);
    static NBRE_TexturePtr ConstructColorTexture(NBRE_IRenderPal& renderPal, const NBRE_Color& c, const NBRE_String& name);
    static NBRE_TexturePtr ConstructImageTexture(NBRE_IRenderPal& renderPal, PAL_Instance* palInstance, const NBRE_String& name, uint32 offset, nb_boolean useLowQuality, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap);
    static NBRE_TexturePtr ConstructCommonTexture(NBRE_IRenderPal& renderPal, const NBRE_String& name, const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap);
    static NBRE_TexturePtr ConstructCommonTexture(NBRE_IRenderPal& renderPal, const NBRE_String& name, NBRE_ITextureImage* textureImage, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap);
    static NBRE_TexturePtr ConstructCommonTexture(NBRE_IRenderPal& renderPal, const NBRE_String& name, NBRE_Image** image, NBRE_Texture::TextureType texType, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap);
};

class NBGM_BuildShaderUtility
{
public:
    static NBRE_ShaderPtr CreatePolylineShader(NBRE_IRenderPal& renderPal, NBRE_Image* image);
};

/*! @} */

#endif
