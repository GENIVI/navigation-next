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

#include "StscAtom.h"

namespace pal
{
    namespace audio
    {
        int StscAtom::DATASIZE = 12;

        TableItem::TableItem(std::stringstream& istream)
        : m_chunkID(0),
          m_chunkSample(0),
          m_descriptionID(0)
        {


            istream.read((char*)&m_chunkID, sizeof(m_chunkID));
            m_chunkID = AACAtom::BigEndianToLittleEndian(m_chunkID);
            istream.read((char*)&m_chunkSample, sizeof(m_chunkSample));
            m_chunkSample = AACAtom::BigEndianToLittleEndian(m_chunkSample);
            istream.read((char*)&m_descriptionID, sizeof(m_descriptionID));
            m_descriptionID = AACAtom::BigEndianToLittleEndian(m_descriptionID);
        }

        void TableItem::Write(std::stringstream& ostream)
        {
            m_chunkID = AACAtom::LittleEndianToBigEndian(m_chunkID);
            ostream.write((char*)&m_chunkID, sizeof(m_chunkID));
            m_chunkSample = AACAtom::LittleEndianToBigEndian(m_chunkSample);
            ostream.write((char*)&m_chunkSample, sizeof(m_chunkSample));
            m_descriptionID = AACAtom::LittleEndianToBigEndian(m_descriptionID);
            ostream.write((char*)&m_descriptionID, sizeof(m_descriptionID));
        }

        StscAtom::StscAtom(int size, std::string& name)
            : AACAtom(size, name)
        {
        }

        StscAtom::~StscAtom()
        {
        }

        int StscAtom::AddData(int chunkCount, int itemCount, std::vector<TableItem>& newData)
        {
            int rValue = itemCount * DATASIZE;
            // update size with new data (12 bytes per count)
            // update frameCount
            // append new data
            // make new byte[] with size of this.data & newFrameData

            // We must keep new chunkID's relative:
            // file A has chunk table as:
            //    1a, 2a, 3a
            // file B has chunk table as:
            //    1b, 4b, 6b
            // combined table must be:
            // 1a,2a,3a, 4b,7b,9b

            for (int i=0; i < itemCount; ++i)
            {
                TableItem& item = newData[i];
                item.m_chunkID += chunkCount;
                m_chunkTable.push_back(item);
            }

            UpdateSize(itemCount * DATASIZE);
            m_dataCount += itemCount;

            return rValue;
        }

        void StscAtom::ReadAtom(std::stringstream& istream)
        {
            istream.read((char*)&m_versionFlags, sizeof(m_versionFlags));
            m_versionFlags = BigEndianToLittleEndian(m_versionFlags);
            istream.read((char*)&m_dataCount, sizeof(m_dataCount));
            m_dataCount = BigEndianToLittleEndian(m_dataCount);

            for (int i=0; i < m_dataCount; ++i)
            {
                TableItem item(istream);
                m_chunkTable.push_back(item);
            }
        }

        void StscAtom::WriteAtom(std::stringstream& ostream)
        {
            m_size = LittleEndianToBigEndian(m_size);
            ostream.write((char*)&m_size, sizeof(m_size));
            ostream << m_name;
            m_versionFlags = LittleEndianToBigEndian(m_versionFlags);
            ostream.write((char*)&m_versionFlags, sizeof(m_versionFlags));
            int dataCount = m_dataCount;
            m_dataCount = LittleEndianToBigEndian(m_dataCount);
            ostream.write((char*)&m_dataCount, sizeof(m_dataCount));

            for (int i=0; i < dataCount; ++i)
            {
                TableItem& item = m_chunkTable[i];
                item.Write(ostream);
            }
        }
    }
}
