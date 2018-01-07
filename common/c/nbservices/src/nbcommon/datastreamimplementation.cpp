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

    @file       datastreamimplementation.cpp

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

extern "C"
{
#include "palstdlib.h"
#include "palfile.h"
}
#include "nbmacros.h"

#include "datastreamimplementation.h"

/*! @{ */

using namespace nbcommon;

// Local Constants ..............................................................................


// Local Types ..................................................................................


// Public functions .............................................................................

/* See header file for description */
DataStreamImplementation::DataStreamImplementation() : m_totalSize(0),
                                                       m_dataBlocks()
{
    // Nothing to do here.
}

/* See header file for description */
DataStreamImplementation::~DataStreamImplementation()
{
    // Nothing to do here.
}

/* See description in 'datastream.h' */
NB_Error
DataStreamImplementation::AppendData(const uint8* buffer,
                                     uint32 bufferSize)
{
    if ((!buffer) || (bufferSize == 0))
    {
        return NE_INVAL;
    }

    // Create a new 'DataBlock' object.
    DataBlockSharedPointer newBlock(CCC_NEW DataBlock());
    if (!newBlock)
    {
        return NE_NOMEM;
    }

    // Initialize the 'DataBlock' object with a buffer.
    NB_Error error = newBlock->Initialize(buffer, bufferSize);
    if (error != NE_OK)
    {
        return error;
    }

    // Take ownership of the 'DataBlock' object.
    m_dataBlocks.push_back(newBlock);

    // Update the total size of data.
    m_totalSize += bufferSize;

    return error;
}

/* See description in 'datastream.h' */
uint32
DataStreamImplementation::GetData(uint8* buffer,
                                  uint32 position,
                                  uint32 requestedSize) const
{
    uint32 remainedSize = requestedSize;
    uint32 bufferOffset = 0;

    if ((!buffer) || (requestedSize == 0))
    {
        // The function returns the number of bytes read, so return zero
        return 0;
    }

    // Loop to find the position and get data of requested size from the found position.
    std::vector<DataBlockSharedPointer>::const_iterator iterator = m_dataBlocks.begin();
    std::vector<DataBlockSharedPointer>::const_iterator end = m_dataBlocks.end();
    for (; (iterator != end) && (remainedSize > 0); ++iterator)
    {
        uint32 blockSize = 0;
        const DataBlockSharedPointer& block = *iterator;

        // This pointer to 'DataBlock' object should be not 'NULL'.
        if (!block)
        {
            // The function returns the number of bytes read, so return zero
            return 0;
        }

        // Get the data size.
        blockSize = block->m_size;

        // Check if the position is in this block.
        if (position >= blockSize)
        {
            position -= blockSize;
            continue;
        }

        // Copy data.
        {
            uint32 copiedSize = std::min(remainedSize, blockSize - position);

            nsl_memcpy(buffer + bufferOffset, block->m_data + position, copiedSize);
            position = 0;
            bufferOffset += copiedSize;
            remainedSize -= copiedSize;
        }
    }

    return (requestedSize - remainedSize);
}

/* See description in 'datastream.h' */
uint32
DataStreamImplementation::GetDataSize() const
{
    return m_totalSize;
}


/* See description in header file. */
NB_Error DataStreamImplementation::DumpToPALFile(PAL_File* file) const
{
    if (!file)
    {
        return NE_INVAL;
    }

    if (PAL_FileTruncate(file, 0) != PAL_Ok)
    {
        return NE_FSYS;
    }

    vector<DataBlockSharedPointer>::const_iterator iterator = m_dataBlocks.begin();
    vector<DataBlockSharedPointer>::const_iterator end      = m_dataBlocks.end();
    for (; iterator != end; ++iterator)
    {
        const DataBlockSharedPointer& block = *iterator;

        // This pointer to 'DataBlock' object should be not 'NULL'.
        if (!block)
        {
            return NE_UNEXPECTED;
        }

        uint32 dataSize     = block->m_size;
        uint8* position      = block->m_data;
        uint32 bytesWritten = 0;
        do
        {
            if (PAL_FileWrite(file, position, dataSize, &bytesWritten) != PAL_Ok)
            {
                return NE_FSYS;
            }
            dataSize -= bytesWritten;
            position += bytesWritten;
        } while (dataSize > 0);
    }

    PAL_FileFlush(file);
    return NE_OK;
}


// Private functions ............................................................................


// DataBlock functions ..........................................................................

/*! DataBlock constructor */
DataStreamImplementation::DataBlock::DataBlock() : m_data(NULL),
                                                   m_size(0)
{
    // Nothing to do here.
}

/*! DataBlock destructor */
DataStreamImplementation::DataBlock::~DataBlock()
{
    Reset();
}

/*! Initialize a 'DataBlock' object with a buffer

    @return NE_OK if success
*/
NB_Error
DataStreamImplementation::DataBlock::Initialize(const uint8* buffer,/*!< Buffer copied to initialize this object */
                                                uint32 bufferSize   /*!< Size of buffer */
                                                )
{
    if ((!buffer) || (bufferSize == 0))
    {
        return NE_INVAL;
    }

    /* The 'DataBlock' object could be reset and initialized
       with a new buffer.
    */
    Reset();

    // Create a new buffer.
    m_data = CCC_NEW uint8[bufferSize];
    if (!m_data)
    {
        return NE_NOMEM;
    }
    nsl_memcpy(m_data, buffer, bufferSize);

    m_size = bufferSize;

    return NE_OK;
}

/*! Reset a 'DataBlock' object to be uninitialized

    Clear all members in this 'DataBlock' object.

    @return None
*/
void
DataStreamImplementation::DataBlock::Reset()
{
    if (m_data)
    {
        delete [] m_data;
        m_data = NULL;
    }
    m_size = 0;
}

/*! @} */
