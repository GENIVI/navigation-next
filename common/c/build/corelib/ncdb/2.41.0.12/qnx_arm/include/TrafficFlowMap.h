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

#ifndef _TRAFFIC_FLOW_MAP_H_INCLUDED_
#define _TRAFFIC_FLOW_MAP_H_INCLUDED_

#ifndef WIN32
    #include <stdint.h>
#endif
#include <string>
#include "Session.h"
#include "VectorTile.h"

namespace NIM_Route{class tl;}

typedef unsigned int uint32_t;

using std::string;
using namespace Ncdb;

namespace Ncdb {

class NCDB_API TrafficFlowMap
{
 public:
  TrafficFlowMap(uint32_t width, uint32_t height);
  ~TrafficFlowMap();

  //Mobius session; using Mobius API to read
  //geography from the map data
  void SetSession(Session& session);

  void SetFeedSource(const string& source);
  void SetProvider(const char* provider);
  void SetCenter(double lat, double lon);
  void SetScale(double scale);
  void SetRank(uint32_t maxRank);
  void SetRotation(double degrees);

  //If this call succeeds, the caller has to release
  //the memory pointed to by *imgBuff using 'array
  //delete' operator
  bool GetImage(char** imgBuff, uint32_t& buffLen);

 private:
  WorldRect GetBoundingBox();
  char GetFlowDirection(const string& tmcCode);

  bool HasConstruction(AutoArray<AscString>& tmcs, string& tmc_code);
  bool GetSpeedRatio(AutoArray<AscString>& tmcs, string& tmc_code, float& ratio);

  void PrintArgs();

  uint32_t m_width;
  uint32_t m_height;
  Session* m_pSession;
  UtfString m_feedSource;
  char m_provider[32];
  double m_lat;
  double m_lon;
  uint32_t m_featureRank;
  double m_scale;
  double m_rotationAngle;
  NIM_Route::tl * m_trafficFeed;
};

}
#endif //_TRAFFIC_FLOW_MAP_H_INCLUDED_
