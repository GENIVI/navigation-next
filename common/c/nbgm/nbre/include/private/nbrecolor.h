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

    @file nbrecolor.h
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
#ifndef _NBRE_COLOR_H_
#define _NBRE_COLOR_H_
#include "nbretypes.h"

typedef uint32 RGBA;
typedef uint32 ARGB;
typedef uint32 ABGR;
typedef uint32 BGRA;

class NBRE_Color
{

public:
    explicit NBRE_Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f)
    { }

    NBRE_Color( float red,
                float green,
                float blue,
                float alpha ) : r(red), g(green), b(blue), a(alpha)
    { }

    NBRE_Color(const NBRE_Color& color)
    {
        r = color.r;
        g = color.g;
        b = color.b;
        a = color.a;
    }

    NBRE_Color& operator = (const NBRE_Color& color)
    {
        if(this != &color)
        {
            r = color.r;
            g = color.g;
            b = color.b;
            a = color.a;
        }
        return *this;
    }

    NBRE_Color operator * (const NBRE_Color& color) const
    {
        return NBRE_Color(r * color.r, g * color.g, b * color.b, a * color.a);
    }

    bool operator == (const NBRE_Color& c) const
    {
        return r == c.r && g == c.g && b == c.b && a == c.a;
    }

    bool operator != (const NBRE_Color& c) const
    {
        return r != c.r || g != c.g || b != c.b || a != c.a;
    }

public:
        /** Retrieves color as RGBA.
        */
        RGBA GetAsRGBA(void) const
        {
            uint8 val8;
            uint32 val32 = 0;

            // Red
            val8 = static_cast<uint8>(r * 255);
            val32 = (val8 << 24);

            // Green
            val8 = static_cast<uint8>(g * 255);
            val32 += (val8 << 16);

            // Blue
            val8 = static_cast<uint8>(b * 255);
            val32 += (val8<<8);

            // Alpha
            val8 = static_cast<uint8>(a * 255);
            val32 += val8;
            return val32;
        }

        /** Retrieves color as ARGB.
        */
        ARGB GetAsARGB(void) const
        {
            uint8 val8;
            uint32 val32 = 0;

            // Alpha
            val8 = static_cast<uint8>(a * 255);
            val32 = val8 << 24;

            // Red
            val8 = static_cast<uint8>(r * 255);
            val32 += val8 << 16;

            // Green
            val8 = static_cast<uint8>(g * 255);
            val32 += val8 << 8;

            // Blue
            val8 = static_cast<uint8>(b * 255);
            val32 += val8;

            return val32;
        }

        /** Retrieves color as BGRA.
        */
        BGRA GetAsBGRA(void) const;

        /** Retrieves color as ABGR */
        ABGR GetAsABGR(void) const
        {
            uint8 val8;
            uint32 val32 = 0;
            
            // Red
            val8 = static_cast<uint8>(a * 255);
            val32 = (val8 << 24);
            
            // Green
            val8 = static_cast<uint8>(b * 255);
            val32 += (val8 << 16);
            
            // Blue
            val8 = static_cast<uint8>(g * 255);
            val32 += (val8<<8);
            
            // Alpha
            val8 = static_cast<uint8>(r * 255);
            val32 += val8;
            return val32;
            }

        /** Sets color as RGBA.
        */
        void SetAsRGBA(const RGBA val)
        {
            uint32 val32 = val;

            // Convert from 32bit pattern
            // (RGBA = 8888)

            // Red
            r = ((val32 >> 24) & 0xFF) / 255.0f;

            // Green
            g = ((val32 >> 16) & 0xFF) / 255.0f;

            // Blue
            b = ((val32 >> 8) & 0xFF) / 255.0f;

            // Alpha
            a = (val32 & 0xFF) / 255.0f;
        }

        /** Sets color as ARGB.
        */
        void SetAsARGB(const ARGB val);

        /** Sets color as BGRA.
        */
        void SetAsBGRA(const BGRA val)
        {
            uint32 val32 = val;

            // Convert from 32bit pattern
            // (ARGB = 8888)

            // Blue
            b = ((val32 >> 24) & 0xFF) / 255.0f;

            // Green
            g = ((val32 >> 16) & 0xFF) / 255.0f;

            // Red
            r = ((val32 >> 8) & 0xFF) / 255.0f;

            // Alpha
            a = (val32 & 0xFF) / 255.0f;
        }

        /** Sets color as ABGR.
        */
        void SetAsABGR(const ABGR val);

public:
    float r;
    float g;
    float b;
    float a;
};

#endif
