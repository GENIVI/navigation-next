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

#include "AACAtom.h"
#include "base.h"

namespace pal
{
    namespace audio
    {
        class MvhdAtom;
        class TkhdAtom;
        class MdhdAtom;
        class SttsAtom;
        class StscAtom;
        class StszAtom;
        class StcoAtom;

        class AACFile : public Base
        {
        public:
            AACFile();
            ~AACFile();

            AACAtom* Read(std::stringstream& istream);
            bool Write(std::stringstream& ostream);
            bool DataAvailable(std::stringstream& istream);
            int GetAtomSize(std::stringstream& istream);
            std::string GetAtomName(std::stringstream& istream);
            std::string GetAtomData(std::stringstream& istream, int byteCount);
            void BackToParent();
            void DownToChild();
            bool DoFTYP(std::stringstream& istream);
            bool DoMOOV(std::stringstream& istream);
            bool DoMVHD(std::stringstream& istream);
            bool DoTRAK(std::stringstream& istream);
            bool DoTKHD(std::stringstream& istream);
            bool DoMDIA(std::stringstream& istream);
            bool DoMDHD(std::stringstream& istream);
            bool DoHDLR(std::stringstream& istream);
            bool DoMINF(std::stringstream& istream);
            bool DoSMHD(std::stringstream& istream);
            bool DoDINF(std::stringstream& istream);
            bool DoSTBL(std::stringstream& istream);
            bool DoSTSD(std::stringstream& istream);
            bool DoSTTS(std::stringstream& istream);
            bool DoSTSC(std::stringstream& istream);
            bool DoSTSZ(std::stringstream& istream);
            bool DoSTCO(std::stringstream& istream);
            bool DoUDTA(std::stringstream& istream);
            bool DoFREE(std::stringstream& istream);
            bool DoMDAT(std::stringstream& istream);

        public:
            static std::string ftyp;
            static std::string moov;
            static std::string mvhd;
            static std::string trak;
            static std::string tkhd;
            static std::string mdia;
            static std::string mdhd;
            static std::string hdlr;
            static std::string minf;
            static std::string smhd;
            static std::string dinf;
            static std::string stsd;
            static std::string stts;
            static std::string stsc;
            static std::string stsz;
            static std::string stco;
            static std::string udta;
            static std::string free;
            static std::string mdat;
            static std::string stbl;

            AACAtom*  ftypAtom;
            AACAtom*  hdlrAtom;
            AACAtom*  smhdAtom;
            AACAtom*  dinfAtom;
            AACAtom*  stsdAtom;
            AACAtom*  udtaAtom;
            AACAtom*  moovAtom;
            MvhdAtom* mvhdAtom;
            AACAtom*  trakAtom;
            TkhdAtom* tkhdAtom;
            AACAtom*  mdiaAtom;
            MdhdAtom* mdhdAtom;
            AACAtom*  minfAtom;
            AACAtom*  stblAtom;
            SttsAtom* sttsAtom;
            StscAtom* stscAtom;
            StszAtom* stszAtom;
            StcoAtom* stcoAtom;
            AACAtom*  freeAtom;
            AACAtom*  mdatAtom;

        private:
            AACAtom     m_root;
            AACAtom*    m_currentAtom;
            AACAtom*    m_parentAtom;
            int         m_currentSize;
            std::string m_currentName;
        };
    }
}