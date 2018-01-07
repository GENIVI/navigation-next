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

#include "SttsAtom.h"

namespace pal
{
    namespace audio
    {
        int SttsAtom::DATASIZE = 8;

        SttsAtom::SttsAtom(int size, std::string& name)
            : AACAtom(size, name)
        {
        }

        SttsAtom::~SttsAtom()
        {
        }

        void SttsAtom::ReadAtom(std::stringstream& istream)
        {
            istream.read((char*)&m_versionFlags, sizeof(m_versionFlags));
            m_versionFlags = BigEndianToLittleEndian(m_versionFlags);
            istream.read((char*)&m_dataCount, sizeof(m_dataCount));
            m_dataCount = BigEndianToLittleEndian(m_dataCount);

            //NOTE: following specs, we add table items.
            // 8 bytes per frame
            //data = new byte[ dataCount * DATASIZE ];
            //dis.read( data );

            // not following specs (Ashish's proven way)
            istream.read((char*)&m_sampleCount, sizeof(m_sampleCount));
            m_sampleCount = BigEndianToLittleEndian(m_sampleCount);
            istream.read((char*)&m_sampleDuration, sizeof(m_sampleDuration));
            m_sampleDuration = BigEndianToLittleEndian(m_sampleDuration);
        }

        void SttsAtom::WriteAtom(std::stringstream& ostream)
        {
            m_size = LittleEndianToBigEndian(m_size);
            ostream.write((char*)&m_size, sizeof(m_size));
            ostream << m_name;
            m_versionFlags = LittleEndianToBigEndian(m_versionFlags);
            ostream.write((char*)&m_versionFlags, sizeof(m_versionFlags));
            m_dataCount = LittleEndianToBigEndian(m_dataCount);
            ostream.write((char*)&m_dataCount, sizeof(m_dataCount));
            m_sampleCount = LittleEndianToBigEndian(m_sampleCount);
            ostream.write((char*)&m_sampleCount, sizeof(m_sampleCount));
            m_sampleDuration = LittleEndianToBigEndian(m_sampleDuration);
            ostream.write((char*)&m_sampleDuration, sizeof(m_sampleDuration));
        }
    }
}