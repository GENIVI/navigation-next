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

    @file       palfreetypeengine.h

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
#ifndef PAL_ITYPE_ENGINE_H
#define	PAL_ITYPE_ENGINE_H

#include "palstdlib.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <pallock.h>
#include "paltestlog.h"
#include "palfont.h"
#include <map>
#include <vector>
#include <set>
#include <string>

#define MAX_FONT_PATH_LENGTH 256

#define FONT_NAME_SANS "00"
#define FONT_NAME_SANS_ITALIC "001"
#define FONT_NAME_SANS_BOLD "01"
#define FONT_NAME_SANS_BOLD_ITALIC "011"
#define FONT_NAME_SERIF "10"
#define FONT_NAME_SERIF_ITALIC "101"
#define FONT_NAME_SERIF_BOLD "11"
#define FONT_NAME_SERIF_BOLD_ITALIC "111"

typedef wchar_t wchar;

class FreetypeEngine
{
    struct Glyph
    {
        uint32 width;
        uint32 height;
        uint8* data;
        int32 xOffset;
        int32 yOffset;
        int32 advance;
    };

    class Font
    {
    public:
        Font(const std::string& path);
        ~Font();
        PAL_Error GetGlyphBitmap(uint32 pixelFontHeight, uint32 glyphID, uint8** data, uint32* width, uint32* height, int32* xOffset, int32* yOffset);
        bool Load(FT_Library ftLibrary, uint32 pixelFontHeight);
        int32 GetAscent();
        int32 GetDescent();
        int32 GetGlyphAdvance(wchar ch);
        const std::string& GetPath() { return path; }
        Glyph* LoadGlyph(uint32 glyphID);
    private:
        std::string path;
        FT_Face face;
        typedef std::map<uint32, Glyph*> GlyphSizeMap;
        typedef std::map<wchar, GlyphSizeMap> GlyphMap;
        GlyphMap glyphCache;        
        uint32 pixelFontHeight;
        int32 ascender;
        int32 descender;
    };

public:
    FreetypeEngine(PAL_Instance* pal);
    ~FreetypeEngine();
    PAL_Error GetComplexTextLayout(uint32 pixelFontHeight, const char* style, const char* utf8String, uint32 lineWidth, PAL_TEXT_ALIGN align, PAL_GlyphInfo** glyphInfos, uint32* glyphCount, PAL_GlyphBoundingBox* boundingBox);
    PAL_Error GetGlyphBitmap(uint32 pixelFontHeight, uint32 fontID, uint32 glyphID, uint8** data, uint32* width, uint32* height, int32* xOffset, int32* yOffset);
    uint32 GetFontId(uint32 style, uint32 glyphID);

private:
    typedef std::map<uint32, uint32> GlyphFontMap;
    typedef std::vector<Font*> FontList;
    FT_Library mLibrary;
    FontList mFontTable;
    GlyphFontMap mGlyphFontMap[8];
};

#endif
