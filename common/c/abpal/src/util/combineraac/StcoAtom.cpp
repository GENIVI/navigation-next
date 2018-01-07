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

#include "StcoAtom.h"

namespace pal
{
    namespace audio
    {
        int StcoAtom::DATASIZE = 4;

        StcoAtom::StcoAtom(int size, std::string& name)
            : AACAtom(size, name)
        {
        }

        StcoAtom::~StcoAtom()
        {
        }

        int StcoAtom::AddData(int dataSize, int itemCount, std::vector<int>& newData)
        {
            int rValue = itemCount * DATASIZE;
            // update size with new data (4 bytes per count)
            // update frameCount
            // append new data
            UpdateSize(rValue);

            std::vector<int> newTable;
            // copy old items to new list
            for (int i=0; i < m_dataCount; ++i)
            {
                newTable.push_back(m_offsetTable[i]);
            }

            // copy new items to new list with updated offsets
            for (int i=0; i < itemCount; ++i)
            {
                newTable.push_back(newData[i] + dataSize - 8);
            }

            // increase count by new items, use new table
            m_dataCount += itemCount;
            m_offsetTable = newTable;

            return rValue;
        }

        void StcoAtom::ReadAtom(std::stringstream& istream)
        {
            istream.read((char*)&m_versionFlags, sizeof(m_versionFlags));
            m_versionFlags = BigEndianToLittleEndian(m_versionFlags);
            istream.read((char*)&m_dataCount, sizeof(m_dataCount));
            m_dataCount = BigEndianToLittleEndian(m_dataCount);

            m_offsetTable.clear();
            for (int i=0; i < m_dataCount; ++i)
            {
                int value = 0;
                istream.read((char*)&value, sizeof(value));
                m_offsetTable.push_back(BigEndianToLittleEndian(value));
            }
        }

        void StcoAtom::WriteAtom(std::stringstream& ostream)
        {
            m_size = LittleEndianToBigEndian(m_size);
            ostream.write((char*)&m_size, sizeof(m_size));
            ostream << m_name;
            m_versionFlags = LittleEndianToBigEndian(m_versionFlags);
            ostream.write((char*)&m_versionFlags, sizeof(m_versionFlags));
            int dataCount = m_dataCount;
            m_dataCount = LittleEndianToBigEndian(m_dataCount);
            ostream.write((char*)&m_dataCount, sizeof(m_dataCount));
            for (int i = 0; i < dataCount; ++i)
            {
                m_offsetTable[i] = LittleEndianToBigEndian(m_offsetTable[i]);
                ostream.write((char*)&m_offsetTable[i], sizeof(m_offsetTable[i]));
            }
        }
    }
}