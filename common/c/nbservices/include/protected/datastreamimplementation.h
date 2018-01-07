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

    @file       datastreamimplementation.h

    Class 'DataStreamImplementation' inherits from 'DataStream'
    interface. A 'DataStreamImplementation' object is used to append
    or get data like a stream.
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

#ifndef DATASTREAMIMPLEMENTATION_H
#define DATASTREAMIMPLEMENTATION_H

/*!
    @addtogroup nbcommon
    @{
*/

extern "C"
{
    #include "paltypes.h"
    #include "nberror.h"
}

#include <vector>
#include "datastream.h"

namespace nbcommon
{

// Types ........................................................................................

/*! Data stream implementation */
class DataStreamImplementation : public DataStream
{
public:
    // Public functions .........................................................................

    /*! DataStreamImplementation constructor */
    DataStreamImplementation();

    /*! DataStreamImplementation destructor */
    virtual ~DataStreamImplementation();

    /* See description in 'datastream.h' */
    virtual NB_Error AppendData(const uint8* buffer,
                                uint32 bufferSize);
    virtual uint32 GetData(uint8* buffer,
                           uint32 position,
                           uint32 requestedSize) const;
    virtual uint32 GetDataSize() const;

    virtual NB_Error DumpToPALFile(PAL_File* file) const;

private:
    // Private types ............................................................................

    /*! Data block */
    class DataBlock
    {
    public:
        /* See source file for description */

        DataBlock();
        virtual ~DataBlock();
        NB_Error Initialize(const uint8* buffer,
                            uint32 bufferSize);
        void Reset();

    private:
        // Copy constructor and assignment operator are not supported.
        DataBlock(const DataBlock& dataBlock);
        DataBlock& operator=(const DataBlock& dataBlock);

    public:
        uint8* m_data;      /*!< Data of this block */
        uint32 m_size;      /*!< Size of this block */
    };

    typedef shared_ptr<DataBlock> DataBlockSharedPointer;


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    DataStreamImplementation(const DataStreamImplementation& dataStream);
    DataStreamImplementation& operator=(const DataStreamImplementation& dataStream);


private:
    // Private members ..........................................................................

    uint32 m_totalSize;                                     /*!< Total data size of all data blocks */
    std::vector<DataBlockSharedPointer> m_dataBlocks;       /*!< Vector of 'DataBlock' items */
};

};  // namespace nbmap

/*! @} */

#endif  // DATASTREAMIMPLEMENTATION_H
