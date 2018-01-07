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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

// AACCombiner.cpp : Defines the entry point for the console application.
//

#include <fstream>
#include "AACCombiner.h"
#include "AACFile.h"
#include "AACAtom.h"
#include "MdhdAtom.h"
#include "MvhdAtom.h"
#include "StcoAtom.h"
#include "StscAtom.h"
#include "StszAtom.h"
#include "SttsAtom.h"
#include "TkhdAtom.h"

namespace pal
{
    namespace audio
    {
    	bool AACCombainer::Combine(byte* buf, uint32 bufSize)
    	{
            std::string tmpData(reinterpret_cast<const char*>(buf), bufSize);
    		// Initalize when this is the first audio file.
    		m_resultBuffer.seekg(0, std::ios::end);
            int size = static_cast<int>(m_resultBuffer.tellg());
            m_resultBuffer.seekg(0, std::ios::beg);
            // TODO: futher seems to be a workaround of Dikum stringbuffer::str buggy implementation, to be rewritten.
            if (size <= 0)
            {
                m_resultBuffer.str(tmpData);
                if (m_resultBuffer.good())
                {
                    return true;
                }
                else
                {
                	if (m_resultBuffer.bad())
                	{
                		m_resultBuffer.clear();
                	}
                	else if (m_resultBuffer.eof())
                	{
                		m_resultBuffer.clear();
                	}
                	else if (m_resultBuffer.fail())
					{
						m_resultBuffer.clear();
					}
                	m_resultBuffer.write(reinterpret_cast<const char*> (buf), bufSize);
                	if (m_resultBuffer.good())
                    {
                        return true;
                    }
                	else if (m_resultBuffer.bad())
                	{
                		m_resultBuffer.clear();
                	}
                	else if (m_resultBuffer.eof())
                	{
                		m_resultBuffer.clear();
                	}
                	else if (m_resultBuffer.fail())
					{
						m_resultBuffer.clear();
					}
                    return false;
                }
            }

            // get bytes of file, create DataInputStreams to be read into AAC_file structures;
            std::stringstream newBuffer(tmpData);
            //newBuffer.write(data.data(), data.size());
            AACFile A;
            AACFile B;
            if (!A.Read(m_resultBuffer))
            {
                return false;
            }
            if (!B.Read(newBuffer))
            {
                return false;
            }

            // combind duration of each file
            A.mvhdAtom->UpdateDuration(B.mvhdAtom->Duration());
            A.tkhdAtom->UpdateDuration(B.tkhdAtom->Duration());
            A.mdhdAtom->UpdateDuration(B.mdhdAtom->Duration());

            // update sample count
            A.sttsAtom->UpdateSampleCount(B.sttsAtom->SampleCount());

            // combind data within like blocks and update their sizes, keep track of how many bytes were added
            int bytesAdded = 0;
            bytesAdded += A.stscAtom->AddData(A.stcoAtom->ChunkCount(), B.stscAtom->DataCount(), B.stscAtom->Table());
            bytesAdded += A.stszAtom->AddData(B.stszAtom->DataCount(), std::string(B.stszAtom->Data(), B.stszAtom->DataSize()));
            bytesAdded += A.stcoAtom->AddData(A.mdatAtom->Size(), B.stcoAtom->DataCount(), B.stcoAtom->m_offsetTable);

            // update Free Atom so it ends at file byte position 0xFF7
            // by subtracting added bytes from free block.
            int freeSize = A.freeAtom->Size() - bytesAdded;
            char* freeData = new char[freeSize-8];    // remove size/tag from free data
            if (!freeData)
            {
                return false;
            }

            memcpy(freeData, A.freeAtom->Data(), freeSize-8);
            A.freeAtom->SetSize(freeSize);
            A.freeAtom->SetData(freeData, freeSize-8);

            // update mdat Atom by appending data from newData
            int mdatSize = A.mdatAtom->Size() + B.mdatAtom->Size() - 8; // remove size/tag from B
            A.mdatAtom->SetSize(mdatSize);
            int mdatDataStreamSize = A.mdatAtom->DataSize() + B.mdatAtom->DataSize();
            char* mdatData = new char[mdatDataStreamSize];
            if (!mdatData)
            {
                return false;
            }

            memcpy(mdatData, A.mdatAtom->Data(), A.mdatAtom->DataSize());
            memcpy(mdatData + A.mdatAtom->DataSize(), B.mdatAtom->Data(), B.mdatAtom->DataSize());
            A.mdatAtom->SetData(mdatData, mdatDataStreamSize);

            m_resultBuffer.clear();
            m_resultBuffer.str("");
            if (!A.Write(m_resultBuffer))
            {
                return false;
            }
            m_resultBuffer.flush();

            return true;
    	}
        bool AACCombainer::Combine(std::string& data)
        {
            // Initalize when this is the first audio file.
            m_resultBuffer.seekg(0, std::ios::end);
            int size = static_cast<int>(m_resultBuffer.tellg());
            m_resultBuffer.seekg(0, std::ios::beg);
            if (size <= 0)
            {
                m_resultBuffer.str(data);
                if (m_resultBuffer.good())
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

            // get bytes of file, create DataInputStreams to be read into AAC_file structures;
            std::stringstream newBuffer(data);
            //newBuffer.write(data.data(), data.size());
            AACFile A;
            AACFile B;
            if (!A.Read(m_resultBuffer))
            {
                return false;
            }
            if (!B.Read(newBuffer))
            {
                return false;
            }

            // combind duration of each file
            A.mvhdAtom->UpdateDuration(B.mvhdAtom->Duration());
            A.tkhdAtom->UpdateDuration(B.tkhdAtom->Duration());
            A.mdhdAtom->UpdateDuration(B.mdhdAtom->Duration());

            // update sample count
            A.sttsAtom->UpdateSampleCount(B.sttsAtom->SampleCount());

            // combind data within like blocks and update their sizes, keep track of how many bytes were added
            int bytesAdded = 0;
            bytesAdded += A.stscAtom->AddData(A.stcoAtom->ChunkCount(), B.stscAtom->DataCount(), B.stscAtom->Table());
            bytesAdded += A.stszAtom->AddData(B.stszAtom->DataCount(), std::string(B.stszAtom->Data(), B.stszAtom->DataSize()));
            bytesAdded += A.stcoAtom->AddData(A.mdatAtom->Size(), B.stcoAtom->DataCount(), B.stcoAtom->m_offsetTable);

            // update Free Atom so it ends at file byte position 0xFF7
            // by subtracting added bytes from free block.
            int freeSize = A.freeAtom->Size() - bytesAdded;
            char* freeData = new char[freeSize-8];    // remove size/tag from free data
            if (!freeData)
            {
                return false;
            }

            memcpy(freeData, A.freeAtom->Data(), freeSize-8);
            A.freeAtom->SetSize(freeSize);
            A.freeAtom->SetData(freeData, freeSize-8);

            // update mdat Atom by appending data from newData
            int mdatSize = A.mdatAtom->Size() + B.mdatAtom->Size() - 8; // remove size/tag from B
            A.mdatAtom->SetSize(mdatSize);
            int mdatDataStreamSize = A.mdatAtom->DataSize() + B.mdatAtom->DataSize();
            char* mdatData = new char[mdatDataStreamSize];
            if (!mdatData)
            {
                return false;
            }

            memcpy(mdatData, A.mdatAtom->Data(), A.mdatAtom->DataSize());
            memcpy(mdatData + A.mdatAtom->DataSize(), B.mdatAtom->Data(), B.mdatAtom->DataSize());
            A.mdatAtom->SetData(mdatData, mdatDataStreamSize);

            m_resultBuffer.clear();
            m_resultBuffer.str("");
            if (!A.Write(m_resultBuffer))
            {
                return false;
            }
            m_resultBuffer.flush();

            return true;
        }

        double AACUtil::GetDuration(byte* buf, uint32 bufSize)
        {
            std::stringstream buffer;
            buffer.write(reinterpret_cast<const char*> (buf), bufSize);
            if (!buffer.good())
            {
                return -1.0;
            }
            AACFile A;
            if (!A.Read(buffer))
            {
                return -1.0;
            }

            double duration = 0.0;

            duration = A.mvhdAtom->DurationNormalized();
            if (duration > 0.0)
            {
                return duration;
            }

            duration = A.mdhdAtom->DurationNormalized();
            if (duration > 0.0)
            {
                return duration;
            }

            return -1.0;
        }
    }
}
