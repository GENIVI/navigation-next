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
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
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
 * nimnbformat.c: created 2008/10/28 by Mark Goddard.
 * Based on lbsutil.c: created 2004/12/31 by Mark Goddard.
 */

#include "nbformat.h"
#include "nbexp.h"
#include "palstdlib.h"
#include "format.h"

/// @todo (BUG 55776) Need to implement these format functions someplace

void
FormatLocation(NB_Context* context, NB_Location* loc, nb_boolean allowLatLonOnly, char* buf, int nSize, int nLine)
{
	/*
	INIMUI *pui = getnimui(papp);
	int li = 0;
	int n = 0;
	int l = 0;
	int la = 0;
	tpselt lfe;
	tpselt lne;
	const char* linesep;
	AECHAR* linestart = NULL;
	char country[4];

	linesep = NULL;
	if (loc->type == NB_Location_MYGPS) {

		if (nLine == -1 || nLine == 0) {
			//STRTOWSTR(ADDRESS_GPS, buf, nSize);
			NIM_LoadResStr(pui, IDS_ADDRESS_GPS, buf, nSize);
		}
	}
	else if (loc->type == NB_Location_IN_MY_DIRECTION) {

		if (nLine == -1 || nLine == 0) {
			//STRTOWSTR(ADDRESS_DIRECTION, buf, nSize);
			NIM_LoadResStr(pui, IDS_ADDRESS_DIRECTION, buf, nSize);
		}
	}
	else if (loc->type == NB_Location_ROUTE || loc->type == NB_Location_ROUTE_GPS) {

		if (nLine == -1 || nLine == 0) {
			//STRTOWSTR(ADDRESS_ROUTE, buf, nSize);
			NIM_LoadResStr(pui, IDS_ADDRESS_ROUTE, buf, nSize);
		}
	}
	else {
		WSTRTOSTR(loc->country, country, sizeof(country));

		if (country[0] == 0)
			STRLCPY(country, get_default_country(getadmincfg(papp)), sizeof(country));

		buf[0] = 0;

		lfe = get_location_format(getadmincfgptr(papp), country);

		if (lfe == NULL)
			return;

		linesep = te_getattrc(lfe, "line-separator");

		while ((lne = te_nextchild(lfe, &li)) != NULL) {

			if (str_cmpx(te_getname(lne), "line") == 0) {

				if (n == nLine || nLine == FORMAT_LOC_ALL) {

					l = WSTRLEN(buf);

					linestart = buf+l;

					if (nLine == FORMAT_LOC_ALL && l > 0 && linesep != NULL) {

						FormatLocationTag(papp, loc, buf+l, nSize-l*sizeof(AECHAR), lne, linestart);

						if (WSTRLEN(buf) > l) {
							
							STRTOWSTR(linesep, buf+l, nSize-l*sizeof(AECHAR));
							l = WSTRLEN(buf);
							linestart = buf+l;
						}
					}

					FormatLocationTag(papp, loc, buf+l, nSize-l*sizeof(AECHAR), lne, linestart);
					la = WSTRLEN(buf);
				}

				n++;
			}
		}

		// some cases string end with , for example CA highway, , 
		// Trim off " " and separator
		l = WSTRLEN(buf) - 1;
		while(l > 0 && linesep ){
			if(*(buf+l) != ' '  && *(buf+l) != (AECHAR)*linesep)
				break;
			else
				*(buf+l) = 0;

			l--;

		}

	}

	l = WSTRLEN(buf);

	if (allowLatLonOnly && l == 0 && loc->latitude != INVALID_LATLON && loc->longitude != INVALID_LATLON) {

		if (nLine != -1) {

			// check if any lines have text
			FormatLocation(papp, loc, FALSE, buf, nSize, -1);
			l = WSTRLEN(buf);
			buf[0] = 0; // reclear the buffer
		}

		if (l == 0) {

			lfe = get_location_format(getadmincfgptr(papp), country);

			if (lfe == NULL)
				return;

			linesep = te_getattrc(lfe, "line-separator");

			if (linesep == NULL)
				return;

			if (nLine == -1) {

				l = WSTRLEN(buf);
				format_latlon(loc->latitude, buf+l, nSize-l*sizeof(AECHAR), TRUE);

				l = WSTRLEN(buf);
				STRTOWSTR(linesep, buf+l, nSize-l*sizeof(AECHAR));

				l = WSTRLEN(buf);
				format_latlon(loc->longitude, buf+l, nSize-l*sizeof(AECHAR), FALSE);
			}
			else if (nLine == 0) {

				format_latlon(loc->latitude, buf, nSize, TRUE);
			}
			else if (nLine == 1) {

				format_latlon(loc->longitude, buf, nSize, FALSE);
			}
		}
	}
	*/
}

void 
FormatPlace(NB_Context* context, NB_Place* place, char* buf, int nSize)
{
	/*
	int l = 0;
	int lb = 0;
	int la = 0;

	nsl_memset(buf, 0, nSize);

	// Check that we won't display the same name twice.
	if (WSTRCMP(place->name, place->location.areaname) != 0) {

		WSTRNCOPYN(buf, nSize/sizeof(AECHAR), place->name, -1);
		l = (int) WSTRLEN(buf);
		lb = (int) WSTRLEN(buf);

		if (l > 0) {
			STRTOWSTR(", ", buf+l, nSize - (l * sizeof(AECHAR)));
			l = WSTRLEN(buf);
		}

		la = (int) WSTRLEN(buf);
	}

	FormatLocation(papp, &place->location, la == 0 ? TRUE : FALSE, buf+l, nSize - (l * sizeof(AECHAR)), FORMAT_LOC_ALL);

	if ((int) WSTRLEN(buf) == la)
		buf[lb] = 0;
	*/
}

void
FormatPlace2(NB_Context* context, NB_Place* place, char* buf_pri, int nSizePri, char* buf_sec, int nSizeSec)
{
	if (place->name[0] == 0) {

		FormatPlace(context, place, buf_pri, nSizePri);
		buf_sec[0] = 0;
	}
	else {

		//WSTRNCOPYN(buf_pri, nSizePri, place->name, -1);
		nsl_strlcpy(buf_pri, place->name, nSizePri);
		FormatLocation(context, &place->location, FALSE, buf_sec, nSizeSec, FORMAT_LOC_ALL);
	}
}
