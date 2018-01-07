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

    @file nbregles2renderpal.h
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
#ifndef _NBRE_GLES2_RENDERPAL_H_
#define _NBRE_GLES2_RENDERPAL_H_
#include "nbretypes.h"
#include "nbrerenderpal.h"
#include "palgl2.h"
#include "nbregles2gpuprogram.h"

class NBRE_GLES2RenderWindow;
class NBRE_GLES2GPUPorgramFactory;
struct PAL_Instance;
class NBRE_GLES2SSAOProgram;

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_GLES2RenderPal
*  @{
*/

/** Implementation of GLES2 as a rendering system. */
class NBRE_GLES2RenderPal: public NBRE_IRenderPal
{
public:
    NBRE_GLES2RenderPal(PAL_Instance* pal);
    ~NBRE_GLES2RenderPal();

public:
    //From NBRE_IRenderPal
    virtual void Configure(const NBRE_RenderConfig& config);
    virtual nb_boolean BeginScene();
    virtual void EndScene();

    virtual void SetMaterial(const NBRE_Material& material);
    virtual void SetVertexBuffer(NBRE_VertexData* vtx );
    virtual void DrawIndexedPrimitive( NBRE_PrimitiveType type, const NBRE_IndexData* index );

    /// Create a render texture, immediate mode, should be called in GL thread
    virtual NBRE_Texture* CreateTexture(const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name);
    /// Create a general texture directly through images, immediate mode, should be called in GL thread
    virtual NBRE_Texture* CreateTexture(NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap, NBRE_Texture::TextureType texType, const NBRE_String& name);
    /// Create a general texture, delay mode
    virtual NBRE_Texture* CreateTexture(NBRE_ITextureImage* textureImage, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name);

    virtual NBRE_HardwareIndexBuffer* CreateIndexBuffer( NBRE_HardwareIndexBuffer::IndexType idxType, uint32 numIndexes, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage);
    virtual NBRE_HardwareVertexBuffer* CreateVertexBuffer( uint32 vertexSize, uint32 numVertices, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage );

    virtual NBRE_RenderTarget* GetDefaultRenderTarget();

    virtual NBRE_RenderTarget* CreateRenderTargetFromTexture(NBRE_Texture* texture,
                                                             int32 face,
                                                             nb_boolean useDepthTexture);

    virtual void SelectRenderTarget( NBRE_RenderTarget* surface );

    virtual void SetViewPort( int32 left, int32 top, uint32 width , uint32 height );
    virtual void SetProjectionTransform( const NBRE_Matrix4x4f& mtrx );
    virtual void SetViewTransform( const NBRE_Matrix4x4d& mtrx );
    virtual void SetWorldTransform( const NBRE_Matrix4x4d& mtrx );
    virtual void SetDepthFunc(NBRE_CompareFunction f);
    virtual void SetAlphaFunc(NBRE_CompareFunction f, float refValue);
    virtual void SetFog(NBRE_FogMode mode, float density, float fogStart, float fogEnd, float red, float green, float blue, float alpha);
    virtual void EnableTexture( uint32 stage, nb_boolean bEnable );
    virtual void SelectTexture(uint32 stage, NBRE_Texture* texture);
    virtual void DisableTextureUnitsFrom( uint32 stage );
    virtual void SetTextureState(uint32 stage, const NBRE_TextureState& state);
    virtual void SetTextureTransform( uint32 stage, const NBRE_Matrix4x4f& mtrx );
    virtual void SetTextureCoordSet(uint32 stage, uint32 index);
    virtual void SetBlendFunc(NBRE_BlendFunction srcFactor, NBRE_BlendFunction dstFactor);
    virtual void SetClearColor(float r, float g, float b, float a);
    virtual void SetClearDepth(float value);
    virtual void SetClearStencil(int32 value);
    virtual void Clear(uint32 flags);
    virtual void SetColor(float r, float g, float b, float a);

    virtual void UseLights( const NBRE_LightList& light );
    virtual void SetActiveLights( const NBRE_LightIndexList& indices );

    virtual void EnableLighting( nb_boolean bEnable );
    virtual void EnableBlend( nb_boolean bEnable );
    virtual void EnableDepthTest( nb_boolean bEnable );
    virtual void EnableAlphaTest( nb_boolean bEnable ){;}
    virtual void EnableStencilTest( nb_boolean bEnable );
    virtual void EnableColorWrite( nb_boolean enableRed, nb_boolean enableGreen, nb_boolean enableBlue, nb_boolean enableAlpha );
    virtual void EnableDepthWrite( nb_boolean bEnable );
    virtual void EnableStencilWrite( uint32 maskBits );
    virtual void EnableCullFace( nb_boolean bEnable );
    virtual void EnableFog( nb_boolean bEnable );

    virtual void LoadGpuProgram(const NBRE_String& name, const NBRE_String& vertShaderSrc, const NBRE_String& fragShaderSrc);
    virtual const NBRE_RenderPalCapabilities& GetCapabilities() const { return mCapabilities; }
    virtual NBRE_VertexDeclaration* CreateVertexDeclaration(void);
    virtual void SetWireframeMode(nb_boolean bLine);
    virtual void ReadPixels(int x, int y, uint32 width, uint32 height, uint8 *buffer) const;

    // width is pixel value
    virtual void SetLineWidth(float width);

    virtual void SetGpuProgram(const NBRE_String& name);
    virtual void SetGpuProgramParam(const NBRE_String& name, const NBRE_GpuProgramParam& param);

    virtual nb_boolean RenderAO(NBRE_RenderTarget* out, NBRE_RenderTarget* depth, NBRE_RenderTarget* blur, NBRE_RenderTarget* ao, const float* projMatrix, float metersToViewSpaceUnits, float radius, float bias, float powerExponent
                                ,float detailAO, float coarseAO, nb_boolean enableBlur, int blurRadius, float blurSharpness);

    NBRE_GLES2GpuProgramPtr GetProgram(const NBRE_String& name);

private:
    void Initialize();
    void EnableNormalArray(nb_boolean enable);
    void EnableColorArray(nb_boolean enable);
    void EnableTexcoordArray(nb_boolean enable, uint32 index);
    void UpdateProgram(uint32 vertexFormat);
    void CreateDefaultTexture();
    void SelectDefaultTexture(uint32 stage);

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_GLES2RenderPal);

private:
    PAL_Instance* mPalInstance;
    NBRE_GLES2RenderWindow* mGlRenderWindow;
    NBRE_RenderTarget* mCurrentRenderTarget;
    NBRE_Matrix4x4d mWorldMatrix;
    NBRE_Matrix4x4d mViewMatrix;
    NBRE_Matrix4x4f mProjMatrix;
    NBRE_RenderPalCapabilities mCapabilities;
    GLenum* mTextureTypes;
    uint32* mTextureCoordIndex;
    nb_boolean mLastEnableNormalArray;
    nb_boolean mLastEnableColorArray;
    NBRE_Vector<nb_boolean> mLastEnableTexcoordArray;
    uint32 mDisabledTexUnitsFrom;
    uint32 mVertexFormat;
    float mLastTextureMatrix[8];
    struct
    {
        GLuint programId;
        GLint  matrixModelViewId;
        GLint  matrixProjectionId;
        GLint  matrixTextureId[2];
        GLint  textureSampler1;
        GLint  textureSampler2;
    }mCurrentShaderState;
    
	int32 mLastViewPort[4];
 
    NBRE_GLES2GPUPorgramFactory *mProgramFactory;
    GLfloat mMaxAnisotropy;
    nb_boolean mCullFace;
    nb_boolean mRedColorWrite;
    nb_boolean mGreenColorWrite;
    nb_boolean mBlueColorWrite;
    nb_boolean mAlphaColorWrite;
    nb_boolean mDepthTest;
    nb_boolean mDepthMask;
    nb_boolean mBlend;

    NBRE_CompareFunction mDepthFunction;
    NBRE_BlendFunction mSrcBlendFunction;
    NBRE_BlendFunction mDestBlendFunction;

    GLuint  mDefaultTextureID;
    GLuint  mTextureID;
    float   mTexCoordinateScaleX;
    float   mTexCoordinateScaleY;

    NBRE_String mGpuProgramName;
    bool    mEnableAntiAliasing;
    bool    mEnableAnisotropicFiltering;
    nb_boolean mHBAOInitDone;
    nb_boolean mHBAOInitOk;
    NBRE_GLES2SSAOProgram* mSSAOProgram;
};

/*! @} */
/*! @} */
#endif
