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

#include "MdhdAtom.h"

namespace pal
{
    namespace audio
    {
        MdhdAtom::MdhdAtom(int size, std::string& name)
            : AACAtom(size, name),
              m_duration(-1),
              m_timeScale(-1)
        {
            m_data = new char[12];
            m_dataSize = 12;
            m_postData = new char[size-8-16-4];
            m_postDataSize = size-8-16-4;
        }

        MdhdAtom::~MdhdAtom()
        {
            delete[] m_postData;
            m_postData = NULL;
        }

        void MdhdAtom::ReadAtom(std::stringstream& istream)
        {
            istream.read(m_data, m_dataSize);
            istream.read((char*)&m_timeScale, sizeof(m_timeScale));
            m_timeScale = BigEndianToLittleEndian(m_timeScale);
            istream.read((char*)&m_duration, sizeof(m_duration));
            m_duration = BigEndianToLittleEndian(m_duration);
            istream.read(m_postData, m_postDataSize);
        }

        void MdhdAtom::WriteAtom(std::stringstream& ostream)
        {
            m_size = LittleEndianToBigEndian(m_size);
            ostream.write((char*)&m_size, sizeof(m_size));
            ostream << m_name;
            ostream.write(m_data, m_dataSize);
            m_timeScale = LittleEndianToBigEndian(m_timeScale);
            ostream.write((char*)&m_timeScale, sizeof(m_timeScale));
            m_duration = LittleEndianToBigEndian(m_duration);
            ostream.write((char*)&m_duration, sizeof(m_duration));
            ostream.write(m_postData, m_postDataSize);
        }
    }
}