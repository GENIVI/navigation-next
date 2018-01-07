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
    @file nbgmmapmaterial.h
*/
/*
(C) Copyright 2011 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_MAP_MATERIAL_H_
#define _NBRE_MAP_MATERIAL_H_

#include "paltypes.h"
#include "nbretypes.h"
#include "nbrecolor.h"
#include "nbgmdom.h"
#include "palerror.h"
#include "nbretexture.h"
#include "nbreshader.h"
#include "nbrefontmaterial.h"
#include "nbrevector2.h"
#include "nbrecontext.h"
#include "nbreaxisalignedbox2.h"

struct NBRE_MapMaterialCreateContext;
class NBRE_TextureManager;
class NBRE_MapMateriaGroup;

/*! \addtogroup NBRE_Mapdata
*  @{
*/
/*! \addtogroup NBRE_MapMaterial
*  @{
*/

typedef enum NBRE_MapMaterialType
{
    NBRE_MMT_None,
    NBRE_MMT_StandardTexturedMaterial,
    NBRE_MMT_StandardColorMaterial,
    NBRE_MMT_OutlinedColorMaterial,
    NBRE_MMT_LightedMaterial,
    NBRE_MMT_FontMaterial,
    NBRE_MMT_LinePatternMaterial,
    NBRE_MMT_BackgroundMaterial,
    NBRE_MMT_ShieldMaterial,
    NBRE_MMT_DashOutlineMaterial,
    NBRE_MMT_PathArrowMaterial,
    NBRE_MMT_HorizontalLightSourceMaterial,
    NBRE_MMT_PinMaterial,
    NBRE_MMT_OutlinedComplexColorFillMaterial,
    NBRE_MMT_OutlinedSimpleColorFillMaterial,
    NBRE_MMT_GPINMaterial,
    NBRE_MMT_StaticPOIMaterial,
    NBRE_MMT_PolylineCapMaterial,
    NBRE_MMT_BitMapPatternMaterial,
    NBRE_MMT_OutlinedHoleyMaterial = 20
}NBRE_MapMaterialType;

/*! Defines an instance of a map material.
*/
class NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    virtual ~NBRE_MapMaterial(){}

public:
    NBRE_MapMaterialType Type() const {return mMaterialType;}
    uint16 ID() const {return mMaterialId;}

    int32 GetShaderCount() const
    {
        return mShaders.size();
    }

    NBRE_ShaderPtr GetShader(int32 index) const
    {
        return mShaders[index];
    }
    const NBRE_String& GetShaderName(int32 index) const
    {
        return mShaderNames[index];
    }

protected:
    NBRE_MapMaterial(NBRE_MapMaterialType type, uint16 id)
        :mMaterialType(type), mMaterialId(id), mLoaded(FALSE)
    {

    }

private:
    void Load(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial) = 0;

protected:
    NBRE_MapMaterialType        mMaterialType;
    uint16                      mMaterialId;

    NBRE_Vector<NBRE_String>    mShaderNames;
    NBRE_Vector<NBRE_ShaderPtr> mShaders;
    const NBRE_MapMateriaGroup* mDependGroup;

private:
    nb_boolean                  mLoaded;
};


/*! Defines an instance of an invalid material.
*/
class NBRE_MapNoneMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    explicit NBRE_MapNoneMaterial():NBRE_MapMaterial(NBRE_MMT_None, NBRE_INVALID_SHORT_INDEX)
    {
        mShaderNames.push_back("None shader");
        mShaders.push_back(NBRE_ShaderPtr((NBRE_Shader*)NULL));
    }
    ~NBRE_MapNoneMaterial(){}

public:
    const NBRE_String& ShaderName() const {return GetShaderName(0);}
    NBRE_ShaderPtr Shader() const {return GetShader(0);}


private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance*, NBRE_IRenderPal&, NBRE_TextureManager&, NBRE_DOM&, NBRE_DOM_Material&, uint16, nb_boolean){}
};

/*! Defines an instance of a standard textured material.
*/
class NBRE_MapStandardTexturedMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapStandardTexturedMaterial(){}

public:
    const NBRE_String& ShaderName() const {return GetShaderName(0);}
    NBRE_ShaderPtr Shader() const {return GetShader(0);}

private:
    explicit NBRE_MapStandardTexturedMaterial(uint16 id)
        :NBRE_MapMaterial(NBRE_MMT_StandardTexturedMaterial, id)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

};

/*! Defines an instance of a standard color material.
*/
class NBRE_MapStandardColorMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapStandardColorMaterial(){}
    static NBRE_MapStandardColorMaterial* CreateMemoryInstance(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, const char* domName, uint32 domColor, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    explicit NBRE_MapStandardColorMaterial(uint16 id)
        :NBRE_MapMaterial(NBRE_MMT_StandardColorMaterial, id){}

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);
    void LoadImplInternal(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, const char* domName, uint32 domColor, uint16 mtrlID, nb_boolean isCommomMaterial);
};

/*! Defines an instance of a standard color material.
*/
class NBRE_MapOutlinedColorsMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapOutlinedColorsMaterial(){}

public:
    static NBRE_ShaderPtr CreateShader(NBRE_IRenderPal& renderPal, NBRE_Image* image);
    NBRE_String ShaderName(){return mShaderNames[0];}

    float OutlineWidth() const {return mOutLineWidth;}

    const NBRE_Point2f& InteriorTexcoord()const { return mInteriorTexcoord; }
    const NBRE_Point2f& OutlineTexcoord()const { return mOutlineTexcoord; }

private:
    explicit NBRE_MapOutlinedColorsMaterial(uint16 id, NBRE_MapMaterialCreateContext* materialContext)
        :NBRE_MapMaterial(NBRE_MMT_OutlinedColorMaterial, id), mContext(materialContext), mOutLineWidth(0.f)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    NBRE_MapMaterialCreateContext* mContext;
    float mOutLineWidth;
    NBRE_Point2f mInteriorTexcoord;
    NBRE_Point2f mOutlineTexcoord;
};

/*! Defines an instance of a standard color material.
*/
class NBRE_MapLightedMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapLightedMaterial(){}

public:
    const NBRE_String& ShaderName() const {return GetShaderName(0);}
    NBRE_ShaderPtr Shader() const {return GetShader(0);}

private:
    explicit NBRE_MapLightedMaterial(uint16 id)
        :NBRE_MapMaterial(NBRE_MMT_LightedMaterial, id)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);
};

/*! Defines an instance of a pattern line material.
*/
class NBRE_MapPatternLineMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapPatternLineMaterial(){}

public:
    const NBRE_String& ShaderName() const{return GetShaderName(0);}
    NBRE_ShaderPtr Shader() const {return GetShader(0);}
    const NBRE_AxisAlignedBox2f& GetTexcoord() const { return mTexcoord;}

private:
    explicit NBRE_MapPatternLineMaterial(uint16 id, NBRE_MapMaterialCreateContext* materialContext)
        :NBRE_MapMaterial(NBRE_MMT_LinePatternMaterial, id), mContext(materialContext)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    NBRE_MapMaterialCreateContext* mContext;
    NBRE_AxisAlignedBox2f mTexcoord;
};

/*! Defines an instance of a dash line material.
*/
class NBRE_MapDashLineMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapDashLineMaterial(){}

public:
    const NBRE_String& ShaderName() const{return GetShaderName(0);}
    NBRE_ShaderPtr Shader() const {return GetShader(0);}
    const NBRE_AxisAlignedBox2f& GetTexcoord() const { return mTexcoord; }

private:
    explicit NBRE_MapDashLineMaterial(uint16 id, NBRE_MapMaterialCreateContext* materialContext)
        :NBRE_MapMaterial(NBRE_MMT_DashOutlineMaterial, id), mContext(materialContext)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    NBRE_MapMaterialCreateContext* mContext;
    NBRE_AxisAlignedBox2f mTexcoord;
};

/*! Defines an instance of a font material.
*/
class NBRE_MapFontMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapFontMaterial(){}

public:
    const NBRE_FontMaterial& GetFontMaterial() const { return mFontMaterial; }
    nb_boolean IsWatermark() const { return mIsWatermark; }

private:
    explicit NBRE_MapFontMaterial(uint16 id)
        :NBRE_MapMaterial(NBRE_MMT_FontMaterial, id)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    NBRE_FontMaterial mFontMaterial;
    nb_boolean mIsWatermark;
};


/*! Defines an instance of a shield material.
*/
class NBRE_MapShieldMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapShieldMaterial(){}

public:
    uint16 GetFontMaterialIndex() const { return mFontMaterialIndex; }
    NBRE_ShaderPtr GetImageShader() const { return mImageShader; }
    const NBRE_Vector2f& GetOffset() const { return mOffset; }
    const NBRE_Vector2f& GetPadding() const { return mPadding; }

private:
    explicit NBRE_MapShieldMaterial(uint16 id)
        :NBRE_MapMaterial(NBRE_MMT_ShieldMaterial, id), mFontMaterialIndex(NBRE_INVALID_SHORT_INDEX)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    NBRE_ShaderPtr mImageShader;
    uint16 mFontMaterialIndex;
    NBRE_Vector2f mOffset;
    NBRE_Vector2f mPadding;
};

/*! Defines an instance of a horizontal light source material.
*/
class NBRE_MapHorizontalLightSourceMaterial: public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapHorizontalLightSourceMaterial(){}

public:
    const NBRE_String& ShaderName() const {return GetShaderName(0);}
    NBRE_ShaderPtr Shader() const {return GetShader(0);}

private:
    explicit NBRE_MapHorizontalLightSourceMaterial(uint16 id)
        :NBRE_MapMaterial(NBRE_MMT_HorizontalLightSourceMaterial, id)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

};

/*! Defines an instance of a background material.
*/
class NBRE_MapBackgroundColorMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;
public:
    ~NBRE_MapBackgroundColorMaterial(){}

public:
    const NBRE_String& ShaderName() const {return GetShaderName(0);}
    NBRE_ShaderPtr Shader() const {return GetShader(0);}
    NBRE_Color GetColor(){return mColor;}

private:
    explicit NBRE_MapBackgroundColorMaterial(uint16 id, NBRE_MapMaterialCreateContext* /*materialContext*/)
        :NBRE_MapMaterial(NBRE_MMT_BackgroundMaterial, id)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    NBRE_Color mColor;
};

/*! Defines an instance of a path arrow material.
*/
class NBRE_MapPathArrowMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;

public:
    ~NBRE_MapPathArrowMaterial(){}

public:
    const NBRE_String& ShaderName() const{return GetShaderName(0);}
    NBRE_ShaderPtr Shader() const {return GetShader(0);}

    float GetLength() const {return mLength;}
    float GetRepeat() const {return mRepeat;}

private:
    explicit NBRE_MapPathArrowMaterial(uint16 id) :NBRE_MapMaterial(NBRE_MMT_PathArrowMaterial, id), mLength(0), mRepeat(0){}

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    float mLength;
    float mRepeat;
};

class NBRE_PinMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;

public:
    ~NBRE_PinMaterial(){}

public:
    const NBRE_String& ShaderName() const{return GetShaderName(0);}
    NBRE_ShaderPtr Shader() const {return GetShader(0);}

    float XImageOffset() const {return mXImageOffset;}
    float YImageOffset() const {return mYImageOffset;}
    float XBubbleOffset() const {return mXBubbleOffset;}
    float YBubbleOffset() const {return mYBubbleOffset;}

private:
    explicit NBRE_PinMaterial(uint16 id):
        NBRE_MapMaterial(NBRE_MMT_PinMaterial, id),
        mXImageOffset(50),
        mYImageOffset(100),
        mXBubbleOffset(50),
        mYBubbleOffset(0)
        {
        }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    float mXImageOffset;
    float mYImageOffset;
    float mXBubbleOffset;
    float mYBubbleOffset;
};

class NBRE_OutlinedComplexColorFillMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;

public:
    ~NBRE_OutlinedComplexColorFillMaterial(){}

public:
    const NBRE_String& InteriorShaderName() const{return GetShaderName(0);}
    NBRE_ShaderPtr InteriorShader() const {return GetShader(0);}

    const NBRE_String& OutlinedShaderName() const{return GetShaderName(1);}
    NBRE_ShaderPtr OutlinedShader() const {return GetShader(1);}

private:
    explicit NBRE_OutlinedComplexColorFillMaterial(uint16 id):
        NBRE_MapMaterial(NBRE_MMT_OutlinedComplexColorFillMaterial, id){}

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);
};

class NBRE_OutlinedSimpleColorFillMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;

public:
    ~NBRE_OutlinedSimpleColorFillMaterial(){}

public:
    const NBRE_String& InteriorShaderName() const{return GetShaderName(0);}
    NBRE_ShaderPtr InteriorShader() const {return GetShader(0);}

    const NBRE_String& OutlinedShaderName() const{return GetShaderName(1);}
    NBRE_ShaderPtr OutlinedShader() const {return GetShader(1);}

private:
    explicit NBRE_OutlinedSimpleColorFillMaterial(uint16 id):
        NBRE_MapMaterial(NBRE_MMT_OutlinedSimpleColorFillMaterial, id){}

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);
};


class NBRE_OutlinedHoleyMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;

public:
    ~NBRE_OutlinedHoleyMaterial(){}

public:
    const NBRE_String& InteriorShaderName() const{return GetShaderName(0);}
    NBRE_ShaderPtr InteriorShader() const {return GetShader(0);}

    const NBRE_String& OutlinedShaderName() const{return GetShaderName(1);}
    NBRE_ShaderPtr OutlinedShader() const {return GetShader(1);}

private:
    explicit NBRE_OutlinedHoleyMaterial(uint16 id):
    NBRE_MapMaterial(NBRE_MMT_OutlinedHoleyMaterial, id){}

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);
};

class NBRE_RadialPINMaterial : public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;

public:
    ~NBRE_RadialPINMaterial(){}

public:
    const NBRE_String& NormalShaderName() const { return GetShaderName(0); }
    NBRE_ShaderPtr NormalShader() const { return GetShader(0); }

    const NBRE_String& SelectedShaderName() const { return GetShaderName(1); }
    NBRE_ShaderPtr SelectedShader() const { return GetShader(1); }

    const NBRE_String& NormalHaloInteriorShaderName() const { return GetShaderName(2); }
    NBRE_ShaderPtr NormalHaloInteriorShader() const { return GetShader(2); }

    const NBRE_String& NormalHaloOutlineShaderName() const { return GetShaderName(3); }
    NBRE_ShaderPtr NormalHaloOutlineShader() const { return GetShader(3); }

    const NBRE_String& SelectedHaloInteriorShaderName() const { return GetShaderName(4); }
    NBRE_ShaderPtr SelectedHaloInteriorShader() const { return GetShader(4); }

    const NBRE_String& SelectedHaloOutlineShaderName() const { return GetShaderName(5); }
    NBRE_ShaderPtr SelectedHaloOutlineShader() const { return GetShader(5); }

    float NormalXImageOffset() const { return mNormalXImageOffset; }
    float NormalYImageOffset() const { return mNormalYImageOffset; }
    float NormalXBubbleOffset() const { return mNormalXBubbleOffset; }
    float NormalYBubbleOffset() const { return mNormalYBubbleOffset; }
    float GetNormalWidth() const { return mNormalWidth;}
    float GetNormalHeight() const { return mNormalHeight;}

    float SelectedXImageOffset() const { return mSelectedXImageOffset; }
    float SelectedYImageOffset() const { return mSelectedYImageOffset; }
    float SelectedXBubbleOffset() const { return mSelectedXBubbleOffset; }
    float SelectedYBubbleOffset() const { return mSelectedYBubbleOffset; }
    float GetSelectedWidth() const { return mSelectedWidth;}
    float GetSelectedHeight() const { return mSelectedHeight;}

private:
    explicit NBRE_RadialPINMaterial(uint16 id):
        NBRE_MapMaterial(NBRE_MMT_GPINMaterial, id),
        mNormalXImageOffset(50.0f),
        mNormalYImageOffset(100.0f),
        mNormalXBubbleOffset(50.0f),
        mNormalYBubbleOffset(0.0f),
        mNormalWidth(0.0f),
        mNormalHeight(0.0f),
        mSelectedXImageOffset(50.0f),
        mSelectedYImageOffset(100.0f),
        mSelectedXBubbleOffset(50.0f),
        mSelectedYBubbleOffset(0.0f),
        mSelectedWidth(0.0f),
        mSelectedHeight(0.0f)
        {
        }

private:
    NBRE_ShaderPtr CreateHaloShader(NBRE_IRenderPal& renderPal, const NBRE_Color& clr, const NBRE_String& texName, float lineWidth);
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    float mNormalXImageOffset;
    float mNormalYImageOffset;
    float mNormalXBubbleOffset;
    float mNormalYBubbleOffset;
    float mNormalWidth;
    float mNormalHeight;
    float mSelectedXImageOffset;
    float mSelectedYImageOffset;
    float mSelectedXBubbleOffset;
    float mSelectedYBubbleOffset;
    float mSelectedWidth;
    float mSelectedHeight;
};

class NBRE_StaticPOIMaterial : public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;

public:
    ~NBRE_StaticPOIMaterial(){}

public:
    const NBRE_String& NormalShaderName() const { return GetShaderName(0); }
    NBRE_ShaderPtr NormalShader() const { return GetShader(0); }

    const NBRE_String& SelectedShaderName() const { return GetShaderName(1); }
    NBRE_ShaderPtr SelectedShader() const { return GetShader(1); }

    const NBRE_String& UnselectedShaderName() const { return GetShaderName(2); }
    NBRE_ShaderPtr UnselectedShader() const { return GetShader(2); }

    float XImageOffset() const { return mXImageOffset; }
    float YImageOffset() const { return mYImageOffset; }
    float XBubbleOffset() const { return mXBubbleOffset; }
    float YBubbleOffset() const { return mYBubbleOffset; }
    float DistanceToAnother() const { return mDistanceToAnother; }
    float DistanceToLabel() const { return mDistanceToLabel; }
    float DistanceToPOI() const { return mDistanceToPOI; }
    float IconWidth() const { return mIconWidth; }
    float IconHeight() const { return mIconHeight; }

private:
    explicit NBRE_StaticPOIMaterial(uint16 id)
        :NBRE_MapMaterial(NBRE_MMT_StaticPOIMaterial, id)
        ,mXImageOffset(0.0f)
        ,mYImageOffset(0.0f)
        ,mXBubbleOffset(0.0f)
        ,mYBubbleOffset(0.0f)
        ,mDistanceToAnother(0.0f)
        ,mDistanceToLabel(0.0f)
        ,mDistanceToPOI(0.0f)
        ,mIconWidth(0.0f)
        ,mIconHeight(0.0f)
        {
        }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);

private:
    float mXImageOffset;
    float mYImageOffset;
    float mXBubbleOffset;
    float mYBubbleOffset;
    float mDistanceToAnother;
    float mDistanceToLabel;
    float mDistanceToPOI;
    float mIconWidth;
    float mIconHeight;
};

enum NBGM_PolylineCapType
{
    NBGM_PCT_Arrow = 0,
    NBGM_PCT_Circle,
    NBGM_PCT_LineJoin
};

class NBRE_PolylineCapMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;

public:
    ~NBRE_PolylineCapMaterial(){}

public:
    NBGM_PolylineCapType GetStartCap() const {return mStartType;}
    NBGM_PolylineCapType GetEndCap() const {return mEndType;}

    NBRE_String ShaderName(){return mShaderNames[0];}
    NBRE_ShaderPtr GetInteriorShader(){return GetShader(0);}

    const NBRE_Point2f& InteriorTexcoord()const { return mInteriorTexcoord; }
    const NBRE_Point2f& OutlineTexcoord()const { return mOutlineTexcoord; }

    float GetOutLineWidthPercentage() const {return mOutLineWidthPercentage;}
    float GetShapeHeightPercentage() const {return mShapeHeightPercentage;}
    float GetShapeWidthPercentage() const {return mShapeWidthPercentage;}
    float GetRadiusPercentage() const {return mRadiusPercentage;}

private:
    explicit NBRE_PolylineCapMaterial(uint16 id, NBRE_MapMaterialCreateContext* materialContext)
        :NBRE_MapMaterial(NBRE_MMT_PolylineCapMaterial, id), mContext(materialContext)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager,
                          NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);
    NBGM_PolylineCapType GetCapType(uint8 type) const;

private:
    NBRE_MapMaterialCreateContext* mContext;
    NBGM_PolylineCapType mStartType;
    NBGM_PolylineCapType mEndType;
    NBRE_Point2f mInteriorTexcoord;
    NBRE_Point2f mOutlineTexcoord;
    float mOutLineWidthPercentage;
    float mShapeHeightPercentage;
    float mShapeWidthPercentage;
    float mRadiusPercentage;
};

class NBRE_BitMapPatternMaterial:public NBRE_MapMaterial
{
    friend class NBRE_MapMaterialFactory;

public:
    ~NBRE_BitMapPatternMaterial(){}

public:
    NBRE_String ShaderName(){return mShaderNames[0];}
    NBRE_ShaderPtr Shader(){return GetShader(0);}

    const NBRE_Color& GetColor() {return mColor;}
    uint16 GetDistance() const {return mDistance;}
    uint16 GetWidth() const {return mWidth;}
    uint16 GetHeight() const {return mHeight;}

private:
    explicit NBRE_BitMapPatternMaterial(uint16 id, NBRE_MapMaterialCreateContext* materialContext)
        :NBRE_MapMaterial(NBRE_MMT_BitMapPatternMaterial, id), mContext(materialContext)
    {
    }

private:
    /// Inherited from NBRE_MapMaterial
    virtual void LoadImpl(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager,
                          NBRE_DOM& dom, NBRE_DOM_Material& domMaterial, uint16 mtrlID, nb_boolean isCommomMaterial);
private:
    NBRE_MapMaterialCreateContext* mContext;
    NBRE_Color mColor;
    uint16 mDistance;
    uint16 mWidth;
    uint16 mHeight;
};

/** @} */
/** @} */
#endif
