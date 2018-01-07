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
#include "nbremath.h"

const float NBRE_Math::Pi = 3.14159265359f;
const float NBRE_Math::PiTwo = 6.28318530718f;
const float NBRE_Math::PiHalf = 1.57079632679f;
const float NBRE_Math::Deg2Rad = 0.017453292520f;
const float NBRE_Math::Rad2Deg = 57.29577951308f;

const double NBRE_Math::Pi64 = 3.14159265358979323846;
const double NBRE_Math::PiTwo64 = 6.28318530717958623200;
const double NBRE_Math::PiHalf64 = 1.57079632679489655800;
const double NBRE_Math::Deg2Rad64 = 0.01745329251994329576;
const double NBRE_Math::Rad2Deg64 = 57.2957795130823208768;

static float GetInfinity()
{
    union 
    {
        uint32 i;
        float f;
    };
    i = 0x7F800000;
    return f;
}

const float NBRE_Math::Infinity = GetInfinity();

float NBRE_Math::DirectionToHeading(float x, float y)
{

    if(x == 0 && y == 0)
    {
        return 0;
    }

    float absX = (float)nsl_fabs(x);
    float absY = (float)nsl_fabs(y);

    float tanVal;

    if(absX > absY)
    {
        tanVal = absY/absX; 
    }
    else
    {
        tanVal = absX/absY; 
    }

    float theta = static_cast<float>(nsl_atan(tanVal));

    if(absX < absY)
    {
        theta = PiHalf - theta;
    }
    if(x < 0)
    {
        theta = Pi - theta;
    }
    if(y < 0)
    {
        theta = PiTwo - theta;
    }
    return RadToDeg(theta);
}
