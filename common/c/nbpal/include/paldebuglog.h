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
/* (C) Copyright 2005 by Networks In Motion, Inc.                */
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
 * nimdebuglog.h: created 2005/02/13 by Mark Goddard.
 */

#ifndef PALDEBUGLOG_H
#define	PALDEBUGLOG_H

#include "paltypes.h"

/* Subsystems */

#define LOG_SS_MEMORY		(1<<0)
#define LOG_SS_FILE			(1<<1)
#define LOG_SS_NET			(1<<2)
#define LOG_SS_GPS			(1<<3)
#define LOG_SS_TPS			(1<<4)
#define LOG_SS_DATA			(1<<5)
#define LOG_SS_RP			(1<<6)
#define LOG_SS_FAV			(1<<7)
#define LOG_SS_MYSEARCH		(1<<8)
#define	LOG_SS_UI			(1<<9)
#define LOG_SS_NAVUI		(1<<10)
#define LOG_SS_REF			(1<<11)
#define LOG_SS_CACHE		(1<<12)
#define LOG_SS_DBGNET		(1<<13)
#define LOG_SS_YP			(1<<14)
#define LOG_SS_CALL			(1<<15)

#define LOG_SS_NB_NET		(1<<16)
#define LOG_SS_NB_GEOCODE	(1<<17)
#define LOG_SS_NB_MAP		(1<<18)
#define LOG_SS_NB_POI		(1<<19)
#define LOG_SS_NB_NAV		(1<<20)
#define LOG_SS_NB_FILESET	(1<<21)
#define LOG_SS_NB_DIR		(1<<22)

#define LOG_SS_CORE			(1<<23)
#define LOG_SS_MEDIA		(1<<24)	// for Moto debugging
#define LOG_SS_NET2			(1<<25)	// for Moto debugging
#define LOG_SS_GPS2			(1<<26)	// for Moto debugging

#define LOG_SS_NB_PAL		(1<<27)
#define LOG_SS_JNI          (1<<28)

/* Severity */

#define LOG_SEV_CRITICAL	(0)
#define LOG_SEV_MAJOR		(1)
#define LOG_SEV_IMPORTANT	(2)
#define LOG_SEV_MINOR		(3)
#define LOG_SEV_INFO		(4)
#define LOG_SEV_MINOR_INFO	(5)
#define LOG_SEV_DEBUG		(6)

#if defined(DEBUG_LOG_ALL)
#define DEBUG_LOG_SS	0x00000000
#define DEBUG_LOG_SEV	LOG_SEV_DEBUG
#elif defined(DEBUG_LOG_MEDIA_GPS_NET)
#define DEBUG_LOG_SS	(LOG_SS_MEDIA | LOG_SS_NET2 | LOG_SS_GPS2)
#define DEBUG_LOG_SEV	LOG_SEV_DEBUG
#elif defined(DEBUG_LOG_INFO)
#define DEBUG_LOG_SS	0xFFFFFFFF
#define DEBUG_LOG_SEV	LOG_SEV_INFO
#elif defined(DEBUG_LOG_IMPORTANT)
#define DEBUG_LOG_SS	0xFFFFFFFF
#define DEBUG_LOG_SEV	LOG_SEV_IMPORTANT
#elif defined(DEBUG_LOG_MAJOR)
#define DEBUG_LOG_SS	0xFFFFFFFF
#define DEBUG_LOG_SEV	LOG_SEV_MAJOR
#elif defined(DEBUG_LOG_SUBSYS) && defined(DEBUG_LOG_SEVERITY)
#define DEBUG_LOG_SS	DEBUG_LOG_SUBSYS
#define DEBUG_LOG_SEV	DEBUG_LOG_SEVERITY
#elif defined(DEBUG_LOG_NONE)
#define DEBUG_LOG_SS	0
#define DEBUG_LOG_SEV	0
#else
#define DEBUG_LOG_SS	0
#define DEBUG_LOG_SEV	0
#endif

// Define this define for testing. Do NOT define this for any release/production build!
#ifdef _DEBUG_LOGGING_ENABLED_
    #define TEST_LOGGING debugf
#else
    #define TEST_LOGGING(fmt, ...)
#endif

/*
    DON'T call this function directly! It does NOT get disabled in release mode!
    Call TEST_LOGGING instead! 
*/
PAL_DEC void debugf(const char *fmt, ...);

/*
    Only used to silence "Unused Entity Issue: Expression result unused" in XCode 4.5. Not sure if there is a better way to do this.
    This should be optimized out by the compiler, I presume.
 */
PAL_DEC void dummyf(const char* fmt, ...);

#ifndef NDEBUG

/*
    Not quite sure how to use these. I guess you whould have to define severity and subsystem for this to work.
*/

#define DEBUGLOG(subsystem,severity,msg)	\
	do {									\
		if ((subsystem & DEBUG_LOG_SS) &&	\
			(severity <= DEBUG_LOG_SEV))	\
			debugf msg ;					\
	} while (0)
#define DEBUGLOGD(subsystem,severity,name,dbl)	\
	do {										\
		if ((subsystem & DEBUG_LOG_SS) &&		\
			(severity <= DEBUG_LOG_SEV))		\
			dumpd(name,dbl);					\
	} while (0)
//#ifdef USE_BREW_LOGGING
//#define DEBUGLOG2 DEBUGLOG
//#else
#define DEBUGLOG2(subsystem,severity,msg)		\
	do {										\
		if ((subsystem & DEBUG_LOG_SS) &&		\
			(severity <= DEBUG_LOG_SEV)) {		\
			DBGPRINTF msg ;						\
		}										\
	} while (0)
//#endif
#else
#define	DEBUGLOG(subsystem,severity,msg)
#define DEBUGLOG2(subsystem,severity,msg)
#define DEBUGLOGD(subsystem,severity,name,dbl)
#endif

#ifdef AEE_SIMULATOR
#define PROFILE_FUNCTION	__FUNCTION__
#else
#define PROFILE_FUNCTION	__func__
#endif

/* Primitive profiling macros				*/
/* GETUPTIMEMS() granularity is 5-10 ms.	*/

#ifndef NDEBUG
	#define PROFILE_COOKIE(cookie)			uint32 cookie
	#define PROFILE_START(cookie)			cookie = GETUPTIMEMS();
	#define PROFILE_STOP(ss,cookie,lbl)		DEBUGLOG(ss, LOG_SEV_DEBUG, ("%s took %u ms (%s() in %s: %u)", lbl, GETUPTIMEMS() - cookie, PROFILE_FUNCTION, __FILE__, __LINE__))
	#define MEMPROFILE_COOKIE(cookie)		uint32 cookie
	#define MEMPROFILE_START(cookie)		cookie = getmemstats();
	#define MEMPROFILE_STOP(ss,cookie,lbl)	DEBUGLOG(ss, LOG_SEV_DEBUG, ("%s took %u bytes (%s() in %s: %u)", lbl, getmemstats() - cookie, PROFILE_FUNCTION, __FILE__, __LINE__))
	#define MEMSTATS(lbl)					DEBUGLOG(LOG_SS_MEMORY, LOG_SEV_DEBUG, ("MEM USED - %s: %d", lbl, getmemstats()))
	#define USES_PROFILE(ss)				uint32 _profcookie; uint32 _profss = ss;
	#define PSTART							PROFILE_START(_profcookie);
	#define PSTOP(msg)						PROFILE_STOP(_profss,_profcookie,msg);
#else
	#define PROFILE_COOKIE(cookie)
	#define PROFILE_START(cookie)
	#define PROFILE_STOP(ss,cookie,lbl)
	#define MEMPROFILE_COOKIE(cookie)
	#define MEMPROFILE_START(cookie)
	#define MEMPROFILE_STOP(ss,cookie,lbl)
	#define MEMSTATS(lbl)
	#define USES_PROFILE(ss)
	#define PSTART
	#define PSTOP(msg)
#endif

#endif
