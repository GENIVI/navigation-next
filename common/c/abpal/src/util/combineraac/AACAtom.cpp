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

#include "AACAtom.h"

namespace pal
{
    namespace audio
    {
        std::string AACAtom::ROOT_NAME = "root";

        AACAtom::AACAtom()
            : m_parent(NULL),
              m_size(-1),
              m_data(NULL),
              m_dataSize(-1),
              m_offset(-1)
        {
        }

        AACAtom::AACAtom(int size, std::string& name)
            : m_parent(NULL),
              m_size(size),
              m_name(name),
              m_data(NULL),
              m_dataSize(-1),
              m_offset(-1)

        {
        }

        AACAtom::AACAtom(int size, std::string& name, char* data, int dataSize)
            : m_parent(NULL),
              m_size(size),
              m_name(name),
              m_data(data),
              m_dataSize(dataSize),
              m_offset(-1)
        {
        }

        AACAtom::~AACAtom()
        {
            delete[] m_data;
            m_data = NULL;
        }

        int AACAtom::BigEndianToLittleEndian(int input)
        {
            int swapped = ((input>>24)&0xff)    |   // move byte 3 to byte 0
                          ((input<<8)&0xff0000) |   // move byte 1 to byte 2
                          ((input>>8)&0xff00)   |   // move byte 2 to byte 1
                          ((input<<24)&0xff000000); // byte 0 to byte 3

            return swapped;
        }

        int AACAtom::LittleEndianToBigEndian(int input)
        {
            int swapped = ((input>>24)&0xff)    |   // move byte 3 to byte 0
                          ((input<<8)&0xff0000) |   // move byte 1 to byte 2
                          ((input>>8)&0xff00)   |   // move byte 2 to byte 1
                          ((input<<24)&0xff000000); // byte 0 to byte 3

            return swapped;
        }

        int AACAtom::DataSize()
        {
            return m_dataSize != -1 ? m_dataSize : m_size;
        }

        void AACAtom::AddChild(AACAtom* child)
        {
            child->SetParent(this);
            m_children.push_back(child);
        }

        void AACAtom::UpdateSize(int amount)
        {
            m_size += amount;
            if (m_parent)
            {
                m_parent->UpdateSize(amount);
            }
        }

        void AACAtom::ReadAtom(std::stringstream& istream)
        {
            delete[] m_data;
            m_data = new(std::nothrow) char[m_size - 8];
            if (m_data)
            {
                istream.read(m_data, m_size - 8);
                m_dataSize = m_size - 8;
            }
        }

        void AACAtom::WriteAtom(std::stringstream& ostream)
        {
            if (m_name != "root")
            {
                m_size = LittleEndianToBigEndian(m_size);
                ostream.write((char*)&m_size, sizeof(m_size));
                ostream << m_name;
                if (m_data)
                {
                    ostream.write(m_data, DataSize());
                }
            }

            AACAtom* atom = NULL;
            for (unsigned int i=0; i < m_children.size(); ++i)
            {
                atom = m_children[i];
                atom->WriteAtom(ostream);
            }
        }
    }
}