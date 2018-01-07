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

#include <iostream>
#include "AACFile.h"
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
        std::string AACFile::ftyp = "ftyp";
        std::string AACFile::moov = "moov";
        std::string AACFile::mvhd = "mvhd";
        std::string AACFile::trak = "trak";
        std::string AACFile::tkhd = "tkhd";
        std::string AACFile::mdia = "mdia";
        std::string AACFile::mdhd = "mdhd";
        std::string AACFile::hdlr = "hdlr";
        std::string AACFile::minf = "minf";
        std::string AACFile::smhd = "smhd";
        std::string AACFile::dinf = "dinf";
        std::string AACFile::stsd = "stsd";
        std::string AACFile::stts = "stts";
        std::string AACFile::stsc = "stsc";
        std::string AACFile::stsz = "stsz";
        std::string AACFile::stco = "stco";
        std::string AACFile::udta = "udta";
        std::string AACFile::free = "free";
        std::string AACFile::mdat = "mdat";
        std::string AACFile::stbl = "stbl";

        AACFile::AACFile()
            : m_parentAtom(NULL),
              m_currentSize(-1)
        {
            ftypAtom = NULL;
            hdlrAtom = NULL;
            smhdAtom = NULL;
            dinfAtom = NULL;
            stsdAtom = NULL;
            udtaAtom = NULL;
            moovAtom = NULL;
            mvhdAtom = NULL;
            trakAtom = NULL;
            tkhdAtom = NULL;
            mdiaAtom = NULL;
            mdhdAtom = NULL;
            minfAtom = NULL;
            stblAtom = NULL;
            sttsAtom = NULL;
            stscAtom = NULL;
            stszAtom = NULL;
            stcoAtom = NULL;
            freeAtom = NULL;
            mdatAtom = NULL;
        }

        AACFile::~AACFile()
        {
            delete ftypAtom;
            ftypAtom = NULL;
            delete hdlrAtom;
            hdlrAtom = NULL;
            delete smhdAtom;
            smhdAtom = NULL;
            delete dinfAtom;
            dinfAtom = NULL;
            delete stsdAtom;
            stsdAtom = NULL;
            delete udtaAtom;
            udtaAtom = NULL;
            delete moovAtom;
            moovAtom = NULL;
            delete mvhdAtom;
            mvhdAtom = NULL;
            delete trakAtom;
            trakAtom = NULL;
            delete tkhdAtom;
            tkhdAtom = NULL;
            delete mdiaAtom;
            mdiaAtom = NULL;
            delete mdhdAtom;
            mdhdAtom = NULL;
            delete minfAtom;
            minfAtom = NULL;
            delete stblAtom;
            stblAtom = NULL;
            delete sttsAtom;
            sttsAtom = NULL;
            delete stscAtom;
            stscAtom = NULL;
            delete stszAtom;
            stszAtom = NULL;
            delete stcoAtom;
            stcoAtom = NULL;
            delete freeAtom;
            freeAtom = NULL;
            delete mdatAtom;
            mdatAtom = NULL;
        }

        AACAtom* AACFile::Read(std::stringstream& istream)
        {
            int currentPosition = static_cast<int>(istream.tellg());
            istream.seekg(0, std::ios::end);
            m_currentSize = static_cast<int>(istream.tellg());
            istream.seekg(currentPosition, std::ios::beg);

            m_root.SetSize(m_currentSize);
            m_root.SetName(AACAtom::ROOT_NAME);
            m_parentAtom = &m_root;

            while (DataAvailable(istream))
            {
                // get size & name
                m_currentSize = GetAtomSize(istream);
                m_currentName = GetAtomName(istream);
                if (m_currentSize == -1 || m_currentName.empty())
                {
                    return NULL;
                }

                // determine what type:
                if      (m_currentName == ftyp && !DoFTYP(istream)) return NULL;
                else if (m_currentName == moov && !DoMOOV(istream)) return NULL;
                else if (m_currentName == mvhd && !DoMVHD(istream)) return NULL;
                else if (m_currentName == trak && !DoTRAK(istream)) return NULL;
                else if (m_currentName == tkhd && !DoTKHD(istream)) return NULL;
                else if (m_currentName == mdia && !DoMDIA(istream)) return NULL;
                else if (m_currentName == mdhd && !DoMDHD(istream)) return NULL;
                else if (m_currentName == hdlr && !DoHDLR(istream)) return NULL;
                else if (m_currentName == minf && !DoMINF(istream)) return NULL;
                else if (m_currentName == stbl && !DoSTBL(istream)) return NULL;
                else if (m_currentName == smhd && !DoSMHD(istream)) return NULL;
                else if (m_currentName == dinf && !DoDINF(istream)) return NULL;
                else if (m_currentName == stsd && !DoSTSD(istream)) return NULL;
                else if (m_currentName == stts && !DoSTTS(istream)) return NULL;
                else if (m_currentName == stsc && !DoSTSC(istream)) return NULL;
                else if (m_currentName == stsz && !DoSTSZ(istream)) return NULL;
                else if (m_currentName == stco && !DoSTCO(istream)) return NULL;
                else if (m_currentName == udta && !DoUDTA(istream)) return NULL;
                else if (m_currentName == free && !DoFREE(istream)) return NULL;
                else if (m_currentName == mdat && !DoMDAT(istream)) return NULL;

                if (moovAtom && mvhdAtom && trakAtom &&
                    tkhdAtom && mdiaAtom && mdhdAtom &&
                    minfAtom && stblAtom && sttsAtom &&
                    stscAtom && stszAtom && stcoAtom &&
                    freeAtom && mdatAtom && ftypAtom &&
                    hdlrAtom && smhdAtom && dinfAtom &&
                    stsdAtom && udtaAtom)
                {
                    break;
                }

            // need to check for currentAtom size has been exceeded, then go back to parent
            }

            return &m_root;
        }

        bool AACFile::Write(std::stringstream& ostream)
        {
            m_root.WriteAtom(ostream);
            if (ostream.good())
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        bool AACFile::DataAvailable(std::stringstream& istream)
        {
            if (!istream.eof())
            {
                return true;
            }

            return false;
        }

        int AACFile::GetAtomSize(std::stringstream& istream)
        {
            int rValue = -1;
            istream.read((char*)&rValue, sizeof(rValue));
            rValue = AACAtom::BigEndianToLittleEndian(rValue);

            return rValue;
        }

        std::string AACFile::GetAtomName(std::stringstream& istream)
        {
            char name[4] = {0};
            istream.read(name, sizeof(name));
            std::string rName(name, sizeof(name));

            return rName;
        }

        std::string AACFile::GetAtomData(std::stringstream& istream, int byteCount)
        {
            char* data = new char[byteCount];
            istream.read(data, byteCount);
            std::string rData(data, byteCount);
            delete[] data;

            return rData;
        }

        void AACFile::BackToParent()
        {
            if (m_currentAtom)
            {
                m_parentAtom = m_currentAtom->Parent();
                m_currentAtom = m_currentAtom->Parent();
            }
        }

        void AACFile::DownToChild()
        {
            m_parentAtom = m_currentAtom;
            m_currentAtom = NULL; // not necessary but for debuggin, next file read will make new atom current
        }

        bool AACFile::DoFTYP(std::stringstream& istream)
        {
            ftypAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = ftypAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            BackToParent();

            return true;
        }

        bool AACFile::DoMOOV(std::stringstream& istream)
        {
            moovAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = moovAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            // moov has no data, but children
            DownToChild();

            return true;
        }

        bool AACFile::DoMVHD(std::stringstream& istream)
        {
            mvhdAtom = new MvhdAtom(m_currentSize, m_currentName);
            m_currentAtom = mvhdAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoTRAK(std::stringstream& istream)
        {
            trakAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = trakAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            // trak has no data, but children
            DownToChild();

            return true;
        }

        bool AACFile::DoTKHD(std::stringstream& istream)
        {
            tkhdAtom = new TkhdAtom(m_currentSize, m_currentName);
            m_currentAtom = tkhdAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoMDIA(std::stringstream& istream)
        {
            mdiaAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = mdiaAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            // mdia has no data, but children
            DownToChild();

            return true;
        }

        bool AACFile::DoMDHD(std::stringstream& istream)
        {
            mdhdAtom = new MdhdAtom(m_currentSize, m_currentName);
            m_currentAtom = mdhdAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoHDLR(std::stringstream& istream)
        {
            hdlrAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = hdlrAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoMINF(std::stringstream& istream)
        {
            minfAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = minfAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            // minf has no data, but children
            DownToChild();

            return true;
        }

        bool AACFile::DoSMHD(std::stringstream& istream)
        {
            smhdAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = smhdAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoDINF(std::stringstream& istream)
        {
            dinfAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = dinfAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoSTBL(std::stringstream& istream)
        {
            stblAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = stblAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            DownToChild();

            return true;
        }

        bool AACFile::DoSTSD(std::stringstream& istream)
        {
            stsdAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = stsdAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoSTTS(std::stringstream& istream)
        {
            sttsAtom = new SttsAtom(m_currentSize, m_currentName);
            m_currentAtom = sttsAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoSTSC(std::stringstream& istream)
        {
            stscAtom = new StscAtom(m_currentSize, m_currentName);
            m_currentAtom = stscAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoSTSZ(std::stringstream& istream)
        {
            stszAtom = new StszAtom(m_currentSize, m_currentName);
            m_currentAtom = stszAtom;
            if (!m_currentAtom)
            {
                return false;
            }
            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }

        bool AACFile::DoSTCO(std::stringstream& istream)
        {
            stcoAtom = new StcoAtom(m_currentSize, m_currentName);
            m_currentAtom = stcoAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();    // stbl
            BackToParent();    // minf
            BackToParent();    // mdia
            BackToParent();    // trak
            BackToParent();    // moov

            return true;
        }

        bool AACFile::DoUDTA(std::stringstream& istream)
        {
            udtaAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = udtaAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent(); // moov
            BackToParent(); // root

            return true;
        }

        bool AACFile::DoFREE(std::stringstream& istream)
        {
            freeAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = freeAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            BackToParent();

            return true;
        }

        bool AACFile::DoMDAT(std::stringstream& istream)
        {
            mdatAtom = new AACAtom(m_currentSize, m_currentName);
            m_currentAtom = mdatAtom;
            if (!m_currentAtom)
            {
                return false;
            }

            m_currentAtom->ReadAtom(istream);
            if (!istream.good())
            {
                return false;
            }

            m_parentAtom->AddChild(m_currentAtom);
            BackToParent();

            return true;
        }
    }
}