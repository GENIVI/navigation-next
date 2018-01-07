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

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "base.h"

namespace pal
{
    namespace audio
    {
        class AACAtom : public Base
        {
        public:
            AACAtom();
            AACAtom(int size, std::string& name);
            AACAtom(int size, std::string& name, char* data, int dataSize);
            virtual ~AACAtom();

            void SetParent(AACAtom* atom)   { m_parent = atom; }
            void SetSize(int size)          { m_size = size; }
            void SetOffset(int offset)      { m_offset = offset; }
            void SetName(std::string& name) { m_name = name; }
            void SetData(char* data, int dataSize)
            {
                if (m_data)
                {
                    delete[] m_data;
                }
                m_data = data;
                m_dataSize = dataSize;
            }
            void AddChild(AACAtom* child);
            void UpdateSize(int amount);

            int Size() const        { return m_size; }
            int Offset() const      { return m_offset; }
            std::string& Name()     { return m_name; }
            char* Data()            { return m_data; }
            int   DataSize();
            AACAtom* Parent()       { return m_parent; }
            AACAtom* Child(int idx) { return m_children[idx]; }

            virtual void ReadAtom(std::stringstream& istream);
            virtual void WriteAtom(std::stringstream& ostream);

            static int BigEndianToLittleEndian(int input);
            static int LittleEndianToBigEndian(int input);

        public:
            static std::string ROOT_NAME;

        protected:
            AACAtom*              m_parent;
            std::vector<AACAtom*> m_children;
            int                   m_size;
            std::string           m_name;
            char*                 m_data;
            int                   m_dataSize;
            int                   m_offset;

        };
    }
}