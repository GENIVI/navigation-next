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

    @file nbrerenderpal.h
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
#ifndef _NBRE_RENDERPAL_H_
#define _NBRE_RENDERPAL_H_
#include "nbretypes.h"
#include "nbrevertexindexdata.h"
#include "nbreveiwport.h"
#include "nbregpuprogram.h"
#include "nbrematrix4x4.h"
#include "nbrelight.h"
#include "nbretexturestate.h"
#include "nbrerendercommon.h"
#include "nbretexture.h"

class NBRE_Texture;
class NBRE_RenderTarget;
class NBRE_Image;
class NBRE_ITextureImage;
class NBRE_Material;
struct NBRE_ImageInfo;

typedef struct NBRE_RenderConfig
{
    NBRE_RenderConfig():enableAntiAliasing(false),enableAnisotropicFiltering(false) {}

    NBRE_String fontFolderPath;
    bool enableAntiAliasing;
    bool enableAnisotropicFiltering;
}NBRE_RenderConfig;

/** \addtogroup NBRE_RenderPal
*  @{
*/
class NBRE_RenderPalCapabilities
{
public:
    NBRE_RenderPalCapabilities():
        maxTextureSize(1),
        maxTextureUnits(1),
        mMaxLights(1),
        mMaxStreams(1),
        mAutoMimmaps(FALSE),
        mSupportCubeMap(FALSE),
        mSupportVBO(FALSE),
        mSupportAnisotropy(FALSE),
        mSupportVAO(FALSE),
        mSupportMultipleSample(FALSE),
        mSupportNVPathRendering(FALSE),
        mSupportFBOAA(FALSE),
        mSupportDepthTexture(FALSE),
        mSupportHBAO(FALSE)
        {
        }
    ~NBRE_RenderPalCapabilities() {}

public:
    uint32 GetMaxTextureUnits() const { return maxTextureUnits; }
    void SetMaxTextureUnits(uint32 val) { maxTextureUnits = val; }
    uint32 GetMaxLights() const { return mMaxLights; }
    void SetMaxLights(uint32 val) { mMaxLights = val; }
    uint32 GetMaxStreams() const { return mMaxStreams; }
    void SetMaxStreams(uint32 val) { mMaxStreams = val; }
    void SetAutoMipmaps(nb_boolean supported) { mAutoMimmaps = supported; }
    nb_boolean GetAutoMipmaps()const { return mAutoMimmaps; }

    void SetMaxTextureSize(int32 val) { maxTextureSize = val; }
    int32 GetMaxTextureSize()const { return maxTextureSize; }

    void SetSupportCubeMap(nb_boolean val) {mSupportCubeMap = val;}
    nb_boolean GetSupportCubeMap() const {return mSupportCubeMap;}

    void SetSupportAnisotropy(nb_boolean val) { mSupportAnisotropy = val; }
    nb_boolean GetSupportAnisotropy() const { return mSupportAnisotropy; }

    void SetSupportVBO(nb_boolean val) {mSupportVBO = val;}
    nb_boolean GetSupportVBO() const {return mSupportVBO;}

    void SetSupportVAO(nb_boolean val) {mSupportVAO = val;}
    nb_boolean GetSupportVAO() const {return mSupportVAO;}

    void SetSupportMultipleSample(nb_boolean val) {mSupportMultipleSample = val;}
    nb_boolean GetSupportMultipleSample() const {return mSupportMultipleSample;}

    void SetSupporFBOAA(nb_boolean val) {mSupportFBOAA = val;}
    nb_boolean GetSupportFBOAA() const {return mSupportFBOAA;}

    void SetSupportNVPathRendering(nb_boolean val) {mSupportNVPathRendering = val;}
    nb_boolean GetSupportNVPathRendering() const {return mSupportNVPathRendering;}

    void SetSupportDepthTexture(nb_boolean val) { mSupportDepthTexture = val; }
    nb_boolean GetSupportDepthTexture() const { return mSupportDepthTexture; }

    void SetSupportHBAO(nb_boolean val) { mSupportHBAO = val; }
    nb_boolean GetSupportHBAO() const { return mSupportHBAO; }

private:
    int32 maxTextureSize;
    uint32 maxTextureUnits;
    uint32 mMaxLights;
    uint32 mMaxStreams;
    nb_boolean mAutoMimmaps;
    nb_boolean mSupportCubeMap;
    nb_boolean mSupportVBO;
    nb_boolean mSupportAnisotropy;
    nb_boolean mSupportVAO;
    nb_boolean mSupportMultipleSample;
    nb_boolean mSupportNVPathRendering;
    nb_boolean mSupportFBOAA;
    nb_boolean mSupportDepthTexture;
    nb_boolean mSupportHBAO;
};
/** Defines the functionality of a 3D API
    @remarks
    The RenderPal(Render Platform Abstract Layer) provides a base interface
    which abstracts the general functionality of the 3D API
    e.g. Direct3D or OpenGL. Whilst a few of the general
    methods have implementations, most of this class is
    abstract, requiring a subclass based on a specific API
    to be constructed to provide the full functionality.
*/
class NBRE_IRenderPal
{
public:
    virtual ~NBRE_IRenderPal() {}

    virtual nb_boolean BeginScene() {return TRUE;}
    virtual void EndScene() {;}
    virtual void Configure(const NBRE_RenderConfig& config) {;}

    /*! Create a hardware vertex buffer by render pal.
        @remarks
            Because different render pal may has different hardware buffer implementation,
            so this is the only way to create a pal specific hardware buffer.
        @param vertexSize The size in bytes of a single vertex in this buffer.
            If a vertex is made up by three point3f, then vertexSize = sizeof(float)*3*3
        @param numVertices The number of vertices in this buffer
        @param useShadowBuffer use shadow buffer or not
        @par
            Buffers have the ability to be 'shadowed' in system memory, this is because
            the kinds of access allowed on hardware buffers is not always as flexible as
            that allowed for areas of system memory - for example it is often either
            impossible, or extremely undesirable from a performance standpoint to read from
            a hardware buffer; when writing to hardware buffers, you should also write every
            byte and do it sequentially. In situations where this is too restrictive,
            it is possible to create a hardware, write-only buffer (the most efficient kind)
            and to back it with a system memory 'shadow' copy which can be read and updated arbitrarily.
            Ogre handles synchronizing this buffer with the real hardware buffer (which should still be
            created with the HBU_DYNAMIC flag if you intend to update it very frequently). Whilst this
            approach does have it's own costs, such as increased memory overhead, these costs can
            often be outweighed by the performance benefits of using a more hardware efficient buffer.
            You should look for the 'useShadowBuffer' parameter on the creation methods used to create
            the buffer of the type you require (see HardwareBufferManager) to enable this feature.
        @return NBRE_HardwareVertexBuffer* A pal specific hardware buffer instance created by render pal.
    */
    virtual NBRE_HardwareVertexBuffer* CreateVertexBuffer(uint32 vertexSize, uint32 numVertices, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage) = 0;

    /*!Create a hardware index buffer by render pal.
        @remarks
            Because different render pal may has different hardware buffer implementation,
            so this is the only way to create a pal specific hardware buffer.
        @param idxType Index type of this buffer
        @param numIndexes   Index number of this buffer
        @param useShadowBuffer use shadow buffer or not
        @par
            Buffers have the ability to be 'shadowed' in system memory, this is because
            the kinds of access allowed on hardware buffers is not always as flexible as
            that allowed for areas of system memory - for example it is often either
            impossible, or extremely undesirable from a performance standpoint to read from
            a hardware buffer; when writing to hardware buffers, you should also write every
            byte and do it sequentially. In situations where this is too restrictive,
            it is possible to create a hardware, write-only buffer (the most efficient kind)
            and to back it with a system memory 'shadow' copy which can be read and updated arbitrarily.
            Ogre handles synchronizing this buffer with the real hardware buffer (which should still be
            created with the HBU_DYNAMIC flag if you intend to update it very frequently). Whilst this
            approach does have it's own costs, such as increased memory overhead, these costs can
            often be outweighed by the performance benefits of using a more hardware efficient buffer.
            You should look for the 'useShadowBuffer' parameter on the creation methods used to create
            the buffer of the type you require (see HardwareBufferManager) to enable this feature.
        @return NBRE_HardwareIndexBuffer*  A pal specific hardware buffer instance created by render pal.
    */
    virtual NBRE_HardwareIndexBuffer* CreateIndexBuffer(NBRE_HardwareIndexBuffer::IndexType idxType, uint32 numIndexes, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage) = 0;
    virtual NBRE_VertexDeclaration* CreateVertexDeclaration(void) = 0;

    virtual void SetVertexBuffer(NBRE_VertexData* vtx) = 0;

    virtual void DrawIndexedPrimitive( NBRE_PrimitiveType type, const NBRE_IndexData* index) = 0;

    virtual void EnableBlend( nb_boolean bEnable ) = 0;
    virtual void EnableDepthTest( nb_boolean bEnable ) = 0;
    virtual void EnableAlphaTest( nb_boolean bEnable ) = 0;
    virtual void EnableStencilTest( nb_boolean bEnable ) = 0;
    virtual void EnableColorWrite( nb_boolean enableRed, nb_boolean enableGreen, nb_boolean enableBlue, nb_boolean enableAlpha ) = 0;
    virtual void EnableDepthWrite( nb_boolean bEnable ) = 0;
    virtual void EnableStencilWrite( uint32 maskBits ) = 0;
    virtual void EnableCullFace( nb_boolean bEnable ) = 0;
    virtual void EnableFog( nb_boolean bEnable ) = 0;

    virtual void SetDepthFunc(NBRE_CompareFunction f) = 0;
    virtual void SetAlphaFunc(NBRE_CompareFunction f, float refValue) = 0;
    virtual void SetFog(NBRE_FogMode mode, float density, float fogStart, float fogEnd, float red, float green, float blue, float alpha) = 0;
    virtual void EnableTexture( uint32 stage, nb_boolean bEnable ) = 0;
    virtual void DisableTextureUnitsFrom( uint32 stage ) = 0;
    virtual void SelectTexture( uint32 stage, NBRE_Texture* texture) = 0;
    virtual void SetTextureState(uint32 stage, const NBRE_TextureState& state) = 0;
    virtual void SetTextureTransform( uint32 stage, const NBRE_Matrix4x4f& mtrx ) = 0;
    virtual void SetTextureCoordSet(uint32 stage, uint32 index) = 0;
    virtual void SetWorldTransform( const NBRE_Matrix4x4d& mtrx ) = 0;
    virtual void SetViewTransform( const NBRE_Matrix4x4d& mtrx ) = 0;
    virtual void SetProjectionTransform( const NBRE_Matrix4x4f& mtrx ) = 0;
    virtual void SetBlendFunc(NBRE_BlendFunction srcFactor, NBRE_BlendFunction dstFactor) = 0;
    virtual void SetClearColor(float r, float g, float b, float a) = 0;
    virtual void SetClearDepth(float value) = 0;
    virtual void SetClearStencil(int32 value) = 0;
    virtual void Clear(uint32 flags) = 0;
    virtual void SetColor(float r, float g, float b, float a) = 0;

    virtual void SetViewPort(int32 left, int32 top, uint32 width , uint32 height) = 0;

    virtual void EnableLighting( nb_boolean bEnable ) = 0;
    virtual void UseLights( const NBRE_LightList& light ) = 0;
    virtual void SetActiveLights( const NBRE_LightIndexList& indices ) = 0;
    /// Create a render texture, immediate mode,
    virtual NBRE_Texture* CreateTexture(const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name) = 0;
    /// Create a general texture directly through images, immediate mode
    virtual NBRE_Texture* CreateTexture(NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap, NBRE_Texture::TextureType texType, const NBRE_String& name) = 0;
    /// Create a general texture through a texture image, delay mode
    virtual NBRE_Texture* CreateTexture(NBRE_ITextureImage* textureImage, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name) = 0;

    virtual void SetMaterial(const NBRE_Material& material) = 0;

    virtual NBRE_RenderTarget* CreateRenderTargetFromTexture(NBRE_Texture* texture,
                                                             int32 face,
                                                             nb_boolean useDepthTexture) = 0;

    virtual void SelectRenderTarget( NBRE_RenderTarget* surface ) = 0;
    virtual NBRE_RenderTarget* GetDefaultRenderTarget() = 0;

    virtual void LoadGpuProgram(const NBRE_String& name, const NBRE_String& vertShaderSrc, const NBRE_String& fragShaderSrc) = 0;
    virtual void SetGpuProgram(const NBRE_String& name) = 0;
    virtual void SetGpuProgramParam(const NBRE_String& name, const NBRE_GpuProgramParam& param) = 0;

    virtual const NBRE_RenderPalCapabilities& GetCapabilities() const = 0;
    virtual void SetWireframeMode(nb_boolean bLine) = 0;
    virtual void ReadPixels(int x, int y, uint32 width, uint32 height, uint8 *buffer) const = 0;

    // width is pixel value
    virtual void SetLineWidth(float width) = 0;
    virtual nb_boolean RenderAO(NBRE_RenderTarget* out, NBRE_RenderTarget* depth, NBRE_RenderTarget* blur, NBRE_RenderTarget* ao, const float* projMatrix, float metersToViewSpaceUnits, float radius, float bias, float powerExponent
                                ,float detailAO, float coarseAO, nb_boolean enableBlur, int blurRadius, float blurSharpness) { return FALSE; }
};

/** @} */
#endif
