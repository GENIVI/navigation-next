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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * format.h: created 2004/12/20 by Mark Goddard.
 */

#include "nbplace.h"
//#include "nimplacesynctypes.h"
#include "navpublictypes.h"
#include "cslutil.h"

#define FORMATTED_PHONE_LEN 35 // Formerly (NIM_PHONE_COUNTRY_LEN + NIM_PHONE_AREA_LEN + NIM_PHONE_NUMBER_LEN + 10) // 10 for good measure

void		format_float(double v, char* pszString, int nSize, int nDec);
void		format_float_zeropad(double v, char* pszString, int nSize, int nDec);
boolean		format_heading(double head, char* pszString, int nSize);
boolean		format_heading_dir(double v, char* pszString, int nSize);
void		format_distance_f(double dist, char* szoutdist, int nSizeDist, boolean showunits, double dec_limit, boolean round, NB_NavigateAnnouncementUnits units, boolean shortunitnames);
void		format_distance_ex_f(double dist, char* szoutdist, int nSizeDist, boolean showunits, double dec_limit, boolean round, NB_NavigateAnnouncementUnits units, boolean shortunitnames, double smallunitthreshold);
void		format_latlon(double v, char* pszString, int nSize, boolean islat);
void		format_tel_w(const char* tel, int len, char* buf, int nSize);
void		unformat_tel_w(const char* tel, char* buf, int nSize);
void		seconds_to_h_m_s(uint32 seconds_in, uint32* hours, uint32* minutes, uint32* seconds);
void		format_time_n(uint32 time, char* szouttime, int nSizeTime, boolean showunits, boolean showseconds, boolean roundLT1min);
void		format_contact_plus_number(const char* name, const char* number, char* pszString, int nSize);
void		format_contact_name(const char* name, const char* number, char* pszString, int nSize);
void		unformat_contact_plus_number(const char* input, char* pszNumber, int nSize);
void		format_time_clock_n(uint32 time, char* szouttime, int nSizeTime, boolean showseconds);

