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

    @file nbregpuprogram.h
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
#ifndef _NBRE_GPUPROGRAM_H_
#define _NBRE_GPUPROGRAM_H_

#include "nbretypes.h"
#include "nbrevector3.h"

class NBRE_GpuProgramParam
{
public:
    enum ValueType
    {
        VT_None,
        VT_Float,
        VT_Float2,
        VT_Float3,
        VT_Float4,
        VT_Int,
        VT_Matrix4,
        VT_Predefined,
        VT_Float3Array
    };

    enum PredefinedVar
    {
        PV_ModelViewProjectionMatrix,
        PV_NormalMatrix,
        PV_ModelMatrix,
        PV_ViewMatrix,
        PV_ProjectionMatrix,
        PV_ViewInverseTransposeMatrix
    };

    union Value
    {
        float f1;
        float f2[2];
        float f3[3];
        float f4[4];
        float mat4[16];
        int i1;
        PredefinedVar pre;
    };

public:
    NBRE_GpuProgramParam():mType(VT_None),mCount(0) {}
    NBRE_GpuProgramParam(float x, float y, float z, float w):mType(VT_Float4),mCount(0) { mValue.f4[0] = x; mValue.f4[1] = y; mValue.f4[2] = z; mValue.f4[3] = w; }
    NBRE_GpuProgramParam(float x, float y, float z):mType(VT_Float3),mCount(0) { mValue.f3[0] = x; mValue.f3[1] = y; mValue.f3[2] = z; }
    NBRE_GpuProgramParam(float x, float y):mType(VT_Float2),mCount(0) { mValue.f2[0] = x; mValue.f2[1] = y; }
    NBRE_GpuProgramParam(float v):mType(VT_Float),mCount(0) { mValue.f1 = v; }
    NBRE_GpuProgramParam(int v):mType(VT_Int),mCount(0) { mValue.i1 = v; }
    NBRE_GpuProgramParam(const float* mat):mType(VT_Matrix4),mCount(0) { memcpy(mValue.mat4, mat, sizeof(float) * 16); }
    NBRE_GpuProgramParam(PredefinedVar var):mType(VT_Predefined),mCount(0) { mValue.pre = var; }
    NBRE_GpuProgramParam(shared_ptr<NBRE_Vector<float> > arr, int count):mType(VT_Float3Array),mCount(count),mArray(arr) { }
    ~NBRE_GpuProgramParam() {}
    ValueType GetType() const { return mType; }
    const Value& GetValue() const { return mValue; }
    int GetCount() const { return mCount; }
    shared_ptr<NBRE_Vector<float> > GetArray() const { return mArray; }

private:
    ValueType mType;
    int mCount;
    Value mValue;
    shared_ptr<NBRE_Vector<float> > mArray;
};


class NBRE_GpuProgram
{
public:
    virtual ~NBRE_GpuProgram() {}
};
#endif
