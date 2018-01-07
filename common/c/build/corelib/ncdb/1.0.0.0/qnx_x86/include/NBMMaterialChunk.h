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

@file     NBMMaterialChunk.h
@date     11/01/2011
@defgroup MOBIUS_NBM NavBuilder Binary Model API
@author   Sevnsson Sun

*/
/*
(C) Copyright 2012 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
#include "NBMChunk.h"

class BufferConverter;

typedef enum
{
    MaterialType_StandardTexture = 0,           // Standard Textured Material
    MaterialType_StandardColor = 1,             // Standard Color Material
    MaterialType_OutlinedColor = 2,             // Outlined Color Material
    MaterialType_Light = 3,                     // Lighted Material
    MaterialType_Font = 4,                      // Font Material
    MaterialType_PatternLine = 5,               // Pattern Line Material
    MaterialType_Background = 6,                // Background Material
    MaterialType_Shield = 7,                    // Shield Material
    MaterialType_DashOutline = 8,               // Dash Outline Material
    MaterialType_PathArrow = 9,                 // Path Arrow Material
    MaterialType_FourSidesGradient = 10,        // Four sides gradient Material
    MaterialType_BackCompatible = 255,          // Back compatible for new material types
	MaterialType_Pin =11,                       // Pin Material
    MaterialType_AreaOutlinedColor = 12,        // Area with outline
    MaterialType_AreaOutlinedColor_Simple = 13, // Area with outline for simple polygon
    MaterialType_ShieldY = 14,                  // Shield with Ygap Material
    MaterialType_NewPin = 15,                      // new pin material
//    MaterialType_BiOutlined = 16,                      // BiOutlined material
	MaterialType_POI = 16,						// POI material
    MaterialType_PolylineCap = 17,                   // Maneuver Mark material
    MaterialType_BmpPatternForLine = 18,        // bitmap pattern for line material
    MaterialType_AreaDashOutlined = 19,         // Area with dash outline
    MaterialType_AreaOutlinedColor_Holey = 20   // Area with outline for polygon with hole
}MaterialType;

inline void SetRealMatiralIndex(unsigned short uRealIndex,bool bExternal,unsigned short& uIndex)
{
	if (bExternal)
	{
		uIndex = uRealIndex | 0x8000;
	}
	else 
	{
		uIndex = uRealIndex;
	}
}

inline void GetRealMatiralIndex(unsigned short uIndex,unsigned short& uRealIndex, bool& bExternal)
{
	if (uIndex & 0x8000)
	{
		uRealIndex = uIndex & 0x7fff;
		bExternal = true;
	}
	else
	{
		uRealIndex = uIndex;
		bExternal = false;
	}
}

struct TextureBitMapBody;

struct MaterialBody
{
	MaterialBody(MaterialType type)
	{
		m_type = (unsigned char)type;
        m_bufSize = 0;
		m_backupIndex = NBM_INVALIDATE_INDEX;
		m_buf = NULL;
	}
	~MaterialBody()
	{
		if (m_buf)
		{
			delete[] m_buf;
            m_buf = NULL;
            m_bufSize = 0;
			m_backupIndex = NBM_INVALIDATE_INDEX;
		}
	}

	unsigned int BaseSize()
	{
		return sizeof(m_type) + sizeof(m_bufSize) + sizeof(m_backupIndex);
	}

	unsigned char  m_type;
    unsigned char  m_bufSize;       // Buffer size for m_buf, not more than 0xFF
	unsigned short m_backupIndex;   // Back compatibility for new material types.
	char*  m_buf;           // Buffer for material

	unsigned char Type()
	{
		return m_type;
	}
};

//! type 0 Standard Textured Material
struct MaterialTextureBody : public MaterialBody
{
	MaterialTextureBody() 
      : MaterialBody(MaterialType_StandardTexture),
        color(-1),
        index(NBM_INVALIDATE_INDEX)
	{
    }

	unsigned int color;
	NBMIndex index;

	unsigned int Size()
	{
		return BaseSize() + sizeof(color) + sizeof(index);
	}
	char* ToBuf();
	int FromBuf(unsigned char* buf);
};

//! type 1 Standard Color
struct MaterialColorBody : public MaterialBody
{
	MaterialColorBody() 
      : MaterialBody(MaterialType_StandardColor),
        color(-1)
	{
	}

	unsigned int color;

	unsigned int Size()
	{
		return BaseSize() + sizeof(color);
	}
	char* ToBuf();
	int FromBuf(unsigned char* buf);
};

//! type 2 Outlined Color Material
struct MaterialOutlinedBody : public MaterialBody
{
	MaterialOutlinedBody()
      : MaterialBody(MaterialType_OutlinedColor),
        interiorColor(-1),
        outlineColor(-1),
        outlineWidth(0)
	{
	}

	unsigned int interiorColor;
	unsigned int outlineColor;
	unsigned char  outlineWidth;

	unsigned int Size()
	{
		return BaseSize() + sizeof(interiorColor) 
			+ sizeof(outlineColor) + sizeof(outlineWidth);
	}
	char* ToBuf();
	int FromBuf(unsigned char* buf);
};

//! type 3 Lighted Material
//! not ready yet
struct MaterialLightedBody : public MaterialBody
{
	MaterialLightedBody() 
      : MaterialBody(MaterialType_Light),
        color(-1),
        texture(NBM_INVALIDATE_INDEX)
	{
	}

	unsigned int color;
	NBMIndex texture;

	unsigned int Size()
	{
		return 0;
	}
};

//! type 4 Font Material
// Specifies the font family:
// 0 = sans-serif
// 1 = serif
// The font style of the text:
// 0 = normal
// 1 = bold
struct MaterialFontBody : public MaterialBody
{
	MaterialFontBody() 
      : MaterialBody(MaterialType_Font),
        color(-1),
        outlineColor(-1),
        outlineWidth(0),
        fontFamily(0),
        fontStyle(0),
        optimalSize(0),
        minimumSize(0),
        maximumSize(0),
        letterSpace(0)
	{
	}

	unsigned int color;
	unsigned int outlineColor;
	unsigned char outlineWidth;
	unsigned char fontFamily;
	unsigned char fontStyle;
	unsigned char optimalSize;
	unsigned char minimumSize;
	unsigned char maximumSize;
	unsigned char letterSpace;

	unsigned int Size()
	{
		return BaseSize() + sizeof(unsigned int) * 2 + sizeof(unsigned char) * 7;
	}
	char* ToBuf();
	int FromBuf(unsigned char* buf);
};

//! type 5 Pattern Line Material
struct MaterialPatternLineBody : public MaterialBody
{
	MaterialPatternLineBody() 
      : MaterialBody(MaterialType_PatternLine),
        BitOnColor(-1),
        BitOffColor(-1),
        outlineColor(-1),
        interiorPattern(0),
        width(0)
	{
	}

	unsigned int BitOnColor;
	unsigned int BitOffColor;
	unsigned int outlineColor;
	unsigned int interiorPattern;
	unsigned char  width;

	unsigned int Size()
	{
		return BaseSize() + sizeof(unsigned int) * 4 + sizeof(width);
	}
	char* ToBuf();
	int FromBuf(unsigned char* buf);
};

//! type 6 Background Material
struct MaterialBackgroundBody : public MaterialTextureBody
{
    MaterialBackgroundBody()
    {
        m_type = MaterialType_Background;
    }
};

//! type 7: Shield Material
struct MaterialShieldBody : public MaterialBody
{
    MaterialShieldBody() 
      : MaterialBody(MaterialType_Shield),
        fontIndex(NBM_INVALIDATE_INDEX),
        iconIndex(NBM_INVALIDATE_INDEX),
        xOffset(0),
        yOffset(0),
        edgePadding(0)
    {       
    }

    NBMIndex fontIndex; // Index of a referenced font meterial
    NBMIndex iconIndex; // Index of a referenced image or bitmap meterial
    signed char  xOffset;      // Text x center offset.
    signed char  yOffset;      // Text y center offset.
    unsigned char edgePadding;  // Padding space to not display text near icon edge.

    unsigned int Size()
    {
        return BaseSize() + sizeof(NBMIndex) * 2 + sizeof(signed char) * 2 + sizeof(unsigned char);
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);
};

//! type 14: Shield with Ygap Material
struct MaterialShieldYBody : public MaterialBody
{
    MaterialShieldYBody() 
      : MaterialBody(MaterialType_ShieldY),
        fontIndex(NBM_INVALIDATE_INDEX),
        iconIndex(NBM_INVALIDATE_INDEX),
        xOffset(0),
        yOffset(0),
        edgePadding(0),
        edgePaddingY(0)
    {       
    }

    NBMIndex fontIndex; // Index of a referenced font meterial
    NBMIndex iconIndex; // Index of a referenced image or bitmap meterial
    signed char  xOffset;      // Text x center offset.
    signed char  yOffset;      // Text y center offset.
    unsigned char edgePadding;  // Xgap
    unsigned char edgePaddingY;  // Ygap

    unsigned int Size()
    {
        return BaseSize() + sizeof(NBMIndex) * 2 + sizeof(signed char) * 2 + sizeof(unsigned char) * 2;
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);
};

//! type 8: Dash Outline Material
struct MaterialDashOutlineBody : public MaterialBody
{
    MaterialDashOutlineBody() 
      : MaterialBody(MaterialType_DashOutline),
        bitOnColor(-1),
        bitOffColor(-1),
        outlinePattern(0),
        interiorColor(-1),
        outlineWidth(0)
    {       
    }

    unsigned int bitOnColor;      // Outline Bit On Color, in b, g, r, a order.
    unsigned int bitOffColor;     // Outline Bit On Color, in b, g, r, a order.
    unsigned int outlinePattern;  // Outline pattern, specified as a 32-bit bitmask.
    unsigned int interiorColor;   // Line interior color, in b, g, r, a order
    unsigned char  outlineWidth;    // Outline width, as a % of the object width

    unsigned int Size()
    {
        return BaseSize() + sizeof(unsigned int) * 4 + sizeof(unsigned char);
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);
};

//! type 9: Path Arrow Material
struct MaterialPathArrowBody : public MaterialBody
{
    MaterialPathArrowBody() 
      : MaterialBody(MaterialType_PathArrow),
        arrowColor(-1),
        tailWidth(0),
        tailLength(0),
        headWidth(0),
        headLenght(0),
        repeatCount(0)
    {       
    }

    unsigned int arrowColor;  // Arrow Color, in b, g, r, a order.
    unsigned char  tailWidth;   // Arrow tail maximum width, specified in points (1/72?.
    unsigned char  tailLength;  // Arrow tail maximum length in pixels, specified in points (1/72?.
    unsigned char  headWidth;   // Arrow head maximum width in pixels, specified in points (1/72?.
    unsigned char  headLenght;  // Arrow head maximum length in pixels, specified in points (1/72?.
    unsigned char  repeatCount; // How often to repeat arrows along a road

    unsigned int Size()
    {
        return BaseSize() + sizeof(unsigned int) + sizeof(unsigned char) * 5;
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);
};

//! type 10 Four side gradient material
struct MaterialFourSidesGradientBody : public MaterialBody
{
    MaterialFourSidesGradientBody() 
        : MaterialBody(MaterialType_FourSidesGradient),
          lightAngle(0),
          degree0Color(-1),
          degree180Color(-1),
          degree270Color(-1),
          degree90Color(-1),
          topColor(-1),
          bottomColor(-1),
          transparency(0)
    {
    }

    float  lightAngle;      // Angle in radians
    unsigned int degree0Color;    // Gradient color at 0 degree, Color in b, g, r order.
    unsigned int degree180Color;  // Gradient color at 180 degree, Color in b, g, r order.
    unsigned int degree270Color;  // Gradient color at 270 degree, Color in b, g, r order.
    unsigned int degree90Color;   // Gradient color at 90 degree, Color in b, g, r order.
    unsigned int topColor;        // Top color, Color in b, g, r order.
    unsigned int bottomColor;     // Bottom color, Color in b, g, r order.
    unsigned char  transparency;    // Alpha color for gradient and top.

    unsigned int Size()
    {
        // Save lightAngle as half, and save color with 3 bytes
        return BaseSize() + sizeof(unsigned short) + sizeof(unsigned char) * 3 * 6 + sizeof(unsigned char);
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);

private:
    void SaveBGRColor(BufferConverter& converter, unsigned int color);
    unsigned int ReadBGRColor(BufferConverter& converter);
};

//! type 15 NewPin material
struct MaterialNewPinBody : public MaterialBody
{
    MaterialNewPinBody()
        : MaterialBody(MaterialType_NewPin),
        unselectedBmp(NBM_INVALIDATE_INDEX),
        selectedBmp(NBM_INVALIDATE_INDEX),
        unselectedCxOffset(0),
        unselectedCyOffset(0),
        unselectedBxOffset(0),
        unselectedByOffset(0),
        selectedCxOffset(0),
        selectedCyOffset(0),
        selectedBxOffset(0),
        selectedByOffset(0),
        selectedInteriorColor(-1),
        unselectedInteriorColor(-1),
        unselectedImageWidth(0),
        unselectedImageHeight(0),
        selectedImageWidth(0),
        selectedImageHeight(0),
        outlineWidth(0),
        selectedBitOnColor(-1),
        selectedBitOffColor(-1),
        unselectedBitOnColor(-1),
        unselectedBitOffColor(-1),
        pattern(0)
    {       
    }

    NBMIndex unselectedBmp; // Index into "IMGS" or "BMAP" chunk
    NBMIndex selectedBmp; // msb:=0 then IMGS chunk; msb:=1 then BMAP chunk
    signed char unselectedCxOffset;      // Unselected Callout x center offset.
    signed char unselectedCyOffset;      // Unselected Callout y center offset.
    signed char unselectedBxOffset;      // Unselected Bubble x center offset
    signed char unselectedByOffset;      // Unselected Bubble y center offset
    signed char selectedCxOffset;      // Selected Callout x center offset.
    signed char selectedCyOffset;      // Selected Callout y center offset.
    signed char selectedBxOffset;      // Selected Bubble x center offset
    signed char selectedByOffset;      // Selected Bubble y center offset
    Uint32_t selectedInteriorColor;  // b.g.r.a order
    Uint32_t unselectedInteriorColor;  // b.g.r.a order
    float unselectedImageWidth;   // unselected pin width in DPI
    float unselectedImageHeight;  // unselected pin height in DPI
    float selectedImageWidth;   // selected pin width in DPI
    float selectedImageHeight;  // selected pin height in DPI
    Uint8_t outlineWidth;
    Uint32_t selectedBitOnColor;   // selected Bit On Color in b,g,r,a order
    Uint32_t selectedBitOffColor;  // selected Bit Off Color in b,g,r,a order
    Uint32_t unselectedBitOnColor; // unselected Bit On Color in b,g,r,a order
    Uint32_t unselectedBitOffColor;// unselected Bit Off Color in b,g,r,a order
    Uint32_t pattern;           // pin pattern

    unsigned int Size()
    {
        return BaseSize() + sizeof(NBMIndex)*2 + sizeof(signed char)*8 + sizeof(Uint32_t)*7 + sizeof(float)*4 + sizeof(Uint8_t);
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);
};

// type 16 POI material
struct MaterialPOIBody: public MaterialBody
{
	MaterialPOIBody(): MaterialBody(MaterialType_POI),
					   icon(NBM_INVALIDATE_INDEX),
					   selectedPin(NBM_INVALIDATE_INDEX),
					   unselectedPin(NBM_INVALIDATE_INDEX),
					   cxOffset(0),
					   cyOffset(0),
					   bxOffset(0),
					   byOffset(0),
					   distanceToObj(0),
					   distanceToLab(0),
					   distanceToPoi(0),
                       width(0),
                       height(0)
	{
	}

	NBMIndex icon;				// image for POI icon
	NBMIndex selectedPin;		// image for selected pin
	NBMIndex unselectedPin;		// image for unselected pin
	signed char cxOffset;		// callout x center offset
	signed char cyOffset;		// callout y center offset
	signed char bxOffset;		// bubble x center offset
	signed char byOffset;		// bubble y center offset
	signed char distanceToObj;  // Minimum distance to another object in dp
	signed char distanceToLab;  // Minimum distance to another label in dp
	signed char distanceToPoi;  // Minimum distance to another poi in dp
    Uint8_t width;                // icon width in pixel
    Uint8_t height;               // icon height in pixel

	unsigned int Size()
	{
		return BaseSize() + sizeof(NBMIndex)*3 + sizeof(signed char)*7 + sizeof(Uint8_t)*2;
	}
	char* ToBuf();
	int FromBuf(unsigned char* buf);
};

// type 17 polyline mark material
struct MaterialPolylineCapBody: public MaterialBody
{
    MaterialPolylineCapBody()
        : MaterialBody(MaterialType_PolylineCap),
          interiorColor(-1),
          outlineColor(-1),
          outlineWidth(0),
          startType(-1),
          endType(-1),
          shapeHeight(0),
          shapeWidth(0),
          radius(0){}

    unsigned int interiorColor;      //Interior fill color, in b, g, r, a order.
    unsigned int outlineColor;       //Outline color, in b, g, r, a order.
    unsigned char outlineWidth;     //Outline width, as %a of the road width.
    unsigned char startType;         //0: Arrow, 1: circle, 2: line join
    unsigned char endType;           //0: Arrow, 1: circle, 2: line join
    unsigned short shapeHeight;      //Arrow height in %a of width of road 
    unsigned short shapeWidth;       //Arrow width in %a of width of road
    unsigned short radius;           //Circle in %a of width of road
    
    unsigned int Size()
    {
        return BaseSize() + sizeof(unsigned int)*2 + sizeof(unsigned char)*3 + sizeof(unsigned short)*3;
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);
};

//! type 18 bitmap pattern for line material
struct MaterialBmpPatternForLineBody: public MaterialBody 
{
    MaterialBmpPatternForLineBody()
        : MaterialBody(MaterialType_BmpPatternForLine),
          color(-1),
          texture(NBM_INVALIDATE_INDEX),
          distance(0),
          width(0),
          height(0){}

    unsigned int color;      //color in b, g, r, a order
    NBMIndex texture;        //Texture index into “IMGS?or “BMAP?chunk.msb: = 0 then IMGS chunk.msb: = 1 then BMAP chunk.

    unsigned short distance; //Transparent block length in pixel
    unsigned short width;    //Texture width in pixel (consist with road width)
    unsigned short height;   //Texture height in pixel

    unsigned int Size()
    {
        return BaseSize() + sizeof(unsigned int) + sizeof(NBMIndex) + sizeof(unsigned short)*3;
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);
};

//! type 16 BiOutlined material
//struct MaterialBiOutlinedBody : public MaterialBody
//{
//    MaterialBiOutlinedBody()
//        : MaterialBody(MaterialType_BiOutlined),
//        interiorColor(-1),
//        outlineColor(-1),
//        outlineWidth(0),
//        hInteriorColor(-1),
//        hOutlineColor(-1),
//        hInteriorWidth(0),
//        hOutlineWidth(0)
//    {
//    }
//    Uint32_t interiorColor;
//    Uint32_t outlineColor;
//    Uint8_t  outlineWidth;
//    Uint32_t hInteriorColor; // highlight interior color
//    Uint8_t  hInteriorWidth; // highlight interior width
//    Uint32_t hOutlineColor;  // highlight outline color
//    Uint8_t  hOutlineWidth;  // highlight outline width
//
//    unsigned int Size()
//    {
//        return BaseSize() + sizeof(interiorColor) + sizeof(outlineColor) + sizeof(outlineWidth) + sizeof(hInteriorColor) + sizeof(hInteriorWidth) + sizeof(hOutlineColor) + sizeof(hOutlineWidth);
//    }
//    char* ToBuf();
//    int FromBuf(unsigned char* buf);
//};
//! type 255 Back Compatible Material
//! not ready yet
struct MaterialBackCompatibleBody : public MaterialBody
{
    MaterialBackCompatibleBody() 
        : MaterialBody(MaterialType_BackCompatible)        
    {
    }

    unsigned int Size()
    {
        return BaseSize();
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);
};

//! type 11
struct MaterialPinBody : public MaterialBody
{
    MaterialPinBody() 
        : MaterialBody(MaterialType_Pin),
		iconIndex(NBM_INVALIDATE_INDEX),
        cxoffset(0),
        cyoffset(0),
		bxoffset(0),
		byoffset(0)
	{
	}

    NBMIndex  iconIndex;      // 0 :then IMGS chunk. 1 :then BMAP chunk.
    signed char  cxoffset;           //Callout x center offset.
	signed char  cyoffset;           //Callout y center offset.
	signed char  bxoffset;           //Bubble x center offset.
	signed char  byoffset;           // Bubble y center offset.
    unsigned int Size()
    {
        return BaseSize() + sizeof(NBMIndex) +  sizeof(signed char)*4;
    }
    char* ToBuf();
    int FromBuf(unsigned char* buf);
};

//! type 12 area with outline
struct MaterialAreaOutlinedBody : public MaterialBody
{
	MaterialAreaOutlinedBody(unsigned char bSimple = 0)
        : MaterialBody(bSimple?(bSimple==1?MaterialType_AreaOutlinedColor_Simple:MaterialType_AreaOutlinedColor_Holey):MaterialType_AreaOutlinedColor),
        interiorColor(-1),
        outlineColor(-1),
        outlineWidth(0)
	{
	}

	unsigned int interiorColor;
	unsigned int outlineColor;
	unsigned char  outlineWidth;

	unsigned int Size()
	{
		return BaseSize() + sizeof(interiorColor) 
			+ sizeof(outlineColor) + sizeof(outlineWidth);
	}
	char* ToBuf();
	int FromBuf(unsigned char* buf);
};

//! type 19 area with dash outline
struct MaterialAreaDashOutlinedBody : public MaterialBody
{
	MaterialAreaDashOutlinedBody(bool bSimple = false)
        : MaterialBody(MaterialType_AreaDashOutlined),
        simple(bSimple?1:0),
        bitOnColor(-1),
        bitOffColor(-1),
        outlinePattern(0),
        interiorColor(-1),
        outlineWidth(0),
        innerBitOnColor(-1),
        innerBitOffColor(-1),
        innerOutlinePattern(0),
        innerOutlineWidth(0)
	{
	}

    unsigned char simple;         // 1 : Simple, 0 : Complex, 2: Inner
    unsigned int bitOnColor;      // Outline Bit On Color, in b, g, r, a order.
    unsigned int bitOffColor;     // Outline Bit On Color, in b, g, r, a order.
    unsigned int outlinePattern;  // Outline pattern, specified as a 32-bit bitmask.
    unsigned int interiorColor;   // Line interior color, in b, g, r, a order
    unsigned char outlineWidth;   // Outline width, as a % of the object width
    unsigned int innerBitOnColor;      // Inner Outline Bit On Color, in b, g, r, a order.
    unsigned int innerBitOffColor;     // Inner Outline Bit On Color, in b, g, r, a order.
    unsigned int innerOutlinePattern;  // Inner Outline pattern, specified as a 32-bit bitmask.
    unsigned char innerOutlineWidth;   // Inner Outline width, as a % of the object width


	unsigned int Size()
	{
		return BaseSize() + sizeof(unsigned char) + sizeof(unsigned int) * 7 + sizeof(unsigned char) * 2;
	}
	char* ToBuf();
	int FromBuf(unsigned char* buf);
};

class NBMMaterialChunk : public NBMChunk
{
public:
	NBMMaterialChunk(NBMFileHeader * pFileHeader,bool bMake);
	~NBMMaterialChunk(void);

	void setMaterialOffset(unsigned short offset) {m_wFlag = offset;}
	unsigned short getMaterialOffset() {return m_wFlag;}
		
	//! type 0
	NBMIndex SetData(TextureBitMapBody& tbb, unsigned int color = -1);
	bool GetData(NBMIndex uIndex, TextureBitMapBody& tbb);
	bool GetData(NBMIndex uIndex, TextureBitMapBody& tbb, unsigned int& color);

	NBMIndex SetData(MaterialTextureBody& mtb);
	bool GetData(NBMIndex uIndex, MaterialTextureBody& mtb);

	//! type 1	
	NBMIndex SetData(unsigned int);
	bool GetData(NBMIndex uIndex, unsigned int&);

	//! type 2
	NBMIndex SetData(MaterialOutlinedBody&);
	bool GetData(NBMIndex uIndex, MaterialOutlinedBody&);

	//! type 3
	NBMIndex SetData(MaterialLightedBody&);
	bool GetData(NBMIndex uIndex, MaterialLightedBody&);

	//! type 4
	NBMIndex SetData(MaterialFontBody&);
	bool GetData(NBMIndex uIndex, MaterialFontBody&);

	//! type 5
	NBMIndex SetData(MaterialPatternLineBody&);
	bool GetData(NBMIndex uIndex, MaterialPatternLineBody&);

	//! type 6
	NBMIndex SetData(MaterialBackgroundBody&);
	bool GetData(NBMIndex uIndex, MaterialBackgroundBody&);

    //! type 7	
    NBMIndex SetData(TextureBitMapBody& tbb,MaterialShieldBody& mtb);
    bool GetData(NBMIndex uIndex, MaterialShieldBody& mtb,TextureBitMapBody& tbb);
	
	NBMIndex SetData(NBMIndex idx, MaterialShieldBody& mtb);
	bool GetData(NBMIndex uIndex, MaterialShieldBody& mtb, NBMIndex& idx);
	

    //! type 14
    NBMIndex SetData(TextureBitMapBody& tbb,MaterialShieldYBody& mtyb);
    bool GetData(NBMIndex uIndex, MaterialShieldYBody& mtyb,TextureBitMapBody& tbb);

	NBMIndex SetData(NBMIndex idx, MaterialShieldYBody& mtyb);
	bool GetData(NBMIndex uIndex, MaterialShieldYBody& mtyb, NBMIndex& idx);

    //! type 15
    NBMIndex SetData(TextureBitMapBody& tbb1,TextureBitMapBody& tbb2,MaterialNewPinBody& body);
    bool GetData(NBMIndex uIndex, MaterialNewPinBody& body,TextureBitMapBody& tbb1,TextureBitMapBody& tbb2);

	//! type 16
	NBMIndex SetData(TextureBitMapBody& tbb1, TextureBitMapBody& tbb2, TextureBitMapBody& tbb3, MaterialPOIBody& body);
	bool GetData(NBMIndex uIndex, TextureBitMapBody& tbb1, TextureBitMapBody& tbb2, TextureBitMapBody& tbb3, MaterialPOIBody& body);

	NBMIndex SetData(NBMIndex idx1, NBMIndex idx2, NBMIndex idx3, MaterialPOIBody& body);
	bool GetData(NBMIndex uIndex, NBMIndex& idx1, NBMIndex& idx2, NBMIndex& idx3, MaterialPOIBody& body);

    //! type 17
    NBMIndex SetData(MaterialPolylineCapBody& mpb);
    bool GetData(NBMIndex uIndex, MaterialPolylineCapBody& mpb);

    //! type 18
    NBMIndex SetData(TextureBitMapBody& tbb,MaterialBmpPatternForLineBody& mbp);
    bool GetData(NBMIndex uIndex, MaterialBmpPatternForLineBody& mbp,TextureBitMapBody& tbb);
    
    //! old type 16 deprecated
    /*NBMIndex SetData(MaterialBiOutlinedBody& mdb);
    bool GetData(NBMIndex uIndex, MaterialBiOutlinedBody& mdb);*/

    //! type 8	
    NBMIndex SetData(MaterialDashOutlineBody& mtb);
    bool GetData(NBMIndex uIndex, MaterialDashOutlineBody& mtb);

    //! type 9	
    NBMIndex SetData(MaterialPathArrowBody& mtb);
    bool GetData(NBMIndex uIndex, MaterialPathArrowBody& mtb);  

    //! type 10
    NBMIndex SetData(MaterialFourSidesGradientBody& body);
    bool GetData(NBMIndex uIndex, MaterialFourSidesGradientBody& body);

    //! type 255
    NBMIndex SetData(MaterialBackCompatibleBody& body);
    bool GetData(NBMIndex uIndex, MaterialBackCompatibleBody& body);

	bool GetDataType(NBMIndex, int& type);
	//!type 11
	NBMIndex SetData(TextureBitMapBody& tbb,MaterialPinBody& body);
	bool GetData(NBMIndex uIndex,MaterialPinBody& body,TextureBitMapBody& tbb);

	//! type 12
	NBMIndex SetData(MaterialAreaOutlinedBody& mob);
	bool GetData(NBMIndex uIndex, MaterialAreaOutlinedBody& mob);

	//! type 19
	NBMIndex SetData(MaterialAreaDashOutlinedBody& mob);
	bool GetData(NBMIndex uIndex, MaterialAreaDashOutlinedBody& mob);

protected:
	unsigned int GetOneItemLen(unsigned char* pBuf);
};
