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
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
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

//! **********************************************************************
//! Create 03/20/2010 
//! This file for creating a link scanner which can enumerate 
//! all TMC links in one state.
//!
//!
//! **********************************************************************
#pragma once
#ifndef NCDB_TMC_LINK_SCANNER_H
#define NCDB_TMC_LINK_SCANNER_H

//#include <ncdb_engine.h>

#include "NcdbTypes.h"
#include "AutoArray.h"
#include "UtfString.h"

//using namespace std;
//using namespace Ncdb;

class Map;
class MapChunk;

namespace Ncdb{

class NCDB_API AutoArrayInt : public AutoArray<int> {};

//! class for iterating TMC link in one state
class NCDB_API TmcLinkIterator
{
public:
	//! constructor and destructor
	TmcLinkIterator();
	~TmcLinkIterator();

private:
	//! map for lookup link
	Map* m_mapForLink;

	MapChunk* m_chunk;

	//! blocks index and max blocks
	int m_fileBinCnt; 
	int m_curFileBin;

	//! current link and max link pos
	int m_curPos;
	int m_maxPos;

	//! country id and state id
	int m_countryID;
	int m_stateIdx;

	//! state fpos range
	int m_stateFposStart;
	int m_stateFposEnd;

	//! save blocks index for iterator
	AutoArrayInt m_vBlocks;

	//! save results
	AutoArray<NodeSegmentID> m_vNodeSegID;
	
public:
	//! Initialize iterator
	//! 
	//! @param[in] Session: map session
	//! @param[in] country: country code
	//! @param[in] state:  state name, dump this links in state
	//! @return 
	//!		NCDB_OPEN_ERR: can not open db files
	//!		NCDB_INVALID: parameters invalid, such wrong country or state
	//!		NCDB_NOT_FOUND: no records
	//!		NCDB_OK: init ok
	ReturnCode Init(Session&, UtfString country, UtfString state);

	//! main function
	//! return all links
	//! 
	//! @param[out] NodeSegmentID
	//! @return 
	//!		NCDB_END_OF_QUERY: no more records to read
	//!		NCDB_OK:	get a record
	ReturnCode NextNodeSegmentID(NodeSegmentID&);

private:
	//! set map pointer
	int SetMap(Map*, AutoArrayInt&);

	//! country id, lookup link for this country
	void SetCountry(int countryID) {m_countryID = countryID;}

	//! state idx, not id, just for this state
	void SetState(int stateIdx) {m_stateIdx = stateIdx;}

	//! bins
	void SetBlocks(AutoArrayInt& vbs) {m_vBlocks = vbs;}

	//! a link is in a state?
	void SetStateFposRange(long fposStart, long fposEnd)
	{
		m_stateFposStart = fposStart;
		m_stateFposEnd = fposEnd;
	}

	//! load links of next blocks
	int Reload();

	//! reset all the members if Init() will be called many times.
	void Reset();

private: //for debug
	int m_totalLinks;
	int m_stateLinks;
	int m_badLinks;
};

}; //! namespace NCDB
#endif
