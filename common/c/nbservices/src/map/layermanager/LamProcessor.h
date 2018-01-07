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
   @file        LamProcessor.h
   @defgroup    nbmap

   Description: LamProcessor is responsible for decode LAM tiles and check if
   tile is available by tile key and lam-index.
   This class can be embedded into LayerAvailabilityMatrixLayer, but it's
   separated in order to perform UnitTest on it.
*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

#ifndef _LAMPROCESSOR_H_
#define _LAMPROCESSOR_H_


extern "C"
{
#include "palclock.h"
}

#include "NBPng.h"
#include "TileInternal.h"
#include <set>

namespace nbmap
{
class LamProcessor : public Base
{
public:
    LamProcessor(NB_Context* context);
    virtual ~LamProcessor();

    /*! Decode LAM from tile.

      @return decoded PNG_Data or NULL.
    */
    PNG_Data* DecodeLamFromTile(TilePtr tile);

    /*! Decode LAM from file.

      @return decoded PNG_Data or NULL.
    */
    PNG_Data* DecodeLamFromFile(const string& path);

    /*! Check if index of LAM tile is true for point (x, y)

      @return status bit of index at point(x,y)
    */
    bool CheckLamIndex(PNG_Data* decodedData, int x, int y, int lamIndex);

    void DestroyPNGData(PNG_Data** decodedData);

private:
    /*! Decode PNG file from data stream.

      @return PNG_Data*
    */
    PNG_Data* DecodePNGFromDataStream(nbcommon::DataStreamPtr dataStream);

    NB_Context* m_pContext;
};

}

#endif /* _LAMPROCESSOR_H_ */

/*! @} */
