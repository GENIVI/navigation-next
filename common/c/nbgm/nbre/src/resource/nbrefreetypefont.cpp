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
#include "nbrefreetypefont.h"
#include "ft2build.h"
#include FT_FREETYPE_H

//#define ENABLE_FONT_DEBUG

static float
ImageBlurFactor(int32 i, float radius)
{
    if (i == 0)
    {
        return 1.0f;
    }
    else
    {
        float low = i - 0.5f;
        if (low > radius + 0.5f)
        {
            return 0;
        }
        else
        {
            float high = MIN(i + 0.5f, radius + 0.5f);
            return ((1.0f - (low - 0.5f) / radius) + (1.0f - (high - 0.5f) / radius)) * 0.5f * (high - low);
        }
    }
}

static void
ImageBlur(uint8* image, int32 width, int32 height, float radius)
{
    int32 i;
    int32 j;
    int32 r;
    float total;
    float component;
    float factor;
    uint8* result = NBRE_NEW uint8[width * height];

    // Do x axis blur on image first, save result to result
    for (i = 0; i < width; ++i)
    {
        for (j = 0; j < height; ++j)
        {
            factor = ImageBlurFactor(0, radius);
            total = (float)image[i + j * width] * factor;
            component = factor;

            for (r = 1; r <= (int32)ceilf(radius); ++r)
            {
                factor = ImageBlurFactor(r, radius);
                if (i - r >= 0)
                {
                    total += image[i - r + j * width] * factor;
                    component += factor;
                }

                if (i + r < width)
                {
                    total += image[i + r + j * width] * factor;
                    component += factor;
                }
            }

            result[i + j * width] = (uint8)(total / component);
        }
    }

    // Do y axis blur on result, save result to image
    for (i = 0; i < width; ++i)
    {
        for (j = 0; j < height; ++j)
        {
            factor = ImageBlurFactor(0, radius);
            total = result[i + j * width] * factor;
            component = factor;

            for (r = 1; r <= (int32)ceilf(radius); ++r)
            {
                factor = ImageBlurFactor(r, radius);
                if (j - r >= 0)
                {
                    total += result[i + (j - r) * width] * factor;
                    component += factor;
                }

                if (j + r < height)
                {
                    total += result[i + (j + r) * width] * factor;
                    component += factor;
                }
            }

            image[i + j * width] = (uint8)(total / component);
        }
    }

    NBRE_DELETE_ARRAY result;
}

static void
ImageColorScale(uint8* image, int32 width, int32 height, float factor)
{
    int32 i = 0;
    for (i = 0; i < width * height; ++i)
    {
        float v = image[i] * factor;
        if (v >= 255.0f)
        {
            image[i] = 0xFF;
        }
        else
        {
            image[i] = (uint8)v;
        }
    }
}

static void
Blit(NBRE_Image* srcImg, int32 srcX, int32 srcY, int32 width, int32 height, NBRE_Image* dstImg, int32 dstX, int32 dstY)
{
    nbre_assert(srcX >= 0 && srcY >= 0 && srcX + width - 1 < static_cast<int>(srcImg->Width()) && srcY + height - 1 < static_cast<int>(srcImg->Height()));

    int32 srcW = static_cast<int>(srcImg->Width());
    int32 dstW = static_cast<int>(dstImg->Width());
    int32 dstH = static_cast<int>(dstImg->Height());
    int32 dstLeft = MAX(dstX, 0);
    int32 dstTop = MAX(dstY, 0);
    int32 dstRight = MIN(dstW - 1, dstX + width - 1);
    int32 dstBottom = MIN(dstH - 1, dstY + height - 1);
    uint8* srcData = srcImg->GetImageData();
    uint8* dstData = dstImg->GetImageData();

    for (int32 y = dstTop; y <= dstBottom; ++y)
    {
        for (int32 x = dstLeft; x <= dstRight; ++x)
        {
            dstData[y * dstW + x] = srcData[(y - dstY + srcY) * srcW + (x - dstX + srcX)];
        }
    }
}

static NBRE_Image*
MakeForeground(FT_GlyphSlot slot, int32 gridHeight, int32 offset, NBRE_AxisAlignedBox2i& foreRect)
{
    // outline pixels
    uint32 w = slot->bitmap.width;
    uint32 h = gridHeight;
    NBRE_Image* img = NBRE_NEW NBRE_Image(w, h, NBRE_PF_A8);
    uint8* data = img->GetImageData();

    int32 yMax = MIN(gridHeight - offset, slot->bitmap.rows);
    // Copy bitmap from glyph slot
    for (int32 y = 0; y < yMax; ++y)
    {
        for (int32 x = 0; x < slot->bitmap.width; ++x)
        {
            data[w * (y + offset) + x] = slot->bitmap.buffer[slot->bitmap.pitch * (slot->bitmap.rows - 1 - y) + x];
        }
    }

    // Calculate texcoord for foreground
    foreRect.minExtend.x = 0;
    foreRect.minExtend.y = 0;
    foreRect.maxExtend.x = w - 1;
    foreRect.maxExtend.y = h - 1;
    return img;
}

static NBRE_Image*
MakeOutline(NBRE_Image* img, float outlineWidth, int32 ow, NBRE_AxisAlignedBox2i& backRect)
{
    // Outline pixels
    uint32 w = img->Width();
    uint32 h = img->Height();

    // Make outline bitmap: blur & enhance
    NBRE_Image* outlineImage = NBRE_NEW NBRE_Image(w + ow * 2, h, NBRE_PF_A8);
    Blit(img, 0, 0, w, img->Height(), outlineImage, ow, 0);
    ImageBlur(outlineImage->GetImageData(), outlineImage->Width(), outlineImage->Height(), outlineWidth);
    ImageColorScale(outlineImage->GetImageData(), outlineImage->Width(), outlineImage->Height(), 20.0f);

    // Calculate texcoord for background
    backRect.minExtend.x = img->Width();
    backRect.minExtend.y = 0;
    backRect.maxExtend.x = img->Width() + outlineImage->Width() - 1;
    backRect.maxExtend.y = h - 1;
    return outlineImage;
}

static NBRE_Image*
MakeGlyphBitmap(int32 fontHeight, float outlinePercentage, FT_Face face, FT_GlyphSlot slot, NBRE_AxisAlignedBox2i& foreRect, NBRE_AxisAlignedBox2i& backRect, uint32 glyphWidth, uint32 glyphHeight)
{
    float outlineWidth = fontHeight * outlinePercentage;
    int32 ow = static_cast<int32>(ceilf(outlineWidth));
    int32 fh = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
    int32 gridHeight = fh + ow * 2;
    int32 offset = ow + (fh - ((face->size->metrics.ascender >> 6) - slot->bitmap_top + slot->bitmap.rows - 1));

    // Generate foreground
    NBRE_Image* fore = MakeForeground(slot, gridHeight, offset, foreRect);
    // Generate background
    NBRE_Image* outlineImage = MakeOutline(fore, outlineWidth, ow, backRect);
    // Make sure font bitmap not exceed glyph size
    nbre_assert(fore->Width() + outlineImage->Width() <= glyphWidth
        && outlineImage->Height() <= glyphHeight);

    // Merge foreground & background to glyph image
    uint32 w = glyphWidth;
    uint32 h = glyphHeight;
    NBRE_Image* result = NBRE_NEW NBRE_Image(w, h, NBRE_PF_A8);

    // Make padding between images as much as possible.
    // To reduce the texture sampling error.
    uint32 xPadding = (glyphWidth - fore->Width() - outlineImage->Width()) / 3;
    uint32 yPadding = (glyphHeight - outlineImage->Height()) / 2;
    // Foreground
    Blit(fore, 0, 0, fore->Width(), fore->Height(), result, xPadding, yPadding);
    foreRect.minExtend.x = xPadding;
    foreRect.minExtend.y = yPadding;
    foreRect.maxExtend.x = xPadding + fore->Width() - 1;
    foreRect.maxExtend.y = yPadding + fore->Height() - 1;
    nbre_assert(foreRect.maxExtend.x < (int32)result->Width() && foreRect.maxExtend.y < (int32)result->Height());
    // Background
    Blit(outlineImage, 0, 0, outlineImage->Width(), outlineImage->Height(), result, foreRect.maxExtend.x + 1 + xPadding, yPadding);
    backRect.minExtend.x = foreRect.maxExtend.x + 1 + xPadding;
    backRect.minExtend.y = yPadding;
    backRect.maxExtend.x = foreRect.maxExtend.x + xPadding + outlineImage->Width();
    backRect.maxExtend.y = yPadding + outlineImage->Height() - 1;
    nbre_assert(backRect.maxExtend.x < (int32)result->Width() && backRect.maxExtend.y < (int32)result->Height());

    // Clean up
    NBRE_DELETE fore;
    NBRE_DELETE outlineImage;
    return result;
}

#ifdef ENABLE_FONT_DEBUG

static void
ImageDrawHorizontalLine(NBRE_Image* image, int32 y, int32 x0, int32 x1, uint8 value)
{
    int32 w = static_cast<int32>(image->Width());
    int32 h = static_cast<int32>(image->Height());
    if (y >= 0 && y < h)
    {
        int32 left = MIN(x0, x1);
        int32 right = MAX(x0, x1);
        left = MAX(0, left);
        right = MIN(w - 1, right);
        if (left <= right)
        {
            uint8* data = image->GetImageData();
            for (int32 x = left; x <= right; ++x)
            {
                data[x + y * w] = value;
            }
        }
    }
}

static void
ImageDrawVerticalLine(NBRE_Image* image, int32 x, int32 y0, int32 y1, uint8 value)
{
    int32 w = static_cast<int32>(image->Width());
    int32 h = static_cast<int32>(image->Height());
    if (x >= 0 && x < w)
    {
        int32 bottom = MIN(y0, y1);
        int32 top = MAX(y0, y1);
        bottom = MAX(0, bottom);
        top = MIN(h - 1, top);
        if (bottom <= top)
        {
            uint8* data = image->GetImageData();
            for (int32 y = bottom; y <= top; ++y)
            {
                data[x + y * w] = value;
            }
        }
    }
}

static void
ImageDrawRect(NBRE_Image* image, const NBRE_AxisAlignedBox2i& rect, uint8 value)
{
    ImageDrawHorizontalLine(image, rect.minExtend.y, rect.minExtend.x, rect.maxExtend.x, value);
    ImageDrawHorizontalLine(image, rect.maxExtend.y, rect.minExtend.x, rect.maxExtend.x, value);
    ImageDrawVerticalLine(image, rect.minExtend.x, rect.minExtend.y, rect.maxExtend.y, value);
    ImageDrawVerticalLine(image, rect.maxExtend.x, rect.minExtend.y, rect.maxExtend.y, value);
}

#endif


static NBRE_Image*
CreateSubGlyph(FT_Face face, wchar character, int32 fontHeight, float outlinePercentage
               , NBRE_AxisAlignedBox2i& fore, NBRE_AxisAlignedBox2i& back, uint32 glyphWidth, uint32 glyphHeight)
{
    FT_GlyphSlot slot = face->glyph;
    NBRE_Image* glyphImage = NULL;

    if (FT_Get_Char_Index(face, character) == 0)
    {
        character = (FT_ULong)' ';
    }

    if (FT_Set_Pixel_Sizes( face, 0, fontHeight ) == 0)
    {
        if (FT_Load_Char( face, character, FT_LOAD_RENDER ) == 0)
        {
            if(character == (FT_ULong)' ')
            {
                float outlineWidth = fontHeight * outlinePercentage;
                int32 ow = static_cast<int32>(ceilf(outlineWidth));
                int32 fh = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
                int32 gridHeight = fh + ow * 2;

                uint32 w = MAX(1, (slot->advance.x + 63) / 64);
                uint32 h = gridHeight;

                glyphImage = NBRE_NEW NBRE_Image(w, h, NBRE_PF_A8);

                fore.minExtend.x = 0;
                fore.maxExtend.x = w - 1;
                back.minExtend.x = 0;
                back.maxExtend.x = w - 1;
                fore.minExtend.y = 0;
                fore.maxExtend.y = h - 1;
                back.minExtend.y = 0;
                back.maxExtend.y = h - 1;
            }
            else
            {
                glyphImage = MakeGlyphBitmap(fontHeight, outlinePercentage, face, slot, fore, back, glyphWidth, glyphHeight);
            }

#ifdef ENABLE_FONT_DEBUG
            ImageDrawRect(glyphImage, fore, 0x88);
            ImageDrawRect(glyphImage, back, 0x44);
#endif

        }
    }

    return glyphImage;
}

static NBRE_GlyphPtr
CreateGlyphMipmaps(FT_Face face, wchar character, int32 fontHeight, float outlinePercentage, const NBRE_Vector2i& glyphSize)
{
    nbre_assert(fontHeight >= 4);

    NBRE_GlyphPtr result;
    NBRE_ImageList bitmaps;
    NBRE_AxisAlignedBox2i foreRect;
    NBRE_AxisAlignedBox2i backRect;

    uint32 glyphWidth = static_cast<uint32>(glyphSize.x);
    uint32 glyphHeight = static_cast<uint32>(glyphSize.y);
    for (int32 f = fontHeight; f >= fontHeight; f /= 2)
    {
        NBRE_AxisAlignedBox2i fore;
        NBRE_AxisAlignedBox2i back;
        NBRE_Image* subImg = NULL;

        if (f < 4)
        {
            subImg = bitmaps.back()->GetResize(glyphWidth, glyphHeight);
        }
        else
        {
            subImg = CreateSubGlyph(face, character, f, outlinePercentage, fore, back, glyphWidth, glyphHeight);
        }

        nbre_assert(subImg != NULL);
        if (f == fontHeight)
        {
            foreRect = fore;
            backRect = back;
        }

        bitmaps.push_back(subImg);
        glyphWidth /= 2;
        glyphHeight /= 2;
        if (glyphWidth == 0)
        {
            glyphWidth = 1;
        }
        if (glyphHeight == 0)
        {
            glyphHeight = 1;
        }
    }

    return NBRE_GlyphPtr(NBRE_NEW NBRE_Glyph(character, foreRect, backRect, bitmaps, fontHeight));
}

struct FreeTypeContext
{
    FT_Library library;
    FT_Face face;
};

NBRE_FreeTypeFont::NBRE_FreeTypeFont(NBRE_Context& context, const NBRE_String& fontPath):
    NBRE_Font(context, NBRE_Vector2i(1024, 1024), NBRE_Vector2i(64, 40)),
    mFontPath(fontPath)
{
    FT_Library library;
    FT_Face face = NULL;
    FT_Error err;
    err = FT_Init_FreeType(&library);
    nbre_assert(err == 0);
    err = err?err:FT_New_Face(library, mFontPath.c_str(), 0, &face);
    nbre_assert(err == 0);
    mFtContext = NBRE_NEW FreeTypeContext;
    mFtContext->face = face;
    mFtContext->library = library;
}

NBRE_FreeTypeFont::NBRE_FreeTypeFont(NBRE_Context& context, uint8* fontBuffer, uint32 fontBufferSize):
    NBRE_Font(context, NBRE_Vector2i(1024, 1024), NBRE_Vector2i(64, 40))
{
    FT_Library library;
    FT_Face face = NULL;
    FT_Error err;
    err = FT_Init_FreeType(&library);
    nbre_assert(err == 0);
    err = err?err:FT_New_Memory_Face(library, fontBuffer, fontBufferSize, 0, &face);
    nbre_assert(err == 0);
    mFtContext = NBRE_NEW FreeTypeContext;
    mFtContext->face = face;
    mFtContext->library = library;
}

NBRE_FreeTypeFont::~NBRE_FreeTypeFont()
{
    FT_Done_Face(mFtContext->face);
    FT_Done_FreeType(mFtContext->library);
    NBRE_DELETE mFtContext;
}

NBRE_GlyphPtr
NBRE_FreeTypeFont::CreateGlyph(wchar character)
{
    return CreateGlyphMipmaps(mFtContext->face, character, 26, 0.1f, NBRE_Vector2i(64, 40));
}

const NBRE_Vector2i&
NBRE_FreeTypeFont::GetGlyphSlotSize()
{
    return mSlotSize;
}
