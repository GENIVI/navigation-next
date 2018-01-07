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
#include "nbgmrasterize.h"
#include "nbreintersection.h"
#include "nbremath.h"

void
NBGM_Rasterize::DrawLine(int32 x1, int32 y1, int32 x2, int32 y2, NBGM_ISetPixelCallback* pixelCallback)
{
    int32 d, x, y, ax, ay, sx, sy, dx, dy;

    dx = x2-x1;  ax = NBRE_Math::Abs<int32>(dx)<<1;  sx = NBRE_Math::Sign<int32>(dx);
    dy = y2-y1;  ay = NBRE_Math::Abs<int32>(dy)<<1;  sy = NBRE_Math::Sign<int32>(dy);

    x = x1;
    y = y1;
    // x dominant
    if (ax > ay) 
    {
        d = ay - (ax >> 1);
        nb_boolean continueLoop = TRUE;
        while (continueLoop) 
        {
            if (!pixelCallback->OnSetPixel(x, y, TRUE))
            {
                return;
            }
            
            if (x == x2) 
            {
                return;
            }
            if (d >= 0) 
            {
                y += sy;
                d -= ax;
            }
            x += sx;
            d += ay;
        }
    }
    // y dominant
    else 
    {
        d = ax - (ay >> 1);
        nb_boolean continueLoop = TRUE;
        while (continueLoop) 
        {
            if (!pixelCallback->OnSetPixel(x, y, FALSE))
            {
                return;
            }

            if (y == y2) 
            {
                return;
            }

            if (d >= 0) 
            {
                x += sx;
                d -= ay;
            }
            y += sy;
            d += ax;
        }
    }
}

void
NBGM_Rasterize::DrawPolyline(const NBGM_Point2iList& polyline, NBGM_ISetPixelCallback* pixelCallback)
{
    for (uint32 i = 0; i < polyline.size() - 1; ++i)
    {
        int32 x1 = polyline[i].x;
        int32 y1 = polyline[i].y;
        int32 x2 = polyline[i + 1].x;
        int32 y2 = polyline[i + 1].y;

        int32 d, x, y, ax, ay, sx, sy, dx, dy;

        dx = x2-x1;  ax = NBRE_Math::Abs<int32>(dx)<<1;  sx = NBRE_Math::Sign<int32>(dx);
        dy = y2-y1;  ay = NBRE_Math::Abs<int32>(dy)<<1;  sy = NBRE_Math::Sign<int32>(dy);

        x = x1;
        y = y1;
        
        // x dominant
        if (ax > ay) 
        {
            d = ay - (ax >> 1);
            
            nb_boolean continueLoop = TRUE;
            while (continueLoop) 
            {
                // Don't draw segment end points twice
                if (i == 0 || x != x1 || y != y1)
                {
                    if (!pixelCallback->OnSetPixel(x, y, TRUE))
                    {
                        return;
                    }
                }

                if (x == x2) 
                {
                    break;
                }
                if (d >= 0) 
                {
                    y += sy;
                    d -= ax;
                }
                x += sx;
                d += ay;
            }
        }
        // y dominant
        else 
        {
            d = ax - (ay >> 1);
            nb_boolean continueLoop = TRUE;
            while (continueLoop) 
            {
                // Don't draw segment end points twice
                if (i == 0 || x != x1 || y != y1)
                {
                    if (!pixelCallback->OnSetPixel(x, y, FALSE))
                    {
                        return;
                    }
                }
                
                if (y == y2) 
                {
                    break;
                }

                if (d >= 0) 
                {
                    x += sx;
                    d -= ay;
                }
                y += sy;
                d += ax;
            }
        }
    }
}

void
NBGM_Rasterize::DrawLine(int32 x1, int32 y1, int32 x2, int32 y2, int32 width, int32 height, NBRE_Vector<NBRE_Vector2i>& result)
{
    int32 d, x, y, ax, ay, sx, sy, dx, dy;

    dx = x2-x1;  ax = NBRE_Math::Abs<int32>(dx)<<1;  sx = NBRE_Math::Sign<int32>(dx);
    dy = y2-y1;  ay = NBRE_Math::Abs<int32>(dy)<<1;  sy = NBRE_Math::Sign<int32>(dy);

    x = x1;
    y = y1;
    // x dominant
    if (ax > ay) 
    {
        d = ay - (ax >> 1);
        nb_boolean continueLoop = TRUE;
        while (continueLoop) 
        {
            if (x >= 0 && x < width && y >= 0 && y < height)
            {
                result.push_back(NBRE_Vector2i(x, y));
            }

            if (x == x2) 
            {
                return;
            }
            if (d >= 0) 
            {
                y += sy;
                d -= ax;
            }
            x += sx;
            d += ay;
        }
    }
    // y dominant
    else 
    {
        d = ax - (ay >> 1);
        nb_boolean continueLoop = TRUE;
        while (continueLoop) 
        {
            if (x >= 0 && x < width && y >= 0 && y < height)
            {
                result.push_back(NBRE_Vector2i(x, y));
            }

            if (y == y2) 
            {
                return;
            }

            if (d >= 0) 
            {
                x += sx;
                d -= ay;
            }
            y += sy;
            d += ax;
        }
    }
}
