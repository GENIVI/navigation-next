/*****************************************************************************
 *
 * Filename:    log_types.h
 *
 * Created:     August 26, 2011
 *
 * Description: Public logworthy log/log set definitions.
  *
 * Copyright 2011, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software.   Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 *
 ****************************************************************************/

#ifndef _LOG_TYPES_H__
#define _LOG_TYPES_H__

#include <stdint.h>

/**
 * Sets of logs that a client can request when triggering a Logworthy Event.
 *
 * This allows the client to specify "I want all typical information/logs"
 * without needing to know the specific log/info types.  This is especially useful
 * because we will want to avoid requiring updates to each client to take advantage
 * of new logs or info that will likely become available over time.
 */
typedef enum {
    LOGSET_NONE = 0,           ///< No set specified.
    LOGSET_BASE,               ///< Common set of logs
    LOGSET_HW_RESET,           ///< Hardware reset logs, etc.
    LOGSET_PROC_CRASH,         ///< Process crash logs, core dump, etc.
    LOGSET_BUG_REPORT,         ///< User-generated bug report logs
    LOGSET_CELLULAR_BASE,      ///< Common set of radio logs
    LOGSET_CELLULAR_MODEM_CRASH,   ///< Special logs for modem crashes
    LOGSET_CELLULAR_CALL_DROP,     ///< Special logs for call drops
    LOGSET_CELLULAR_AUDIO_QUALITY, ///< Special logs for audio quality report
    LOGSET_BUGREPORT_FIRST,        ///< First set of logs to be sent during a user triggered
    LOGSET_PROC_CRASH_FIRST,       ///< First set of logs to be sent during a process crash
//...
    LOGSET_NUM,                ///< Total number of log sets.
    LOGSET_ALL = 0xffff        ///< Capture all logs (used for user-generated
                               ///< BugReports
} _forensics_log_set_t;
typedef uint16_t forensics_log_set_t;

/**
 * Specific logs/info to capture as part of a Logworthy Event.
 *
 * Note that items forensics_log_set_t are defined by collections from
 * this enumeration. */
#define LOG_OEM_START   0x1000
typedef enum {
    LOG_NONE = 0,              ///< End of logs

// Generic log types
    LOG_SLOGGER,
    LOG_OS_INFO,
    LOG_PPS,
    LOG_QUIP_EXIT,
    LOG_INVALID_CREATION_TIME,
    LOG_NUM

} _forensics_log_t;
typedef uint16_t forensics_log_t;

#endif

#include <sys/srcversion.h>
__SRCVERSION( "$URL$ $Rev$" )
