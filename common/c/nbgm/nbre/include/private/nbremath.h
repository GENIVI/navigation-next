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

@file nbremath.h
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
#ifndef _NBRE_MATH_H_
#define _NBRE_MATH_H_
#include "paltypes.h"
#include "palstdlib.h"

/*! Static class with some useful math constants and functions */
class NBRE_Math
{
public:
    /// Checks whether the float value is zero
    static nb_boolean IsZero(float v, float epsilon);

    /// Checks whether the float value is not a number (NAN)
    template<class T> static nb_boolean IsNaN(T v);
    /// return NaN of type
    template<class T> static T NaN();

    /// Checks whether the number is a power of 2
    static nb_boolean IsPowerOfTwo(uint32 number);

    static int32 PowerOfTwo(int32 number);

    /// Checks the two value are equal
    static nb_boolean AreEqual(float f1, float f2, float epsilon);
    static nb_boolean AreEqual(double d1, double d2, double epsilon);

    static float RadToDeg(float radians);
    static double RadToDeg(double radians);

    static float DegToRad(float degree);
    static double DegToRad(double degree);

    static float DirectionToHeading(float x, float y);


    template<class T> static T Min(const T& a, const T& b);
    template<class T> static T Max(const T& a, const T& b);
    template<class T> static T Abs(const T& a);
    template<class T> static T Sign(const T& a);
    template<class T> static T Clamp(const T& value, const T& low, const T& high);
    template<class T1, class T2> static void Swap(T1& a, T2& b);    

public:
    static const float  Pi;          /**< Pi      3.14159265359f */
    static const float  PiTwo;       /**< Pi*2    6.28318530718f */
    static const float  PiHalf;      /**< Pi/2    1.57079632679f */
    static const float  Deg2Rad;     /**< Pi/180  0.01745329252f */
    static const float  Rad2Deg;     /**< 180/Pi  57.29577951308f*/
    static const double Pi64;        /**< Pi      3.14159265359  */
    static const double PiTwo64;     /**< Pi*2    6.28318530718  */
    static const double PiHalf64;    /**< Pi/2    1.57079632679  */
    static const double Deg2Rad64;   /**< Pi/180  0.01745329252  */
    static const double Rad2Deg64;   /**< 180/Pi  57.29577951308 */
    static const float  Infinity;    /**< INFINITY 0x7F800000 */
};


#include "nbremath.inl"


#endif
