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

    @file       palfreetypeengine.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "palfreetypeengine.h"

unsigned UNICODE_RANGE[] =
{ 0x0000, 0x007F
, 0x0080, 0x00FF
, 0x0100, 0x017F
, 0x0180, 0x024F
, 0x0250, 0x02AF
, 0x02B0, 0x02FF
, 0x0300, 0x036F
, 0x0370, 0x03FF
, 0x0400, 0x04FF
, 0x0500, 0x052F
, 0x0530, 0x058F
, 0x0590, 0x05FF
, 0x0600, 0x06FF
, 0x0700, 0x074F
, 0x0750, 0x077F
, 0x0780, 0x07BF
, 0x07C0, 0x07FF
, 0x0800, 0x083F
, 0x0840, 0x085F
, 0x08A0, 0x08FF
, 0x0900, 0x097F
, 0x0980, 0x09FF
, 0x0A00, 0x0A7F
, 0x0A80, 0x0AFF
, 0x0B00, 0x0B7F
, 0x0B80, 0x0BFF
, 0x0C00, 0x0C7F
, 0x0C80, 0x0CFF
, 0x0D00, 0x0D7F
, 0x0D80, 0x0DFF
, 0x0E00, 0x0E7F
, 0x0E80, 0x0EFF
, 0x0F00, 0x0FFF
, 0x1000, 0x109F
, 0x10A0, 0x10FF
, 0x1100, 0x11FF
, 0x1200, 0x137F
, 0x1380, 0x139F
, 0x13A0, 0x13FF
, 0x1400, 0x167F
, 0x1680, 0x169F
, 0x16A0, 0x16FF
, 0x1700, 0x171F
, 0x1720, 0x173F
, 0x1740, 0x175F
, 0x1760, 0x177F
, 0x1780, 0x17FF
, 0x1800, 0x18AF
, 0x18B0, 0x18FF
, 0x1900, 0x194F
, 0x1950, 0x197F
, 0x1980, 0x19DF
, 0x19E0, 0x19FF
, 0x1A00, 0x1A1F
, 0x1A20, 0x1AAF
, 0x1B00, 0x1B7F
, 0x1B80, 0x1BBF
, 0x1BC0, 0x1BFF
, 0x1C00, 0x1C4F
, 0x1C50, 0x1C7F
, 0x1CC0, 0x1CCF
, 0x1CD0, 0x1CFF
, 0x1D00, 0x1D7F
, 0x1D80, 0x1DBF
, 0x1DC0, 0x1DFF
, 0x1E00, 0x1EFF
, 0x1F00, 0x1FFF
, 0x2000, 0x206F
, 0x2070, 0x209F
, 0x20A0, 0x20CF
, 0x20D0, 0x20FF
, 0x2100, 0x214F
, 0x2150, 0x218F
, 0x2190, 0x21FF
, 0x2200, 0x22FF
, 0x2300, 0x23FF
, 0x2400, 0x243F
, 0x2440, 0x245F
, 0x2460, 0x24FF
, 0x2500, 0x257F
, 0x2580, 0x259F
, 0x25A0, 0x25FF
, 0x2600, 0x26FF
, 0x2700, 0x27BF
, 0x27C0, 0x27EF
, 0x27F0, 0x27FF
, 0x2800, 0x28FF
, 0x2900, 0x297F
, 0x2980, 0x29FF
, 0x2A00, 0x2AFF
, 0x2B00, 0x2BFF
, 0x2C00, 0x2C5F
, 0x2C60, 0x2C7F
, 0x2C80, 0x2CFF
, 0x2D00, 0x2D2F
, 0x2D30, 0x2D7F
, 0x2D80, 0x2DDF
, 0x2DE0, 0x2DFF
, 0x2E00, 0x2E7F
, 0x2E80, 0x2EFF
, 0x2F00, 0x2FDF
, 0x2FF0, 0x2FFF
, 0x3000, 0x303F
, 0x3040, 0x309F
, 0x30A0, 0x30FF
, 0x3100, 0x312F
, 0x3130, 0x318F
, 0x3190, 0x319F
, 0x31A0, 0x31BF
, 0x31C0, 0x31EF
, 0x31F0, 0x31FF
, 0x3200, 0x32FF
, 0x3300, 0x33FF
, 0x3400, 0x4DBF
, 0x4DC0, 0x4DFF
, 0x4E00, 0x9FFF
, 0xA000, 0xA48F
, 0xA490, 0xA4CF
, 0xA4D0, 0xA4FF
, 0xA500, 0xA63F
, 0xA640, 0xA69F
, 0xA6A0, 0xA6FF
, 0xA700, 0xA71F
, 0xA720, 0xA7FF
, 0xA800, 0xA82F
, 0xA830, 0xA83F
, 0xA840, 0xA87F
, 0xA880, 0xA8DF
, 0xA8E0, 0xA8FF
, 0xA900, 0xA92F
, 0xA930, 0xA95F
, 0xA960, 0xA97F
, 0xA980, 0xA9DF
, 0xAA00, 0xAA5F
, 0xAA60, 0xAA7F
, 0xAA80, 0xAADF
, 0xAAE0, 0xAAFF
, 0xAB00, 0xAB2F
, 0xABC0, 0xABFF
, 0xAC00, 0xD7AF
, 0xD7B0, 0xD7FF
, 0xD800, 0xDB7F
, 0xDB80, 0xDBFF
, 0xDC00, 0xDFFF
, 0xE000, 0xF8FF
, 0xF900, 0xFAFF
, 0xFB00, 0xFB4F
, 0xFB50, 0xFDFF
, 0xFE00, 0xFE0F
, 0xFE10, 0xFE1F
, 0xFE20, 0xFE2F
, 0xFE30, 0xFE4F
, 0xFE50, 0xFE6F
, 0xFE70, 0xFEFF
, 0xFF00, 0xFFEF
, 0xFFF0, 0xFFFF
, 0x10000, 0x1007F
, 0x10080, 0x100FF
, 0x10100, 0x1013F
, 0x10140, 0x1018F
, 0x10190, 0x101CF
, 0x101D0, 0x101FF
, 0x10280, 0x1029F
, 0x102A0, 0x102DF
, 0x10300, 0x1032F
, 0x10330, 0x1034F
, 0x10380, 0x1039F
, 0x103A0, 0x103DF
, 0x10400, 0x1044F
, 0x10450, 0x1047F
, 0x10480, 0x104AF
, 0x10800, 0x1083F
, 0x10840, 0x1085F
, 0x10900, 0x1091F
, 0x10920, 0x1093F
, 0x10980, 0x1099F
, 0x109A0, 0x109FF
, 0x10A00, 0x10A5F
, 0x10A60, 0x10A7F
, 0x10B00, 0x10B3F
, 0x10B40, 0x10B5F
, 0x10B60, 0x10B7F
, 0x10C00, 0x10C4F
, 0x10E60, 0x10E7F
, 0x11000, 0x1107F
, 0x11080, 0x110CF
, 0x110D0, 0x110FF
, 0x11100, 0x1114F
, 0x11180, 0x111DF
, 0x11680, 0x116CF
, 0x12000, 0x123FF
, 0x12400, 0x1247F
, 0x13000, 0x1342F
, 0x16800, 0x16A3F
, 0x16F00, 0x16F9F
, 0x1B000, 0x1B0FF
, 0x1D000, 0x1D0FF
, 0x1D100, 0x1D1FF
, 0x1D200, 0x1D24F
, 0x1D300, 0x1D35F
, 0x1D360, 0x1D37F
, 0x1D400, 0x1D7FF
, 0x1EE00, 0x1EEFF
, 0x1F000, 0x1F02F
, 0x1F030, 0x1F09F
, 0x1F0A0, 0x1F0FF
, 0x1F100, 0x1F1FF
, 0x1F200, 0x1F2FF
, 0x1F300, 0x1F5FF
, 0x1F600, 0x1F64F
, 0x1F680, 0x1F6FF
, 0x1F700, 0x1F77F
, 0x20000, 0x2A6DF
, 0x2A700, 0x2B73F
, 0x2B740, 0x2B81F
, 0x2F800, 0x2FA1F
, 0xE0000, 0xE007F
, 0xE0100, 0xE01EF
, 0xF0000, 0xFFFFF
, 0x100000, 0x10FFFF
};
const uint32 UNICODE_BLOCK_COUNT = 227 - 8 + 1;

unsigned UNICODE_SEARCH_RANGE[UNICODE_BLOCK_COUNT] = {0};

static int strlen_utf8_c(const char *s)
{
    int i = 0;
    int j = 0;
    while (s[i])
    {
        if ((s[i] & 0xc0) != 0x80)
        {
            j++;
        }
        i++;
    }
    return j;
}

static int utf8_to_unicode(const char* utf8Str, wchar** out, uint32* outsize)
{
    uint8* p = (uint8*)utf8Str;
    wchar* result = NULL;

    uint32 len = strlen_utf8_c(utf8Str);

    result = new wchar[len + 1];
    nsl_memset(result, 0, sizeof(wchar) * (len + 1));
    wchar* pDst = result;

    while(*p)
    {
        uint8 ch = *p;
        if ((ch & 0xFE) == 0xFC)//1111110x
        {
            *pDst++ = ' ';
            p += 6;
        }
        else if ((ch & 0xFC) == 0xF8)//111110xx
        {
            *pDst++ = ' ';
            p += 5;
        }
        else if ((ch & 0xF8) == 0xF0)//11110xxx
        {
            *pDst++ = ' ';
            p += 4;
        }
        else if ((ch & 0xF0) == 0xE0)//1110xxxx
        {
            *pDst++ =
                ( (((p[0] << 4) & 0xF0) | ((p[1] >> 2) & 0x0F)) << 8)
                | (((p[1] << 6) & 0xC0) | ((p[2]) & 0x3F));
            p += 3;
        }
        else if ((ch & 0xE0) == 0xC0)//110xxxxx
        {
            *pDst++ =
                (((p[0] >> 2) & 0x07) << 8)
                |( ((p[0] << 6) & 0xC0) | (p[1] & 0x3F));
            p += 2;
        }
        else//0xxxxxxx
        {
            *pDst++ = *p++;
        }
    }

    *out = result;
    *outsize = len;
    return 0;
}

FreetypeEngine::FreetypeEngine(PAL_Instance* pal)
:mLibrary(NULL)
{
    FT_Init_FreeType(&mLibrary);
    //@TODO: use a config file instead of hardcoded fonts
    mFontTable.push_back(new Font("/system/fonts/Roboto-Regular.ttf"));
    mFontTable.push_back(new Font("/system/fonts/Roboto-Bold.ttf"));
    mFontTable.push_back(new Font("/system/fonts/DroidSerif-Regular.ttf"));
    mFontTable.push_back(new Font("/system/fonts/DroidSerif-Bold.ttf"));
    mFontTable.push_back(new Font("/system/fonts/Roboto-Italic.ttf"));
    mFontTable.push_back(new Font("/system/fonts/Roboto-BoldItalic.ttf"));
    mFontTable.push_back(new Font("/system/fonts/DroidSerif-Italic.ttf"));
    mFontTable.push_back(new Font("/system/fonts/DroidSerif-BoldItalic.ttf"));

    mFontTable.push_back(new Font("/system/fonts/DroidSansFallback.ttf"));
    mFontTable.push_back(new Font("/system/fonts/DroidSansArabic.ttf"));
    mFontTable.push_back(new Font("/system/fonts/DroidSansThai.ttf"));

    // Default
    for (uint32 j = 0; j < 8; ++j)
    {
        for (uint32 i = 0; i < UNICODE_BLOCK_COUNT; ++i)
        {
            mGlyphFontMap[j][UNICODE_RANGE[i  * 2 + 1]] = 8;
        }
    }

    // Latin
    for (uint32 i = 0; i < 8; ++i)
    {
        mGlyphFontMap[i][0x0000007f] = i;
        mGlyphFontMap[i][0x000000ff] = i;
        mGlyphFontMap[i][0x0000017f] = i;
        mGlyphFontMap[i][0x000003ff] = i;
        mGlyphFontMap[i][0x000004ff] = i;
    }    

    // Arabic
    for (uint32 i = 0; i < 8; ++i)
    {
        mGlyphFontMap[i][0x000006FF] = 9;
        mGlyphFontMap[i][0x0000077F] = 9;
        mGlyphFontMap[i][0x000008FF] = 9;
        mGlyphFontMap[i][0x0000FDFF] = 9;
        mGlyphFontMap[i][0x0000FEFF] = 9;
        mGlyphFontMap[i][0x00010E7F] = 9;
        mGlyphFontMap[i][0x0001EEFF] = 9;
    }

    // Thai
    for (uint32 i = 0; i < 8; ++i)
    {
        mGlyphFontMap[i][0x00000E7F] = 10;
    }


    // Initialize Unicode block search order.
    // When search the block of a character, we iterate UNICODE_SEARCH_RANGE[].
    // Most recently accessed block is always at the front of UNICODE_SEARCH_RANGE.
    for (uint32 i = 0; i < UNICODE_BLOCK_COUNT; ++i)
    {
        UNICODE_SEARCH_RANGE[i] = i;
    }
}

FreetypeEngine::~FreetypeEngine()
{
    for (uint32 i = 0; i < mFontTable.size(); ++i)
    {
        delete mFontTable[i];
    }
    FT_Done_FreeType(mLibrary);
}

void AlignLine(uint32 lineBeginIndex, uint32 lineWidth, int32 actualWidth, PAL_TEXT_ALIGN align, std::vector<PAL_GlyphInfo>& gs)
{
    int32 lineOffset = 0;

    switch (align)
    {
    case PAL_TA_LEFT:
        lineOffset = 0;
        break;
    case PAL_TA_RIGHT:
        lineOffset = lineWidth - actualWidth;
        break;
    case PAL_TA_CENTER:
    default:
        lineOffset = ((int32)lineWidth - actualWidth) / 2;
        break;
    }    

    // Align to line center
    for (uint32 i = lineBeginIndex; i < gs.size(); ++i)
    {
        gs[i].boundingBox.xMin += lineOffset;
        gs[i].boundingBox.xMax += lineOffset;
    }
}

PAL_Error FreetypeEngine::GetComplexTextLayout(uint32 pixelFontHeight, const char* style, const char* utf8String, uint32 lineWidth, PAL_TEXT_ALIGN align, PAL_GlyphInfo** glyphInfos, uint32* glyphCount, PAL_GlyphBoundingBox* boundingBox)
{
    *glyphInfos = NULL;
    *glyphCount = 0;

    wchar* ustring = NULL;
    uint32 ulen = 0;
    utf8_to_unicode(utf8String, &ustring, &ulen);

    if (ustring == NULL || ulen == 0)
    {
        return PAL_ErrBadParam;
    }

    uint32 styleId = 0;
    if (nsl_strcmp(style, FONT_NAME_SANS) == 0)
    {
        styleId = 0;
    }
    else if (nsl_strcmp(style, FONT_NAME_SANS_BOLD) == 0)
    {
        styleId = 1;
    }
    else if (nsl_strcmp(style, FONT_NAME_SERIF) == 0)
    {
        styleId = 2;
    }
    else if (nsl_strcmp(style, FONT_NAME_SERIF_BOLD) == 0)
    {
        styleId = 3;
    }
    else if (nsl_strcmp(style, FONT_NAME_SANS_ITALIC) == 0)
    {
        styleId = 4;
    }
    else if (nsl_strcmp(style, FONT_NAME_SANS_BOLD_ITALIC) == 0)
    {
        styleId = 5;
    }
    else if (nsl_strcmp(style, FONT_NAME_SERIF_ITALIC) == 0)
    {
        styleId = 6;
    }
    else if (nsl_strcmp(style, FONT_NAME_SERIF_BOLD_ITALIC) == 0)
    {
        styleId = 7;
    }

    std::vector<uint32> glyphFontIds;
    std::set<uint32> fontIds;
    for (uint32 i = 0; i < ulen; ++i)
    {
        wchar ch = ustring[i];
        uint32 fontId = GetFontId(styleId, ch);
        glyphFontIds.push_back(fontId);
        fontIds.insert(fontId);
    }

    int32 ascent = 0;
    int32 descent = 0;
    for (std::set<uint32>::iterator it = fontIds.begin(); it != fontIds.end(); ++it)
    {
        Font& font = *mFontTable[*it];
        font.Load(mLibrary, pixelFontHeight);
        int32 asc = font.GetAscent();
        if (asc > ascent)
        {
            ascent = asc;
        }
        int32 desc = font.GetDescent();
        if (desc > descent)
        {
            descent = desc;
        }
    }

    std::vector<PAL_GlyphInfo> gs;
    int32 dx = 0;
    int32 dy = 0;
    int32 lineHeight = 0;
    uint32 lineBeginIndex = 0;
    uint32 breakIndex = (uint32)-1;
    std::vector<uint32> breakableIndices;
    std::vector<uint32> breakableIndexWidths;
    nb_boolean hasRetrun = FALSE;
    
    // find break index
    for (uint32 i = 0; i < ulen; ++i)
    {
        if (ustring[i] == '\r')
        {
            continue;
        }
        if (ustring[i] == '\n')
        {
            hasRetrun = TRUE;
            break;
        }
        if (ustring[i] == ' ' || ustring[i] == '\t')
        {
            breakableIndices.push_back(i);
            breakableIndexWidths.push_back(dx);
        }

        uint32 fontId = glyphFontIds[i];
        Font& font = *mFontTable[fontId];
        dx += font.GetGlyphAdvance(ustring[i]);
    }

    if (dx > (int32)lineWidth && !hasRetrun)
    {
        uint32 minDeltaWidth = dx + 1;
        for (uint32 i = 0; i < breakableIndices.size(); ++i)
        {
            int32 d = breakableIndexWidths[i];
            d = d - (dx - d);
            if (d < 0)
            {
                d = -d;
            }
            if ((uint32)d < minDeltaWidth)
            {
                breakIndex = breakableIndices[i];
                minDeltaWidth = d;
            }
        }
    }

    dx = 0;
    for (uint32 i = 0; i < ulen; ++i)
    {
        if (ustring[i] == '\r')
        {
            continue;
        }
        if (ustring[i] == '\n' || i == breakIndex)
        {
            AlignLine(lineBeginIndex, lineWidth, dx, align, gs);
            lineBeginIndex = gs.size();

            // Go to next line
            dy += lineHeight + pixelFontHeight / 4;
            dx = 0;
            lineHeight = 0;
            continue;
        }

        uint32 fontId = glyphFontIds[i];
        Font& font = *mFontTable[fontId];
        if (!(ustring[i] == ' '
            ||ustring[i] == '\t'
            ||ustring[i] == '\r'
            ||ustring[i] == '\n'
            ))
        {
            FreetypeEngine::Glyph* glyph = font.LoadGlyph(ustring[i]);
            if (glyph != NULL)
            {
                PAL_GlyphInfo g;
                g.fontId = fontId;
                g.glyphId = ustring[i];
                g.boundingBox.xMin = dx + glyph->xOffset;
                g.boundingBox.yMin = dy + ascent - font.GetAscent() + glyph->yOffset;
                g.boundingBox.xMax = g.boundingBox.xMin + glyph->width - 1;
                g.boundingBox.yMax = g.boundingBox.yMin + glyph->height - 1;
                gs.push_back(g);

                int32 lh = g.boundingBox.yMax - g.boundingBox.yMin + 1;
                if (lh > lineHeight)
                {
                    lineHeight = lh;
                }
            }
        }

        dx += font.GetGlyphAdvance(ustring[i]);
    }
    AlignLine(lineBeginIndex, lineWidth, dx, align, gs);

    if (gs.size() == 0)
    {
        delete[] ustring;
        return PAL_ErrNotFound;
    }

    for (uint32 i = 0; i < gs.size(); ++i)
    {
        PAL_GlyphInfo& g = gs[i];
        if (i == 0)
        {
            *boundingBox = g.boundingBox;
        }
        else
        {
            if (g.boundingBox.xMin < boundingBox->xMin)
            {
                boundingBox->xMin = g.boundingBox.xMin;
            }
            if (g.boundingBox.yMin < boundingBox->yMin)
            {
                boundingBox->yMin = g.boundingBox.yMin;
            }
            if (g.boundingBox.xMax > boundingBox->xMax)
            {
                boundingBox->xMax = g.boundingBox.xMax;
            }
            if (g.boundingBox.yMax > boundingBox->yMax)
            {
                boundingBox->yMax = g.boundingBox.yMax;
            }
        }
    }

    *glyphCount = gs.size();
    *glyphInfos = new PAL_GlyphInfo[gs.size()];
    nsl_memcpy(*glyphInfos, &gs[0], gs.size() * sizeof(PAL_GlyphInfo));
    //{
    //    wchar buf[256];
    //    wsprintf(buf, L"GetComplexTextLayout: str=%s\n", ustring);
    //    OutputDebugStringW(buf);
    //}

    delete[] ustring;
    return PAL_Ok;
}

PAL_Error FreetypeEngine::GetGlyphBitmap(uint32 pixelFontHeight, uint32 fontID, uint32 glyphID, uint8** data, uint32* width, uint32* height, int32* xOffset, int32* yOffset)
{
    *data = NULL;
    *width = 0;
    *height = 0;
    *xOffset = 0;
    *yOffset = 0;

    if (fontID >= mFontTable.size())
    {
        return PAL_ErrBadParam;
    }

    if (!mFontTable[fontID]->Load(mLibrary, pixelFontHeight))
    {
        return PAL_ErrNotFound;
    }
    return mFontTable[fontID]->GetGlyphBitmap(pixelFontHeight, glyphID, data, width, height, xOffset, yOffset);
}

PAL_Error FreetypeEngine::Font::GetGlyphBitmap(uint32 pixelFontHeight, uint32 glyphID, uint8** data, uint32* width, uint32* height, int32* xOffset, int32* yOffset)
{
    *width = 0;
    *height = 0;
    *data = 0;
    *xOffset = 0;
    *yOffset = 0;

    if (pixelFontHeight < 1)
    {
        return PAL_ErrBadParam;
    }

    FT_Face fFace = face;
    if (FT_Load_Char(fFace, glyphID, FT_LOAD_RENDER) != 0)
    {
        return PAL_ErrNotFound;
    }
    FT_Pixel_Mode mode = (FT_Pixel_Mode)fFace->glyph->bitmap.pixel_mode;
    if (!(mode == FT_PIXEL_MODE_GRAY || mode == FT_PIXEL_MODE_MONO))
    {
        return PAL_ErrNotFound;
    }

    uint8* src = (uint8*)fFace->glyph->bitmap.buffer;
    uint32 w = fFace->glyph->bitmap.width;
    uint32 h = fFace->glyph->bitmap.rows;
    uint32 pitch = fFace->glyph->bitmap.pitch >= 0 ? fFace->glyph->bitmap.pitch : -fFace->glyph->bitmap.pitch;

    *data = new uint8[w * h];
    switch(mode)
    {
    case FT_PIXEL_MODE_GRAY:
        nsl_memcpy(*data, src, w * h);
        break;
    case FT_PIXEL_MODE_MONO:
        {
            uint8* dst = *data;
            uint8* pSrc = src;
            for (uint32 y = 0; y < h; ++y)
            {
                for (uint32 x = 0; x < w; ++x)
                {
                    *dst++ = pSrc[x / 8] & (1 << (7 - (x % 8))) ? 0xFF : 0;
                }
                pSrc += pitch;
            }
        }
        break;
    default:
        break;
    }

    *width = w;
    *height = h;
    return PAL_Ok;
}

inline uint32 FreetypeEngine::GetFontId(uint32 style, uint32 glyphID)
{
    for (uint32 i = 0; i < UNICODE_BLOCK_COUNT; ++i)
    {
        uint32 rangeIndex = UNICODE_SEARCH_RANGE[i];
        if (glyphID >= UNICODE_RANGE[rangeIndex * 2]
        && glyphID <= UNICODE_RANGE[rangeIndex * 2 + 1])
        {
            if (i > 0)
            {
                // Move current search range to front
                for (uint32 j = i; j > 0; --j)
                {
                    UNICODE_SEARCH_RANGE[j] = UNICODE_SEARCH_RANGE[j - 1];
                }
                UNICODE_SEARCH_RANGE[0] = rangeIndex;
            }
            return mGlyphFontMap[style][UNICODE_RANGE[rangeIndex * 2 + 1]];
        }
    }
    // not found
    return (uint32)-1;
}

int32 FreetypeEngine::Font::GetAscent()
{
    return ascender;
}

int32 FreetypeEngine::Font::GetDescent()
{
    return descender;
}

int32 FreetypeEngine::Font::GetGlyphAdvance(wchar ch)
{
    Glyph* g = LoadGlyph(ch);
    if (g)
    {
        return g->advance;
    }
    else
    {
        return 0;
    }
}

FreetypeEngine::Font::Font(const std::string& path)
:path(path)
,face(NULL)
,ascender(0)
,descender(0)
{
}

FreetypeEngine::Font::~Font()
{
    for (GlyphMap::iterator it = glyphCache.begin(); it != glyphCache.end(); ++it)
    {
        for (GlyphSizeMap::iterator si = it->second.begin(); si != it->second.end(); ++si)
        {
            delete si->second;
        }
    }
    if (face)
    {
        FT_Done_Face(face);
    }
}

bool FreetypeEngine::Font::Load(FT_Library ftLibrary, uint32 pixelFontHeight)
{
    if (face == NULL)
    {
        FT_Error error = FT_New_Face(ftLibrary, path.c_str(), 0, &face);
        if (error != 0)
        {
            return false;
        }
    }
    FT_Set_Pixel_Sizes(face, 0, pixelFontHeight);
    this->pixelFontHeight = pixelFontHeight;

    ascender = (int32) (face->ascender * face->size->metrics.y_ppem / face->units_per_EM);
    descender = (int32) (face->descender * face->size->metrics.y_ppem / face->units_per_EM);

    return true;
}

FreetypeEngine::Glyph* FreetypeEngine::Font::LoadGlyph(uint32 glyphID)
{
    GlyphMap::iterator it = glyphCache.find((wchar)glyphID);
    if (it != glyphCache.end())
    {
        GlyphSizeMap::iterator si = it->second.find(pixelFontHeight);
        if (si != it->second.end())
        {
            return si->second;
        }
    }

    FT_Face fFace = face;
    if ( FT_Get_Char_Index(face, glyphID) == 0 )
    {
        // missing glyph
        return NULL;
    }

    FT_Set_Pixel_Sizes(face, 0, pixelFontHeight);

    if (FT_Load_Char(fFace, glyphID, FT_LOAD_RENDER) != 0)
    {
        return NULL;
    }
    FT_Pixel_Mode mode = (FT_Pixel_Mode)fFace->glyph->bitmap.pixel_mode;
    if (!(mode == FT_PIXEL_MODE_GRAY || mode == FT_PIXEL_MODE_MONO))
    {
        return NULL;
    }

    uint32 w = fFace->glyph->bitmap.width;
    uint32 h = fFace->glyph->bitmap.rows;
    //uint8* src = (uint8*)fFace->glyph->bitmap.buffer;
    //uint32 pitch = fFace->glyph->bitmap.pitch >= 0 ? fFace->glyph->bitmap.pitch : -fFace->glyph->bitmap.pitch;

    FreetypeEngine::Glyph* result = new FreetypeEngine::Glyph;
    result->width = w;
    result->height = h;
    result->xOffset = fFace->glyph->bitmap_left;
    result->yOffset = (fFace->ascender * fFace->size->metrics.y_ppem / fFace->units_per_EM) - fFace->glyph->bitmap_top;
    result->advance = (int32)(face->glyph->metrics.horiAdvance >> 6);
    result->data = NULL;

    //result->data = new uint8[w * h];
    //switch(mode)
    //{
    //case FT_PIXEL_MODE_GRAY:
    //    nsl_memcpy(result->data, src, w * h);
    //    break;
    //case FT_PIXEL_MODE_MONO:
    //    {
    //        uint8* dst = result->data;
    //        uint8* pSrc = src;
    //        for (uint32 y = 0; y < h; ++y)
    //        {
    //            for (uint32 x = 0; x < w; ++x)
    //            {
    //                *dst++ = pSrc[x / 8] & (1 << (7 - (x % 8))) ? 0xFF : 0;
    //            }
    //            pSrc += pitch;
    //        }
    //    }
    //    break;
    //}

    if (it != glyphCache.end())
    {
        it->second[pixelFontHeight] = result;
    }
    else
    {
        GlyphSizeMap sizeList;
        sizeList[pixelFontHeight] = result;
        glyphCache[(wchar)glyphID] = sizeList;
    }
    return result;
}
