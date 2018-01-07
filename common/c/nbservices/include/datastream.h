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

    @file       datastream.h

    Class 'DataStream' is an interface. It is used to append or get data
    like a stream.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef DATASTREAM_H
#define DATASTREAM_H

/*!
    @addtogroup nbcommon
    @{
*/

extern "C"
{
#include "paltypes.h"
#include "nberror.h"
}

#include "nbexp.h"
#include "smartpointer.h"

struct PAL_File;

namespace nbcommon
{

// Types ........................................................................................

class DataStream;
typedef shared_ptr<DataStream> DataStreamPtr;

/*! Data stream interface */
class NB_DLL DataStream
{
public:
    // Interface Functions ......................................................................

    /*! Append data

        This function copies the buffer and saves in this 'DataStream' object.

        @return NE_OK if success
     */
    virtual NB_Error
    AppendData(const uint8* buffer,     /*!< Buffer to append. This function does not take
                                             ownership of this buffer. */
               uint32 bufferSize        /*!< Number of bytes to append */
               ) = 0;

    /*! Get size of data from a position

        @return number of bytes got
    */
    virtual uint32
    GetData(uint8* buffer,              /*!< Buffer to get data into. The size of this buffer
                                             must be not less than parameter 'requestedSize'. */
            uint32 position,            /*!< Position started to get data */
            uint32 requestedSize        /*!< Requested size of data */
            ) const = 0;

    /*! Get total data size

        @return Total data size
    */
    virtual uint32
    GetDataSize() const = 0;

    /*! Dump content into file.

      @return NE_OK when succeeded.
    */
    virtual NB_Error
    DumpToPALFile(PAL_File* file) const = 0;

public:
    // Public functions .........................................................................

    /*! DataStream destructor */
    virtual ~DataStream();

    /*! Create a data stream object

        @return A shared pointer to created object
    */
    static DataStreamPtr Create();
};

};  // namespace nbmap

/*! @} */

#endif  // DATASTREAM_H
