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
 * format.c: created 2004/12/20 by Mark Goddard.
 */

#include "paltypes.h"
#include "palstdlib.h"
#include "palmath.h"
#include "format.h"
//#include "nimresutil.h"
//#include "util.h"
//#include "debuglog.h"

#define METRIC_DISTANCE_ROUND		10
#define ENGLISH_DISTANCE_ROUND		50

void 
format_float(double v, char* pszString, int nSize, int nDec)
{
	boolean bret;
	char	buf[64];
	char* ptxt;
	char* ptxt2;
	boolean pastpoint;
	int16	ndecpassed;
	char	zero[4] = { '0', '.', '0', 0 };

	if (v == 0) {

		nsl_strlcpy(buf, zero, sizeof(buf));
		//WSTRNCOPYN(buf, ARRAYSIZE(buf), zero, -1);
		bret = TRUE;
	}
	else {
		bret = (boolean)nsl_doubletostr(v, buf, sizeof(buf));
	}

	if (bret) {

		for (ptxt = buf; *ptxt == ' ' && *ptxt != 0; ptxt++)
			; /* nothing */

		pastpoint = FALSE;
		ndecpassed = 0;

		for (ptxt2 = ptxt; *ptxt2 != 0; ptxt2++) {
			
			if (pastpoint)
				ndecpassed++;

			if (*ptxt2 == '.')
			{
				pastpoint = TRUE;

				if(nDec == 0)
				{
					*ptxt2 = 0;//don't show decimal point for no sig-fig's.
					break;
				}
			}

			if (ndecpassed > nDec) {
				*ptxt2 = 0;
				break;
			}
		}

		nsl_strlcpy(pszString, ptxt, nSize);
		//WSTRNCOPYN(pszString, nSize/sizeof(AECHAR), ptxt, -1);
	}
}

void 
format_float_zeropad(double v, char* pszString, int nSize, int nDec)
{
	boolean bret;
	char	buf[64];
	char*	ptxt;
	char*	ptxt2;
	boolean pastpoint;
	int16	ndecpassed;
	char	zero[4] = { '0', '.', '0', 0 };
	char	zeroes[4] = { '0', '0', '0', 0 };

	if (v == 0) {

		nsl_strlcpy(buf, zero, sizeof(buf));
		//WSTRNCOPYN(buf, ARRAYSIZE(buf), zero, -1);
		bret = TRUE;
	}
	else {
		bret = (boolean)nsl_doubletostr(v, buf, sizeof(buf));
	}

	if (bret) {

		for (ptxt = buf; *ptxt == ' ' && *ptxt != 0; ptxt++)
			; /* nothing */

		pastpoint = FALSE;
		ndecpassed = 0;

		for (ptxt2 = ptxt; *ptxt2 != 0; ptxt2++) {
			
			if (pastpoint)
				ndecpassed++;

			if (*ptxt2 == '.')
			{
				pastpoint = TRUE;

				if(nDec == 0)
				{
					*ptxt2 = 0;//don't show decimal point for no sig-fig's.
					break;
				}
			}

			if (ndecpassed > nDec) {
				*ptxt2 = 0;
				break;
			}
		}

		//pad with zeroes if needed (e.g., currency requires 2 digits after the decimal)
		if(ndecpassed < nDec)
		{
			nsl_strncpy(zero, zeroes, nDec-ndecpassed<=3 ? nDec-ndecpassed : 3);
			//WSTRNCOPYN(zero, 4, zeroes, nDec-ndecpassed<=3 ? nDec-ndecpassed : 3);//max 3 zeroes for padding
			nsl_strcat(ptxt, zero);
		}

		nsl_strlcpy(pszString, ptxt, nSize);
		//WSTRNCOPYN(pszString, nSize/sizeof(AECHAR), ptxt, -1);
	}
}

boolean 
format_heading(double head, char* pszString, int nSize)
{
	char	fmt[32];
	char	dir[4];

	nsl_strlcpy(fmt, "%s (%d)", sizeof(fmt));
	//STRTOWSTR("%s (%d)", fmt, sizeof(fmt));

	if(head < 0) 
		head = head + 360.0;

	format_heading_dir(head, dir, sizeof(dir));

	nsl_snprintf(pszString, nSize, fmt, dir, (int) head);

	return TRUE;
}

boolean 
format_heading_dir(double v, char* pszString, int nSize)
{
	if(v < 0) 
		v = v + 360.0;

	if( (v >= 0.0 && v < 22.5) || (v >= 337.5 && v <= 360.0))
		nsl_strlcpy(pszString, "N", nSize);
	else if (v >= 22.5  && v < 67.5)
		nsl_strlcpy(pszString, "NE", nSize);
	else if (v >= 67.5  && v < 112.5)
		nsl_strlcpy(pszString, "E", nSize);
	else if (v >= 112.5 && v < 157.5)
		nsl_strlcpy(pszString, "SE", nSize);
	else if (v >= 157.5 && v < 202.5)
		nsl_strlcpy(pszString, "S", nSize);
	else if (v >= 202.5 && v < 247.5)
		nsl_strlcpy(pszString, "SW", nSize);
	else if (v >= 247.5 && v < 292.5)
		nsl_strlcpy(pszString, "W", nSize);
	else if (v >= 292.5 && v < 337.5)
		nsl_strlcpy(pszString, "NW", nSize);

	return TRUE;
}

void
format_distance_f(double dist, char* szoutdist, int nSizeDist, boolean showunits, double dec_limit, boolean round, NB_NavigateAnnouncementUnits units, boolean shortunitnames)
{
	format_distance_ex_f(dist, szoutdist, nSizeDist, showunits, dec_limit, round, units, shortunitnames, -1);
}

void
format_distance_ex_f(double dist, char* szoutdist, int nSizeDist, boolean showunits, double dec_limit, boolean round, NB_NavigateAnnouncementUnits units, boolean shortunitnames, double smallunitthreshold)
{
	char	fmt[8];
	double disto = 0;
	double dists = 0;
	size_t		l = 0;
	boolean smallunits = FALSE;
/*
	if (dist <= 0.0) {

		STRTOWSTR("--", szoutdist, nSizeDist);
		return;
	}
*/
	disto = dist;

	if (units == NB_NAU_Kilometers) {

		dist = disto/1000.0; // convert Meters to Kilometers
		dists = disto;
	}
	else if (units == NB_NAU_MilesYards) {
		dist = disto/1609.0; // convert Meters to Miles
		dists = disto * 1.0936; // yards
	}
	else if (units == NB_NAU_Miles) {
		dist = disto/1609.0; // convert Meters to Miles
		dists = disto*3.2808; // feet
	}

	nsl_strlcpy(fmt, "%d", sizeof(fmt));
	//STRTOWSTR("%d", fmt, sizeof(fmt));

	if (((smallunitthreshold <  0 && dist < 0.1) ||
		 (smallunitthreshold >= 0 && dists <= smallunitthreshold)) && showunits) {

		smallunits = TRUE;

		if (units == NB_NAU_Kilometers) {

			dist = disto; // meters

			if (round) {

				dist = METRIC_DISTANCE_ROUND * (int) ((dist + (METRIC_DISTANCE_ROUND/2)) / METRIC_DISTANCE_ROUND);
			}
		}
		else {

			dist = disto*3.2808; // feet	

			if (round) {

				dist = ENGLISH_DISTANCE_ROUND * (int) ((dist + (ENGLISH_DISTANCE_ROUND/2)) / ENGLISH_DISTANCE_ROUND);
			}
		}

		nsl_snprintf(szoutdist, nSizeDist, fmt, (int) dist);
	}
	else
	{
		/* no scientific notation */
		if (dist <= 0.0001) {

            dist = 0.0;
        }

        if(dist < dec_limit) {

            format_float(dist, szoutdist, nSizeDist, 1);
        }
        else {

            dist += 0.5; // to prevent rough round
            nsl_snprintf(szoutdist, nSizeDist, fmt, (int) dist);
        }
    }

	if (showunits) {

		l = nsl_strlen(szoutdist);

		if (shortunitnames) {
			if (units == NB_NAU_MilesYards){
				nsl_strlcpy(szoutdist+l, (smallunits ? " yd" : " mi"), nSizeDist-l);
			}
			else
			{
			
				nsl_strlcpy(szoutdist+l, (units == NB_NAU_Kilometers) ? (smallunits ? " m" : " km") : (smallunits ? " ft" : " mi"), nSizeDist-l);
				//STRTOWSTR(bmetric ? (smallunits ? " m" : " km") : (smallunits ? " ft" : " mi"), szoutdist+l, nSizeDist-l*sizeof(AECHAR));
			}
		}
		else {

			//remove large unit plurality for values <= 1 mile or 1 kilometer. Assume we will at minimum show 2 meters and 2 feet.
			if ((dist<=1.0) && !smallunits) {
				if (units == NB_NAU_MilesYards){
					nsl_strlcpy(szoutdist+l, (smallunits ? " yards" : " mile"), nSizeDist-l);
				}
				else
				{
				
					nsl_strlcpy(szoutdist+l, (units == NB_NAU_Kilometers) ? (smallunits ? " meters" : " kilometer") : (smallunits ? " feet" : " mile"), nSizeDist-l);
					//STRTOWSTR(bmetric ? (smallunits ? " meters" : " kilometer") : (smallunits ? " feet" : " mile"), szoutdist+l, nSizeDist-l*sizeof(AECHAR));
				}
			}
			else {
				if (units == NB_NAU_MilesYards){
					nsl_strlcpy(szoutdist+l, (smallunits ? " yards" : " miles"), nSizeDist-l);
				}
				else
				{
					nsl_strlcpy(szoutdist+l, (units == NB_NAU_Kilometers) ? (smallunits ? " meters" : " kilometers") : (smallunits ? " feet" : " miles"), nSizeDist-l);
					//STRTOWSTR(bmetric ? (smallunits ? " meters" : " kilometers") : (smallunits ? " feet" : " miles"), szoutdist+l, nSizeDist-l*sizeof(AECHAR));
				}
			}
		}
	}

	return;
}

void 
format_latlon(double v, char* pszString, int nSize, boolean islat)
{
	boolean neg = (boolean)(v < 0.0);
	double abs_v = neg ? -v : v;
	int l;
	int n;
	double d;
	char	fmt[16];

	nsl_strlcpy(pszString, islat ? (neg ? "S " : "N ") : (neg ? "W " : "E "), nSize);
	//STRTOWSTR(islat ? (neg ? "S " : "N ") : (neg ? "W " : "E "), pszString, nSize);
	nsl_strlcpy(fmt, "%d", sizeof(fmt));
	//STRTOWSTR("%d", fmt, sizeof(fmt));

	n = (int) abs_v;

	l = (int)nsl_strlen(pszString);

	nsl_snprintf(pszString+l, nSize-l, fmt, n);

	l = (int)nsl_strlen(pszString);

	nsl_strlcpy(pszString+l, " ", nSize-l);
	//STRTOWSTR(" ", pszString+l, nSize-l*sizeof(AECHAR));

	d = n;
	abs_v = abs_v - d;
	abs_v = 60.0 * abs_v;
	n = (int) abs_v;

	l = (int)nsl_strlen(pszString);

	nsl_snprintf(pszString+l, nSize-l, fmt, n);

	l = (int)nsl_strlen(pszString);

	nsl_strlcpy(pszString+l, "' ",  nSize-l);
	//STRTOWSTR("' ", pszString+l, nSize-l*sizeof(AECHAR));
	
	d = n;
	abs_v = abs_v - d;
	abs_v = 60.0 * (abs_v);

	l = (int)nsl_strlen(pszString);

	format_float(abs_v, pszString+l, nSize-l, 1);

	l = (int)nsl_strlen(pszString);

	nsl_strlcpy(pszString+l, "\"", nSize-l);
	//STRTOWSTR("\"", pszString+l, nSize-l*sizeof(AECHAR));
}

#define TEL_FS_SHORT "%s-%s"
#define TEL_FS_FULL  "%s-%s-%s"

void
format_tel_w(const char* tel, int len, char* buf, int nSize)
{
#if 0
// too much BREW
	char	ac[4];
	char	pre[4];
	char	num[FORMATTED_PHONE_LEN+1];	// plenty big for left over digits
	char	telfmt[20]= { 0 };
	boolean	alldigits;
	const char*	p;

	ac[0] = 0;
	pre[0] = 0;
	num[0] = 0;

	alldigits = TRUE;
	for (p = tel; alldigits && *p != 0; p++)
		alldigits = (boolean)(GETCHTYPE(*p) == SC_DIGIT);

	if (!alldigits) {

		WSTRNCOPYN(buf, nSize / sizeof(AECHAR), tel, -1);
		return;	/* all done */
	}

	if (len >= 11 && tel[0] == (AECHAR)'1') {

		/* 11 digit phone number i.e. 19495551212 -> 949-555-1212 */

		WSTRNCOPYN(ac, ARRAYSIZE(ac), tel+1, 3);
		WSTRNCOPYN(pre, ARRAYSIZE(pre), tel+4, 3);
		WSTRNCOPYN(num, ARRAYSIZE(num), tel+7, -1);
	}
	else if (len > 7) {

		WSTRNCOPYN(ac, ARRAYSIZE(ac), tel, 3);
		WSTRNCOPYN(pre, ARRAYSIZE(pre), tel+3, 3);
		WSTRNCOPYN(num, ARRAYSIZE(num), tel+6, -1);
	}
	else if (len > 5) {

		WSTRNCOPYN(pre, ARRAYSIZE(pre), tel, 3);
		WSTRNCOPYN(num, ARRAYSIZE(num), tel+3, -1);
	}
	else {

		WSTRNCOPYN(num, ARRAYSIZE(num), tel, -1);
	}

	if (!STREMPTY(ac)) {

		STRTOWSTR(TEL_FS_FULL, telfmt, sizeof(telfmt));
		WSPRINTF(buf, nSize, telfmt, ac, pre, num);
	}
	else if (!STREMPTY(pre)) {

		STRTOWSTR(TEL_FS_SHORT, telfmt, sizeof(telfmt));
		WSPRINTF(buf, nSize, telfmt, pre, num);
	}
	else {

		WSTRNCOPYN(buf, nSize / sizeof(AECHAR), num, -1);
	}
#endif
}

void
unformat_tel_w(const char* tel, char* buf, int nSize)
{
#if 0
// too much BREW
	const char *p1;
	char *p2;
	for (p1 = tel, p2 = buf; p2 - buf < (int)(nSize / sizeof(char) - 1) && *p1 != 0; p1++)
		if (GETCHTYPE(*p1) == SC_DIGIT || *p1 == (AECHAR)'*' || *p1 == (AECHAR)'#')
			*p2++ = *p1;
	*p2 = 0;
#endif
}

#define LESS_THAN_ONE_MIN					"< 1 min"
#define LESS_THAN_ONE_MIN_NO_UNITS			"< 1"
#define ZERO_MIN							"0 min"
#define ZERO_MIN_NO_UNITS					"0"
#define NOUNITS_FMT							"%u"
#define SECONDS_FORMAT						"%u sec"
#define MINUTES_FORMAT						"%u min"
#define HOURS_FORMAT						"%u %s"
#define HOURS_MINUTES_FORMAT				"%u %s, %u min"
#define HOURS_MINUTES_CLOCK_FORMAT			"%02u:%02u"
#define HOURS_MINUTES_SECONDS_CLOCK_FORMAT	"%02u:%02u:%02u"

void 
seconds_to_h_m_s(uint32 seconds_in, uint32* hours, uint32* minutes, uint32* seconds)
{
	uint32	h = 0;
	uint32	m = 0;
	uint32	s = 0;

	if (seconds_in < (60*60)) {

		// less than one hour

		h = 0;
		m = seconds_in / 60;
	}
	else {

		h = seconds_in / (60 * 60);
		m = (seconds_in / 60) - (h * 60);
	}

	s = seconds_in % 60;

	if (s > 30 && m > 0) {
		m++;
		s = 0;
	}

	while (m >= 60) {

		m-=60;
		h++;
	}

	if (hours)
		*hours = h;
	if (minutes)
		*minutes = m;
	if (seconds)
		*seconds = s;
}

void
format_time_n(uint32 time, char* szouttime, int nSizeTime, boolean showunits, boolean showseconds, boolean roundLT1min)
{
	uint32	h,m,s;
	char	fmt[32];
	char	hr[8];
	char	hrs[8];

	nsl_strlcpy(hr, "hr", sizeof(hr));
	nsl_strlcpy(hrs, "hrs", sizeof(hrs));

	if (roundLT1min && time > 30 && time < 60)
		time = 60;

	seconds_to_h_m_s(time, &h, &m, &s);
	
	if (time < 60) {

		if (showseconds) {
			nsl_strlcpy(fmt, showunits ? SECONDS_FORMAT : NOUNITS_FMT, sizeof(fmt));
			nsl_snprintf(szouttime, nSizeTime, fmt, time);
		}
		else
		{
			if (roundLT1min && time <= 30)
				nsl_strlcpy(szouttime, showunits ? ZERO_MIN : ZERO_MIN_NO_UNITS, nSizeTime);
			else
				nsl_strlcpy(szouttime, showunits ? LESS_THAN_ONE_MIN : LESS_THAN_ONE_MIN_NO_UNITS, nSizeTime);
		}
	}
	else if (h == 0) {

		nsl_strlcpy(fmt, showunits ? MINUTES_FORMAT : NOUNITS_FMT, sizeof(fmt));
		nsl_snprintf(szouttime, nSizeTime, fmt, m);
	}
	else if (m == 0) {

		nsl_strlcpy(fmt, showunits ? HOURS_FORMAT : NOUNITS_FMT, sizeof(fmt));
		nsl_snprintf(szouttime, nSizeTime, fmt, h, (h > 1) ? hrs : hr);
	}
	else {

		if (showunits) {

			nsl_strlcpy(fmt, HOURS_MINUTES_FORMAT, sizeof(fmt));
			nsl_snprintf(szouttime, nSizeTime, fmt, h, (h > 1) ? hrs : hr, m);
		}
		else {

			format_time_clock_n(time, szouttime, nSizeTime, showseconds);
		}
	}
}

// h:mm(:ss)
void
format_time_clock_n(uint32 time, char* szouttime, int nSizeTime, boolean showseconds)
{
	uint32	h,m,s;
	char	fmt[32];

	seconds_to_h_m_s(time, &h, &m, &s);

	if (showseconds)
	{
		nsl_strlcpy(fmt, HOURS_MINUTES_SECONDS_CLOCK_FORMAT, sizeof(fmt));
		nsl_snprintf(szouttime, nSizeTime, fmt, h, m, s);
	}
	else
	{
		nsl_strlcpy(fmt, HOURS_MINUTES_CLOCK_FORMAT, sizeof(fmt));
		nsl_snprintf(szouttime, nSizeTime, fmt, h, m);
	}
}

void
format_contact_plus_number(const char* name, const char* fmtnumber, char* pszString, int nSize)
{
	char fmt[32] = {0};

	nsl_memset(pszString, 0, nSize);

	if (name != NULL && nsl_strlen(name) > 0 && fmtnumber != NULL && nsl_strlen(fmtnumber) > 0) {

		nsl_strlcpy(fmt, "%s (%s)", sizeof(fmt));
		//STRTOWSTR("%s (%s)", fmt, sizeof(fmt));
		nsl_snprintf(pszString, nSize, fmt, name, fmtnumber);
	}
	else if ((fmtnumber == NULL || nsl_strlen(fmtnumber) == 0) && (name != NULL && nsl_strlen(name) > 0)) {

		nsl_strlcpy(pszString, name, nSize);
		//WSTRNCOPY(pszString, nSize, name);
	}
	else if ((name == NULL || nsl_strlen(name) == 0) && (fmtnumber != NULL && nsl_strlen(fmtnumber) > 0)) {

		nsl_strlcpy(pszString, fmtnumber, nSize);
		//WSTRNCOPY(pszString, nSize, fmtnumber);
	}
}

void
format_contact_name(const char* name, const char* fmtnumber, char* pszString, int nSize)
{
	nsl_memset(pszString, 0, nSize);

	if (name != NULL && nsl_strlen(name) > 0) {

		nsl_strlcpy(pszString, name, nSize);
		//WSTRNCOPY(pszString, nSize, name);
	}
	else if (fmtnumber != NULL && nsl_strlen(fmtnumber) > 0) {

		nsl_strlcpy(pszString, fmtnumber, nSize);
		//WSTRNCOPY(pszString, nSize, fmtnumber);
	}
}

// Extract the number from a contact-plus-number formatting
void
unformat_contact_plus_number(const char* input, char* pszNumber, int nSize)
{
	uint16 startIdx = 0;
	uint16 endIdx = (uint16)(nsl_strlen(input)-1);
	const char* tmp;
	while (startIdx < endIdx && input[startIdx] != '(')
		startIdx++;
	while (endIdx > startIdx && input[endIdx] != ')')
		endIdx--;
	if (startIdx < endIdx)
	{
		tmp = &input[startIdx + 1];
		nsl_strlcpy(pszNumber, tmp, MIN(nSize, endIdx - startIdx - 1));
		//WSTRNCOPYN(pszNumber, nSize, tmp, endIdx - startIdx - 1);
	}
	else
		nsl_strcpy(pszNumber, input);
}
